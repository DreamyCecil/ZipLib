#include "ZipArchive.h"
#include "streams/serialization.h"
#include <algorithm>
#include <cassert>

#define CALL_CONST_METHOD(expression) \
  const_cast<      std::remove_pointer<std::remove_const<decltype(expression)>::type>::type*>( \
  const_cast<const std::remove_pointer<std::remove_const<decltype(this)      >::type>::type*>(this)->expression)

//////////////////////////////////////////////////////////////////////////
// internal shared buffer

ZipArchive::InternalSharedBuffer* ZipArchive::InternalSharedBuffer::GetInstance()
{
  static ZipArchive::InternalSharedBuffer instance;
  return &instance;
}

size_t ZipArchive::InternalSharedBuffer::GetBufferSize()
{
  return INTERNAL_BUFFER_SIZE;
}

char* ZipArchive::InternalSharedBuffer::GetBuffer()
{
  if (_buffer == nullptr)
  {
    _buffer = new char[INTERNAL_BUFFER_SIZE];
  }

  return _buffer;
}

void ZipArchive::InternalSharedBuffer::IncRef()
{
  ++_refCount;
}

void ZipArchive::InternalSharedBuffer::DecRef()
{
  assert(_refCount > 0);

  if (_refCount == 0)
  {
    return;
  }

  if (--_refCount == 0 && _buffer != nullptr)
  {
    delete[] _buffer;
    _buffer = nullptr;
  }
}

//////////////////////////////////////////////////////////////////////////
// zip archive

ZipArchive::ZipArchive()
  : _zipStream(nullptr)
  , _destroySimultaneously(false)
{
  InternalSharedBuffer::GetInstance()->IncRef();
}

ZipArchive::ZipArchive(ZipArchive&& other)
{
  InternalSharedBuffer::GetInstance()->IncRef();

  _endOfCentralDirectoryBlock = other._endOfCentralDirectoryBlock;
  _entries = std::move(other._entries);
  _zipStream = other._zipStream;
  _destroySimultaneously = other._destroySimultaneously;

  // clean "other"
  other._zipStream = nullptr;
  other._destroySimultaneously = false;
}

ZipArchive::ZipArchive(std::istream* stream)
  : _zipStream(stream)
  , _destroySimultaneously(false)
{
  InternalSharedBuffer::GetInstance()->IncRef();

  if (stream != nullptr)
  {
    this->ReadEndOfCentralDirectory();
    this->EnsureCentralDirectoryRead();
  }
}

ZipArchive::ZipArchive(std::istream* stream, bool destroySimultaneously)
  : _zipStream(stream)
  , _destroySimultaneously(stream != nullptr ? destroySimultaneously : false)
{
  InternalSharedBuffer::GetInstance()->IncRef();

  // jesus blew up a school bus when this metod has been implemented
  if (stream != nullptr)
  {
    this->ReadEndOfCentralDirectory();
    this->EnsureCentralDirectoryRead();
  }
}

ZipArchive::~ZipArchive()
{
  InternalSharedBuffer::GetInstance()->DecRef();

  std::for_each(_entries.begin(), _entries.end(), [](ZipArchiveEntry* e) { delete e; });

  if (_destroySimultaneously && _zipStream != nullptr) 
  {
    delete _zipStream;
  }
}

ZipArchive& ZipArchive::operator = (ZipArchive&& other)
{
  _endOfCentralDirectoryBlock = other._endOfCentralDirectoryBlock;
  _entries = std::move(other._entries);
  _zipStream = other._zipStream;
  _destroySimultaneously = other._destroySimultaneously;

  // clean "other"
  other._zipStream = nullptr;
  other._destroySimultaneously = false;
  
  return *this;
}

ZipArchiveEntry* ZipArchive::CreateEntry(const std::string& fileName)
{
  ZipArchiveEntry* result = nullptr;

  if (this->GetEntry(fileName) == nullptr)
  {
    if ((result = ZipArchiveEntry::CreateNew(this, fileName)) != nullptr)
    {
      _entries.push_back(result);
    }
  }

  return result;
}

const std::string& ZipArchive::GetComment() const
{
  return _endOfCentralDirectoryBlock.Comment;
}

void ZipArchive::SetComment(const std::string& comment)
{
  _endOfCentralDirectoryBlock.Comment = comment;
}

const ZipArchiveEntry* ZipArchive::GetEntry(int index) const
{
  return _entries[index];
}

const ZipArchiveEntry* ZipArchive::GetEntry(const std::string& entryName) const
{
  auto it = std::find_if(_entries.begin(), _entries.end(), [&entryName](ZipArchiveEntry* value) { return value->GetFullName() == entryName; });

  if (it != _entries.end())
  {
    return *it;
  }

  return nullptr;
}

ZipArchiveEntry* ZipArchive::GetEntry(int index)
{
  return _entries[index];
}

ZipArchiveEntry* ZipArchive::GetEntry(const std::string& entryName)
{
  return CALL_CONST_METHOD(GetEntry(entryName));
}

size_t ZipArchive::GetEntriesCount() const
{
  return _entries.size();
}

void ZipArchive::RemoveEntry(const std::string& entryName)
{
  auto it = std::find_if(_entries.begin(), _entries.end(), [&entryName](ZipArchiveEntry* value) { return value->GetFullName() == entryName; });

  if (it != _entries.end())
  {
    delete *it;
    _entries.erase(it);
  }
}

void ZipArchive::RemoveEntry(int index)
{
  delete _entries[index];
  _entries.erase(_entries.begin() + index);
}

bool ZipArchive::EnsureCentralDirectoryRead()
{
  ZipCentralDirectoryFileHeader zipCentralDirectoryFileHeader;

  _zipStream->seekg(_endOfCentralDirectoryBlock.OffsetOfStartOfCentralDirectoryWithRespectToTheStartingDiskNumber, std::ios::beg);

  while (zipCentralDirectoryFileHeader.Deserialize(*_zipStream))
  {
    ZipArchiveEntry* newEntry;

    if ((newEntry = ZipArchiveEntry::CreateExisting(this, zipCentralDirectoryFileHeader)) != nullptr)
    {
      _entries.push_back(newEntry);
    }

    // ensure clearing of the CDFH struct
    zipCentralDirectoryFileHeader = ZipCentralDirectoryFileHeader();
  }

  return true;
}

bool ZipArchive::ReadEndOfCentralDirectory()
{
  const int EOCDB_SIZE     = 22; // sizeof(EndOfCentralDirectoryBlockBase);
  const int SIGNATURE_SIZE = 4;  // sizeof(std::declval<EndOfCentralDirectoryBlockBase>().Signature);
  const int MIN_SHIFT      = (EOCDB_SIZE - SIGNATURE_SIZE);

  _zipStream->seekg(-MIN_SHIFT, std::ios::end);

  if (this->SeekToSignature(EndOfCentralDirectoryBlock::SignatureConstant, SeekDirection::Backward))
  {
    _endOfCentralDirectoryBlock.Deserialize(*_zipStream);
    return true;
  }

  return false;
}

bool ZipArchive::SeekToSignature(uint32_t signature, SeekDirection direction)
{
  std::streampos streamPosition = _zipStream->tellg();
  uint32_t buffer = 0;
  int appendix = static_cast<int>(direction == SeekDirection::Backward ? 0 - 1 : 1);

  while (!_zipStream->eof() && !_zipStream->fail())
  {
    deserialize(*_zipStream, buffer);

    if (buffer == signature)
    {
      _zipStream->seekg(streamPosition, std::ios::beg);
      return true;
    }

    streamPosition += appendix;
    _zipStream->seekg(streamPosition, std::ios::beg);
  }

  return false;
}

void ZipArchive::WriteToStream(std::ostream& stream)
{
  auto startPosition = stream.tellp();

  for (auto& entry : _entries)
  {
    entry->SerializeLocalFileHeader(stream);
  }

  auto offsetOfStartOfCDFH = stream.tellp() - startPosition;
  for (auto& entry : _entries)
  {
    entry->SerializeCentralDirectoryFileHeader(stream);
  }

  _endOfCentralDirectoryBlock.NumberOfThisDisk = 0;
  _endOfCentralDirectoryBlock.NumberOfTheDiskWithTheStartOfTheCentralDirectory = 0;

  _endOfCentralDirectoryBlock.NumberOfEntriesInTheCentralDirectory = static_cast<uint16_t>(_entries.size());
  _endOfCentralDirectoryBlock.NumberOfEntriesInTheCentralDirectoryOnThisDisk = static_cast<uint16_t>(_entries.size());

  _endOfCentralDirectoryBlock.SizeOfCentralDirectory = static_cast<uint32_t>(stream.tellp() - offsetOfStartOfCDFH);
  _endOfCentralDirectoryBlock.OffsetOfStartOfCentralDirectoryWithRespectToTheStartingDiskNumber = static_cast<uint32_t>(offsetOfStartOfCDFH);
  _endOfCentralDirectoryBlock.Serialize(stream);
}

void ZipArchive::Swap(ZipArchive& other)
{
  if (this == &other) return;

  std::swap(_endOfCentralDirectoryBlock, other._endOfCentralDirectoryBlock);
  std::swap(_entries, other._entries);
  std::swap(_zipStream, other._zipStream);
  std::swap(_destroySimultaneously, other._destroySimultaneously);
}

