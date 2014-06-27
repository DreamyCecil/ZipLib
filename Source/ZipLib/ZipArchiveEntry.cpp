#include "ZipArchiveEntry.h"
#include "ZipArchive.h"

#include "methods/ZipMethodResolver.h"

#include "streams/encryption_encoder_stream.h"
#include "streams/encryption_decoder_stream.h"

#include "streams/compression_encoder_stream.h"
#include "streams/compression_decoder_stream.h"

#include "streams/nullstream.h"
#include "streams/auditstream.h"

#include "utils/stream_utils.h"
#include "utils/time_utils.h"

#include <iostream>
#include <cassert>
#include <algorithm>
#include <memory>
#include <sstream>
#include "detail/ZipBitFlag.h"

namespace
{
  bool IsValidFilename(const std::string& fullPath)
  {
    // this function ensures, that the filename will have non-zero
    // length, when the filename will be normalized

    if (fullPath.length() > 0)
    {
      std::string tmpFilename = fullPath;
      std::replace(tmpFilename.begin(), tmpFilename.end(), '\\', '/');

      // if the filename is built only from '/', then it is invalid path
      return tmpFilename.find_first_not_of('/') != std::string::npos;
    }

    return false;
  }

  std::string GetFilenameFromPath(const std::string& fullPath)
  {
    std::string::size_type dirSeparatorPos;

    if ((dirSeparatorPos = fullPath.find_last_of('/')) != std::string::npos)
    {
      return fullPath.substr(dirSeparatorPos + 1);
    }
    else
    {
      return fullPath;
    }
  }

  bool IsDirectoryPath(const std::string& fullPath)
  {
    return (fullPath.length() > 0 && fullPath.back() == '/');
  }
}

ZipArchiveEntry::ZipArchiveEntry()
  : _archive(nullptr)
  , _inputStream(nullptr)
  , _compressionMode(CompressionMode::Deferred)

  , _originallyInArchive(false)
  , _isNewOrChanged(false)
  , _hasLocalFileHeader(false)

  , _offsetOfCompressedData(-1)
  , _offsetOfSerializedLocalFileHeader(-1)

{

}

ZipArchiveEntry::~ZipArchiveEntry()
{
  this->CloseRawStream();
  this->CloseDecompressionStream();
}

ZipArchiveEntry::Ptr ZipArchiveEntry::CreateNew(ZipArchive* zipArchive, const std::string& fullPath)
{
  ZipArchiveEntry::Ptr result;

  assert(zipArchive != nullptr);

  if (IsValidFilename(fullPath))
  {
    result.reset(new ZipArchiveEntry());

    result->_archive = zipArchive;
    result->_isNewOrChanged = true;
    result->SetAttributes(Attributes::Archive);
    result->_entryInfo.CentralDirectoryFileHeader.VersionNeededToExtract = VERSION_NEEDED_DEFAULT;
    result->_entryInfo.SetVersionMadeBy(VERSION_MADEBY_DEFAULT);
    result->SetLastWriteTime(time(nullptr));
  
    result->SetFullName(fullPath);

    result->_entryInfo.SetCompressionMethod(StoreMethod::CompressionMethod);
    result->_entryInfo.SetGeneralPurposeBitFlag(detail::ZipBitFlag::None);
  }

  return result;
}

ZipArchiveEntry::Ptr ZipArchiveEntry::CreateExisting(ZipArchive* zipArchive, detail::ZipCentralDirectoryFileHeader& cd)
{
  ZipArchiveEntry::Ptr result;

  assert(zipArchive != nullptr);

  if (IsValidFilename(cd.Filename))
  {
    result.reset(new ZipArchiveEntry());

    result->_archive                    = zipArchive;
    result->_entryInfo.CentralDirectoryFileHeader = cd;
    result->_originallyInArchive        = true;
    result->CheckFilenameCorrection();

    // determining folder by path has more priority
    // than attributes. however, if attributes
    // does not correspond with path, they will be fixed.
    result->SetAttributes(IsDirectoryPath(result->GetFullName())
                          ? Attributes::Directory
                          : Attributes::Archive);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
// public methods & getters & setters

const std::string& ZipArchiveEntry::GetFullName() const
{
  return _entryInfo.CentralDirectoryFileHeader.Filename;
}

void ZipArchiveEntry::SetFullName(const std::string& fullName)
{
  std::string filename = fullName;
  std::string correctFilename;

  // unify slashes
  std::replace(filename.begin(), filename.end(), '\\', '/');

  bool isDirectory = IsDirectoryPath(filename);

  // if slash is first char, remove it
  if (filename[0] == '/')
  {
    filename = filename.substr(filename.find_first_not_of('/'));
  }

  // find multiply slashes
  bool prevWasSlash = false;
  for (std::string::size_type i = 0; i < filename.length(); ++i)
  {
    if (filename[i] == '/' && prevWasSlash) continue;
    prevWasSlash = (filename[i] == '/');

    correctFilename += filename[i];
  }

  _entryInfo.CentralDirectoryFileHeader.Filename = correctFilename;
  _name = GetFilenameFromPath(correctFilename);

  this->SetAttributes(isDirectory ? Attributes::Directory : Attributes::Archive);
}

const std::string& ZipArchiveEntry::GetName() const
{
  return _name;
}

void ZipArchiveEntry::SetName(const std::string& name)
{
  std::string folder;
  std::string::size_type dirDelimiterPos;

  // search for '/' in path name.
  // if this entry is directory, search up to one character
  // before the last one (which is '/')
  // if this entry is file, just search until last '/'
  // will be found
  dirDelimiterPos = this->GetFullName().find_last_of('/', 
    (uint32_t)this->GetAttributes() & (uint32_t)Attributes::Archive
    ? std::string::npos
    : this->GetFullName().length() - 1);
      
  if (dirDelimiterPos != std::string::npos)
  {
    folder = this->GetFullName().substr(0, dirDelimiterPos);
  }

  this->SetFullName(folder + name);

  if (this->IsDirectory())
  {
    this->SetFullName(this->GetFullName() + '/');
  }
}

const std::string& ZipArchiveEntry::GetComment() const
{
  return _entryInfo.GetComment();
}

void ZipArchiveEntry::SetComment(const std::string& comment)
{
  _entryInfo.SetComment(comment);
}

time_t ZipArchiveEntry::GetLastWriteTime() const
{
  return _entryInfo.GetLastWriteTime();
}

void ZipArchiveEntry::SetLastWriteTime(time_t modTime)
{
  _entryInfo.SetLastWriteTime(modTime);
}

ZipArchiveEntry::Attributes ZipArchiveEntry::GetAttributes() const
{
  return static_cast<Attributes>(_entryInfo.CentralDirectoryFileHeader.ExternalFileAttributes);
}

uint16_t ZipArchiveEntry::GetCompressionMethod() const
{
  return _entryInfo.CentralDirectoryFileHeader.CompressionMethod;
}

void ZipArchiveEntry::SetAttributes(Attributes value)
{
  Attributes prevVal = this->GetAttributes();
  Attributes newVal = prevVal | value;

  // if we're changing from directory to file
  if (!!(prevVal & Attributes::Directory) && !!(newVal & Attributes::Archive))
  {
    newVal &= ~Attributes::Directory;

    // if path ends with '/', remove it
    if (IsDirectoryPath(_entryInfo.CentralDirectoryFileHeader.Filename))
    {
      _entryInfo.CentralDirectoryFileHeader.Filename.pop_back();
    }
  }

  // if we're changing from file to directory
  else if (!!(prevVal & Attributes::Archive) && !!(newVal & Attributes::Directory))
  {
    newVal &= ~Attributes::Archive;

    // if path does not end with '/', add it 
    if (!IsDirectoryPath(_entryInfo.CentralDirectoryFileHeader.Filename))
    {
      _entryInfo.CentralDirectoryFileHeader.Filename += '/';
    }
  }

  // if this entry is directory, ensure that crc32 & sizes
  // are set to 0 and do not include any stream
  if (!!(newVal & Attributes::Directory))
  {
    _entryInfo.CentralDirectoryFileHeader.Crc32 = 0;
    _entryInfo.CentralDirectoryFileHeader.CompressedSize = 0;
    _entryInfo.CentralDirectoryFileHeader.UncompressedSize = 0;
  }

  _entryInfo.CentralDirectoryFileHeader.ExternalFileAttributes = static_cast<uint32_t>(newVal);
}

bool ZipArchiveEntry::IsEncrypted() const
{
  return !!(_entryInfo.GetGeneralPurposeBitFlag() & detail::ZipBitFlag::Encrypted);
}

uint32_t ZipArchiveEntry::GetCrc32() const
{
  return _entryInfo.CentralDirectoryFileHeader.Crc32;
}

size_t ZipArchiveEntry::GetSize() const
{
  return static_cast<size_t>(_entryInfo.CentralDirectoryFileHeader.UncompressedSize);
}

size_t ZipArchiveEntry::GetCompressedSize() const
{
  return static_cast<size_t>(_entryInfo.CentralDirectoryFileHeader.CompressedSize);
}

bool ZipArchiveEntry::CanExtract() const
{
  return (_entryInfo.CentralDirectoryFileHeader.VersionNeededToExtract <= VERSION_MADEBY_DEFAULT);
}

bool ZipArchiveEntry::IsDirectory() const
{
  return !!(this->GetAttributes() & Attributes::Directory);
}

bool ZipArchiveEntry::IsUsingDataDescriptor() const
{
  return !!(_entryInfo.GetGeneralPurposeBitFlag() & detail::ZipBitFlag::DataDescriptor);
}

void ZipArchiveEntry::UseDataDescriptor(bool use)
{
  _entryInfo.SetGeneralPurposeBitFlag(detail::ZipBitFlag::DataDescriptor, use);
}

std::istream* ZipArchiveEntry::GetRawStream()
{
  if (_rawStream == nullptr)
  {
    if (_originallyInArchive)
    {
      auto offsetOfCompressedData = this->SeekToCompressedData();
      _rawStream = std::make_shared<isubstream>(*_archive->_zipStream, offsetOfCompressedData, this->GetCompressedSize());
    }
    else
    {
      _rawStream = std::make_shared<isubstream>(*_immediateBuffer);
    }
  }

  return _rawStream.get();
}

std::istream* ZipArchiveEntry::GetDecompressionStream()
{
  return this->GetDecompressionStream(std::string());
}

std::istream* ZipArchiveEntry::GetDecompressionStream(const std::string& password)
{
  std::shared_ptr<std::istream> intermediateStream;

  _password = password;

  // there shouldn't be opened another stream
  if (this->CanExtract() && _archiveStream == nullptr && _encryptionStream == nullptr)
  {
    auto offsetOfCompressedData = this->SeekToCompressedData();
    bool needsPassword = !!(_entryInfo.GetGeneralPurposeBitFlag() & detail::ZipBitFlag::Encrypted);
    bool needsDecompress = this->GetCompressionMethod() != StoreMethod::CompressionMethod;

    if (needsPassword && _password.empty())
    {
      // we need password, but we do not have it
      return nullptr;
    }

    // make correctly-ended substream of the input stream
    intermediateStream = _archiveStream = std::make_shared<isubstream>(*_archive->_zipStream, offsetOfCompressedData, this->GetCompressedSize());

    CompressionMethod::Ptr zipMethod = ZipMethodResolver::GetZipMethodInstance(this->shared_from_this());

    if (zipMethod != nullptr)
    {
      std::shared_ptr<encryption_decoder_stream> cryptoStream;
      bool hasCorrectPassword = true;

      if (auto encryptionMethod = zipMethod->GetEncryptionMethod())
      {
        encryptionMethod->SetPassword(_password);
//         encryptionMethod->OnCompressionBegin(this->shared_from_this());

        cryptoStream = std::make_shared<encryption_decoder_stream>(
          encryptionMethod->GetDecoder(),
          encryptionMethod->GetProperties(),
          *intermediateStream);

        // set it here, because in case the hasCorrectPassword is false
        // the method CloseDecompressionStream() will properly delete the stream
        intermediateStream = _encryptionStream = cryptoStream;

        hasCorrectPassword = encryptionMethod->GetDecoder()->has_correct_password();
      }

      if (!hasCorrectPassword)
      {
        this->CloseDecompressionStream();
        return nullptr;
      }

      intermediateStream = _compressionStream = std::make_shared<compression_decoder_stream>(
        zipMethod->GetDecoder(),
        zipMethod->GetProperties(),
        *intermediateStream);
    }
  }

  return intermediateStream.get();
}

bool ZipArchiveEntry::IsRawStreamOpened() const
{
  return _rawStream != nullptr;
}

bool ZipArchiveEntry::IsDecompressionStreamOpened() const
{
  return _compressionStream != nullptr;
}

void ZipArchiveEntry::CloseRawStream()
{
  _rawStream.reset();
}

void ZipArchiveEntry::CloseDecompressionStream()
{
  _compressionStream.reset();
  _encryptionStream.reset();
  _archiveStream.reset();
  _immediateBuffer.reset();
}

bool ZipArchiveEntry::SetCompressionStream(std::istream& stream, CompressionMethod::Ptr method /* = DeflateMethod::Create() */, CompressionMode mode /* = CompressionMode::Deferred */)
{
  // if _inputStream is set, we already have some stream to compress
  // so we discard it
  if (_inputStream != nullptr)
  {
    this->UnloadCompressionData();
  }

  _isNewOrChanged = true;

  _inputStream = &stream;
  _compressionMethod = method;
  _compressionMode = mode;
  _entryInfo.SetCompressionMethod(method->GetFinalMethodDescriptor().CompressionMethod);

  if (_inputStream != nullptr && _compressionMode == CompressionMode::Immediate)
  {
    if (auto encryptionMethod = _compressionMethod->GetEncryptionMethod())
    {
      _entryInfo.SetGeneralPurposeBitFlag(detail::ZipBitFlag::Encrypted);

      encryptionMethod->OnEncryptionBegin(this->shared_from_this());
    }

    _immediateBuffer = std::make_shared<std::stringstream>();
    this->InternalCompressStream(*_inputStream, *_immediateBuffer);

    // we have everything we need, let's act like we were loaded from archive :)
    _isNewOrChanged = false;
    _inputStream = nullptr;
  }

  return true;
}

void ZipArchiveEntry::UnsetCompressionStream()
{
  if (!this->HasCompressionStream())
  {
    this->FetchLocalFileHeader();
  }

  this->UnloadCompressionData();
}

void ZipArchiveEntry::Remove()
{
  auto it = std::find(
    _archive->_entries.begin(),
    _archive->_entries.end(),
    this->shared_from_this());

  if (it != _archive->_entries.end())
  {
    _archive->_entries.erase(it);
    delete this;
  }
}

//////////////////////////////////////////////////////////////////////////
// private getters & setters

bool ZipArchiveEntry::HasCompressionStream() const
{
  return _inputStream != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// private working methods

void ZipArchiveEntry::FetchLocalFileHeader()
{
  if (!_hasLocalFileHeader && _originallyInArchive && _archive != nullptr)
  {
    _archive->_zipStream->seekg(_entryInfo.GetOffsetOfLocalHeader(), std::ios::beg);
    _entryInfo.LocalFileHeader.Deserialize(*_archive->_zipStream);

    _offsetOfCompressedData = _archive->_zipStream->tellg();
  }

  // sync data
  _entryInfo.SyncWithCentralDirectoryFileHeader();
  _hasLocalFileHeader = true;
}

void ZipArchiveEntry::CheckFilenameCorrection()
{
  // this forces recheck of the filename.
  // this is useful when the check is needed after
  // deserialization
  this->SetFullName(this->GetFullName());
}

std::ios::pos_type ZipArchiveEntry::GetOffsetOfCompressedData()
{
  if (!_hasLocalFileHeader)
  {
    this->FetchLocalFileHeader();
  }

  return _offsetOfCompressedData;
}

std::ios::pos_type ZipArchiveEntry::SeekToCompressedData()
{
  // check for fail bit?
  _archive->_zipStream->seekg(this->GetOffsetOfCompressedData(), std::ios::beg);
  return this->GetOffsetOfCompressedData();
}

void ZipArchiveEntry::SerializeLocalFileHeader(std::ostream& stream)
{
  // ensure opening the stream
  std::istream* compressedDataStream = nullptr;

  if (!this->IsDirectory())
  {
    if (_inputStream == nullptr)
    {
      if (!_isNewOrChanged)
      {
        // the file was either compressed in immediate mode,
        // or was in previous archive
        compressedDataStream = this->GetRawStream();
      }

      // if file is new and empty or stream has been set to nullptr,
      // just do not set any compressed data stream
    }
    else
    {
      assert(_isNewOrChanged);
      compressedDataStream = _inputStream;
    }
  }

  if (auto encryptionMethod = _compressionMethod->GetEncryptionMethod())
  {
    _entryInfo.SetGeneralPurposeBitFlag(detail::ZipBitFlag::Encrypted);

    encryptionMethod->OnEncryptionBegin(this->shared_from_this());
  }

  if (!_hasLocalFileHeader)
  {
    this->FetchLocalFileHeader();
  }

  // save offset of stream here
  _offsetOfSerializedLocalFileHeader = stream.tellp();      

  if (this->IsUsingDataDescriptor())
  {
    _entryInfo.LocalFileHeader.CompressedSize = 0;
    _entryInfo.LocalFileHeader.UncompressedSize = 0;
    _entryInfo.LocalFileHeader.Crc32 = 0;
  }

  _entryInfo.LocalFileHeader.Serialize(stream);

  // if this entry is a directory, it should not contain any data
  // nor crc.
  assert(
    this->IsDirectory()
    ? !GetCrc32() && !GetSize() && !GetCompressedSize() && !_inputStream
    : true
  );

  if (!this->IsDirectory() && compressedDataStream != nullptr)
  {
    if (_isNewOrChanged)
    {
      this->InternalCompressStream(*compressedDataStream, stream);

      if (this->IsUsingDataDescriptor())
      {
        _entryInfo.LocalFileHeader.SerializeAsDataDescriptor(stream);
      }
      else
      {
        // actualize local file header
        // make non-seekable version?
        stream.seekp(_offsetOfSerializedLocalFileHeader);
        _entryInfo.LocalFileHeader.Serialize(stream);
        stream.seekp(this->GetCompressedSize(), std::ios::cur);
      }
    }
    else
    {
      utils::stream::copy(*compressedDataStream, stream);
    }
  }
}

void ZipArchiveEntry::SerializeCentralDirectoryFileHeader(std::ostream& stream)
{
  _entryInfo.CentralDirectoryFileHeader.RelativeOffsetOfLocalHeader = static_cast<int32_t>(_offsetOfSerializedLocalFileHeader);
  _entryInfo.CentralDirectoryFileHeader.Serialize(stream);
}

void ZipArchiveEntry::UnloadCompressionData()
{
  // unload stream
  _immediateBuffer->clear();
  _inputStream = nullptr;

  _entryInfo.CentralDirectoryFileHeader.CompressedSize = 0;
  _entryInfo.CentralDirectoryFileHeader.UncompressedSize = 0;
  _entryInfo.CentralDirectoryFileHeader.Crc32 = 0;
}

void ZipArchiveEntry::InternalCompressStream(std::istream& inputStream, std::ostream& outputStream)
{
  iauditstream inputAuditStream(inputStream);
  oauditstream outputAuditStream(outputStream);

  crc32stream crc32Stream;

  {
    std::ostream* intermediateStream = &outputAuditStream;

    std::unique_ptr<encryption_encoder_stream> encryptionStream;
    if (auto encryptionMethod = _compressionMethod->GetEncryptionMethod())
    {
      encryptionStream = std::unique_ptr<encryption_encoder_stream>(new encryption_encoder_stream(
        encryptionMethod->GetEncoder(),
        encryptionMethod->GetProperties(),
        *intermediateStream));

      intermediateStream = encryptionStream.get();
    }

    crc32Stream.init(inputAuditStream);

    compression_encoder_stream compressionStream(
      _compressionMethod->GetEncoder(),
      _compressionMethod->GetProperties(),
      *intermediateStream);
    intermediateStream = &compressionStream;
    utils::stream::copy(crc32Stream, *intermediateStream);

    intermediateStream->flush();
  }
  _entryInfo.LocalFileHeader.UncompressedSize = static_cast<uint32_t>(inputAuditStream.get_bytes_read());
  _entryInfo.LocalFileHeader.CompressedSize   = static_cast<uint32_t>(outputAuditStream.get_bytes_written());
  _entryInfo.LocalFileHeader.Crc32 = crc32Stream.get_crc32();

  _entryInfo.SyncWithLocalFileHeader();

  // Fix VersionToExtract.
  _entryInfo.FixVersionToExtractAtLeast(this->IsDirectory()
    ? VERSION_NEEDED_EXPLICIT_DIRECTORY
    : _compressionMethod->GetFinalMethodDescriptor().VersionNeededToExtract);
}

void ZipArchiveEntry::FigureCrc32()
{
  if (this->IsDirectory() || _inputStream == nullptr || !_isNewOrChanged)
  {
    return;
  }

  // stream must be seekable
  auto position = _inputStream->tellg();

  // compute crc32
  crc32stream crc32Stream;
  crc32Stream.init(*_inputStream);

  // just force to read all from crc32stream
  nullstream nulldev;
  utils::stream::copy(crc32Stream, nulldev);

  // seek back
  _inputStream->clear();
  _inputStream->seekg(position);

  _entryInfo.CentralDirectoryFileHeader.Crc32 = crc32Stream.get_crc32();
}

uint8_t ZipArchiveEntry::GetLastByteOfEncryptionHeader()
{
  if (!!(_entryInfo.GetGeneralPurposeBitFlag() & detail::ZipBitFlag::DataDescriptor))
  {
    // In the case that bit 3 of the general purpose bit flag is set to
    // indicate the presence of a 'data descriptor' (signature
    // 0x08074b50), the last byte of the decrypted header is sometimes
    // compared with the high-order byte of the lastmodified time,
    // rather than the high-order byte of the CRC, to verify the
    // password.
    //
    // This is not documented in the PKWare Appnote.txt.
    // This was discovered this by analysis of the Crypt.c source file in the
    // InfoZip library
    // http://www.info-zip.org/pub/infozip/

    // Also, winzip insists on this!
    return uint8_t((_entryInfo.CentralDirectoryFileHeader.LastModificationTime >> 8) & 0xff);
  }
  else
  {
    // When bit 3 is not set, the CRC value is required before
    // encryption of the file data begins. In this case there is no way
    // around it: must read the stream in its entirety to compute the
    // actual CRC before proceeding.
    this->FigureCrc32();
    return uint8_t((this->GetCrc32() >> 24) & 0xff);
  }
}
