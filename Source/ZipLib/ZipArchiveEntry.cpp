#include <iostream>
#include <cassert>

#include "ZipArchiveEntry.h"
#include "ZipArchive.h"
#include "ZipLocalFileHeader.h"

#include "streams/defstream.h"
#include "streams/teestream.h"
#include "streams/zip_cryptostream.h"

#if defined(_MSC_VER)
# define ZAE_localtime(dt, ts)  do { localtime_s((ts), (dt)); } while (0)
#elif defined(__GNUC__) || defined(__GNUG__)
# define ZAE_localtime(dt, ts)  do { localtime_r((dt), (ts)); } while (0)
#else
# define ZAE_localtime(dt, ts)  do { tm* _tmp = localtime(dt); memcpy((ts), _tmp, sizeof(tm)); } while (0)
#endif

ZipArchiveEntry::ZipArchiveEntry()
  : _archive(nullptr)
  , _openedArchive(nullptr)
  , _inflateStream(nullptr)
  , _zipCryptoStream(nullptr)
  , _rawStream(nullptr)

  , _originallyInArchive(false)
  , _isNewOrChanged(false)
  , _hasLocalFileHeader(false)

  , _offsetOfCompressedData(-1)
  , _offsetOfSerializedLocalFileHeader(-1)

  , _compressionStream(nullptr)
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
    result->SetVersionToExtract(ZipVersionNeeded::Default);
    result->SetVersionMadeBy(DEFAULT_VERSION_MADEBY);
    result->SetLastWriteTime(time(nullptr));
  
    result->SetFullName(fullPath);

    result->SetCompressionMethod(CompressionMethod::Stored);
    result->SetGeneralPurposeBitFlag(BitFlag::None);
  }

  return result;
}

ZipArchiveEntry::Ptr ZipArchiveEntry::CreateExisting(ZipArchive* zipArchive, ZipCentralDirectoryFileHeader& cd)
{
  ZipArchiveEntry::Ptr result;

  assert(zipArchive != nullptr);

  if (IsValidFilename(cd.Filename))
  {
    result.reset(new ZipArchiveEntry());

    result->_archive                    = zipArchive;
    result->_centralDirectoryFileHeader = cd;
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

void ZipArchiveEntry::TimestampToDateTime(time_t dateTime, uint16_t& date, uint16_t& time)
{
  tm timeStruct;

  ZAE_localtime(&dateTime, &timeStruct);

  date = ((timeStruct.tm_year - 80) << 9) + ((timeStruct.tm_mon + 1) << 5) + timeStruct.tm_mday;
  time = (timeStruct.tm_hour << 11) + (timeStruct.tm_min << 5) + (timeStruct.tm_sec >> 1);
}

time_t ZipArchiveEntry::DateTimeToTimestamp(uint16_t date, uint16_t time)
{
  tm timeStruct;

  timeStruct.tm_year = ((date >>  9) & 0x7f) + 80;
  timeStruct.tm_mon  = ((date >>  5) & 0x0f) - 1;
  timeStruct.tm_mday = ((date)       & 0x1f);

  timeStruct.tm_hour = ((time >> 11) & 0x1f);
  timeStruct.tm_min  = ((time >>  5) & 0x3f);
  timeStruct.tm_sec  = ((time <<  1) & 0x3f);

  return mktime(&timeStruct);
}

bool ZipArchiveEntry::IsValidFilename(const std::string& fullPath)
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

std::string ZipArchiveEntry::GetFilenameFromPath(const std::string& fullPath)
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

bool ZipArchiveEntry::IsDirectoryPath(const std::string& fullPath)
{
  return (fullPath.length() > 0 && fullPath.back() == '/');
}

void ZipArchiveEntry::CopyStream(std::istream& input, std::ostream& output)
{
  auto   safeBuff = ZipArchive::InternalSharedBuffer::GetInstance()->get_threadsafe_instance();
  char*  buff     = safeBuff->GetBuffer();
  size_t buffSize = safeBuff->GetBufferSize();

  assert(buff != nullptr && buffSize > 0);

  do
  {
    input.read(buff, buffSize);
    output.write(buff, input.gcount());
  } while (static_cast<size_t>(input.gcount()) == buffSize);
}

//////////////////////////////////////////////////////////////////////////
// public methods & getters & setters

const std::string& ZipArchiveEntry::GetFullName() const
{
  return _centralDirectoryFileHeader.Filename;
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

  _centralDirectoryFileHeader.Filename = correctFilename;
  this->SetAttributes(isDirectory ? Attributes::Directory : Attributes::Archive);
}

std::string ZipArchiveEntry::GetName() const
{
  return GetFilenameFromPath(_centralDirectoryFileHeader.Filename);
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
  return _centralDirectoryFileHeader.FileComment;
}

void ZipArchiveEntry::SetComment(const std::string& comment)
{
  _centralDirectoryFileHeader.FileComment = comment;
}

time_t ZipArchiveEntry::GetLastWriteTime() const
{
  return DateTimeToTimestamp(_centralDirectoryFileHeader.LastModificationDate, _centralDirectoryFileHeader.LastModificationTime);
}

void ZipArchiveEntry::SetLastWriteTime(time_t modTime)
{
  TimestampToDateTime(modTime, _centralDirectoryFileHeader.LastModificationDate, _centralDirectoryFileHeader.LastModificationTime);
}

ZipArchiveEntry::Attributes ZipArchiveEntry::GetAttributes() const
{
  return static_cast<Attributes>(_centralDirectoryFileHeader.ExternalFileAttributes);
}

ZipArchiveEntry::CompressionMethod ZipArchiveEntry::GetCompressionMethod() const
{
  return static_cast<CompressionMethod>(_centralDirectoryFileHeader.CompressionMethod);
}

void ZipArchiveEntry::SetAttributes(Attributes value)
{
  Attributes prevVal = this->GetAttributes();
  Attributes newVal = prevVal | value;

  // if we're changing from directory to file
  if (!!(prevVal & Attributes::Directory) && !!(newVal & Attributes::Archive))
  {
    newVal &= ~Attributes::Directory;

    if (IsDirectoryPath(_centralDirectoryFileHeader.Filename))
    {
      _centralDirectoryFileHeader.Filename.pop_back();
    }
  }

  // if we're changing from file to directory
  else if (!!(prevVal & Attributes::Archive) && !!(newVal & Attributes::Directory))
  {
    newVal &= ~Attributes::Archive;

    if (!IsDirectoryPath(_centralDirectoryFileHeader.Filename))
    {
      _centralDirectoryFileHeader.Filename += '/';
    }
  }

  // if this entry is directory, ensure that crc32 & sizes
  // are set to 0 and do not include any stream
  if (!!(newVal & Attributes::Directory))
  {
    _centralDirectoryFileHeader.Crc32 = 0;
    _centralDirectoryFileHeader.CompressedSize = 0;
    _centralDirectoryFileHeader.UncompressedSize = 0;
  }

  _centralDirectoryFileHeader.ExternalFileAttributes = static_cast<uint32_t>(newVal);
}

bool ZipArchiveEntry::IsPasswordProtected() const
{
  return !!(this->GetGeneralPurposeBitFlag() & BitFlag::Encrypted);
}

const std::string& ZipArchiveEntry::GetPassword() const
{
  return _password;
}

void ZipArchiveEntry::SetPassword(const std::string& password)
{
  _password = password;

  // allow unset password only for empty files
  if (!_originallyInArchive || (_hasLocalFileHeader && this->GetSize() == 0))
  {
    this->SetGeneralPurposeBitFlag(BitFlag::Encrypted, !_password.empty());
  }
}

uint32_t ZipArchiveEntry::GetCrc32() const
{
  return _centralDirectoryFileHeader.Crc32;
}

size_t ZipArchiveEntry::GetSize() const
{
  return static_cast<size_t>(_centralDirectoryFileHeader.UncompressedSize);
}

size_t ZipArchiveEntry::GetCompressedSize() const
{
  return static_cast<size_t>(_centralDirectoryFileHeader.CompressedSize);
}


bool ZipArchiveEntry::CanExtract() const
{
  return (uint16_t(this->GetVersionToExtract()) <= DEFAULT_VERSION_MADEBY);
}

bool ZipArchiveEntry::IsDirectory() const
{
  return !!(this->GetAttributes() & Attributes::Directory);
}

bool ZipArchiveEntry::IsUsingDataDescriptor() const
{
  return !!(this->GetGeneralPurposeBitFlag() & BitFlag::DataDescriptor);
}

void ZipArchiveEntry::UseDataDescriptor(bool use)
{
  this->SetGeneralPurposeBitFlag(BitFlag::DataDescriptor, use);
}

std::istream* ZipArchiveEntry::GetRawStream()
{
  if (_rawStream == nullptr)
  {
    if (_originallyInArchive)
    {
      auto offsetOfCompressedData = this->SeekToCompressedData();
      _rawStream = new isubstream(*_archive->_zipStream, offsetOfCompressedData, this->GetCompressedSize());
    }
    else
    {
      _rawStream = new isubstream(_compressionImmediateBuffer);
    }
  }

  return _rawStream;
}

std::istream* ZipArchiveEntry::GetDecompressionStream()
{
  std::istream* intermediateStream = nullptr;

  // there shouldn't be opened another stream
  if (this->CanExtract() && _openedArchive == nullptr && _zipCryptoStream == nullptr)
  {
    auto offsetOfCompressedData = this->SeekToCompressedData();
    bool needsPassword = !!(uint16_t(this->GetGeneralPurposeBitFlag()) & uint16_t(BitFlag::Encrypted));
    bool needsDecompress = this->GetCompressionMethod() != CompressionMethod::Stored;

    if (needsPassword && _password.empty())
    {
      // we need password, but we does not have it
      return nullptr;
    }

    // make correctly-ended substream of the input stream
    intermediateStream = _openedArchive = new isubstream(*_archive->_zipStream, offsetOfCompressedData, this->GetCompressedSize());

    if (needsPassword)
    {
      zip_cryptostream* cryptoStream = new zip_cryptostream(*intermediateStream, _password.c_str());
      cryptoStream->set_final_byte(this->GetLastByteOfEncryptionHeader());
      bool hasCorrectPassword = cryptoStream->prepare_for_decryption();

      if (!hasCorrectPassword)
      {
        this->CloseDecompressionStream();
        return nullptr;
      }

      intermediateStream = _zipCryptoStream = cryptoStream;
    }

    if (needsDecompress)
    {
      intermediateStream = _inflateStream = new infstream(*intermediateStream);
    }
  }

  return intermediateStream;
}

bool ZipArchiveEntry::IsRawStreamOpened() const
{
  return _rawStream != nullptr;
}

bool ZipArchiveEntry::IsDecompressionStreamOpened() const
{
  return _inflateStream != nullptr;
}

void ZipArchiveEntry::CloseRawStream()
{
  if (_rawStream != nullptr)
  {
    delete _rawStream;
    _rawStream = nullptr;
  }
}

void ZipArchiveEntry::CloseDecompressionStream()
{
  if (_inflateStream != nullptr)
  {
    delete _inflateStream;
    _inflateStream = nullptr;
  }

  if (_zipCryptoStream != nullptr)
  {
    delete _zipCryptoStream;
    _zipCryptoStream = nullptr;
  }

  if (_openedArchive != nullptr)
  {
    delete _openedArchive;
    _openedArchive = nullptr;
  }
}

bool ZipArchiveEntry::SetCompressionStream(std::istream& stream,
                                           CompressionLevel  level  /* = CompressionLevel::Default */,
                                           CompressionMethod method /* = CompressionMethod::Deflate */,
                                           CompressionMode mode /* = CompressionMode::Deferred */)
{
  // validate data
  assert((method == CompressionMethod::Deflate || method == CompressionMethod::Stored ) &&
         (mode   == CompressionMode::Deferred  || mode   == CompressionMode::Immediate));

  assert(int(level) >= 0 && int(level) <= 9);
  assert((level == CompressionLevel::Stored ? method == CompressionMethod::Stored ? true : false : true));

  // clamp compression level
  int levelAsInt = std::min(std::max(int(CompressionLevel::Stored), int(level)), int(CompressionLevel::BestCompression));  

  // force level and method to be synchronized, when one of them
  // is set to be stored
  if (level == CompressionLevel::Stored || method == CompressionMethod::Stored)
  {
    method = CompressionMethod::Stored;
    level = CompressionLevel::Stored;
  }

  // if _compressionStream is set, we already have some stream to compress
  // so we discard it
  if (_compressionStream != nullptr)
  {
    this->UnloadCompressionData();
  }

  _isNewOrChanged = true;

  _compressionStream = &stream;
  _compressionMode = mode;
  _compressionLevel = level;
  this->SetCompressionMethod(method);

  if (_compressionStream != nullptr && _compressionMode == CompressionMode::Immediate)
  {
    this->InternalCompressStream(*_compressionStream, _compressionImmediateBuffer);

    // we have everything we need, let's act like we were loaded from archive :)
    _isNewOrChanged = false;
    _compressionStream = nullptr;
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
  this->SetPassword(std::string());
}

void ZipArchiveEntry::Remove()
{
  auto it = std::find(_archive->_entries.begin(), _archive->_entries.end(), this->shared_from_this());

  if (it != _archive->_entries.end())
  {
    _archive->_entries.erase(it);
    delete this;
  }
}

//////////////////////////////////////////////////////////////////////////
// private getters & setters

void ZipArchiveEntry::SetCompressionMethod(CompressionMethod value)
{
  _centralDirectoryFileHeader.CompressionMethod = static_cast<uint16_t>(value);
}

ZipArchiveEntry::BitFlag ZipArchiveEntry::GetGeneralPurposeBitFlag() const
{
  return static_cast<BitFlag>(_centralDirectoryFileHeader.GeneralPurposeBitFlag);
}

void ZipArchiveEntry::SetGeneralPurposeBitFlag(BitFlag value, bool set)
{
  if (set)
  {
    _centralDirectoryFileHeader.GeneralPurposeBitFlag |= static_cast<uint16_t>(value);
  }
  else
  {
    _centralDirectoryFileHeader.GeneralPurposeBitFlag &= static_cast<uint16_t>(~value);
  }
}

ZipArchiveEntry::ZipVersionNeeded ZipArchiveEntry::GetVersionToExtract() const
{
  return static_cast<ZipVersionNeeded>(_centralDirectoryFileHeader.VersionNeededToExtract);
}

void ZipArchiveEntry::SetVersionToExtract(ZipVersionNeeded value)
{
  _centralDirectoryFileHeader.VersionNeededToExtract = static_cast<uint16_t>(value);
}

uint16_t ZipArchiveEntry::GetVersionMadeBy() const
{
  return _centralDirectoryFileHeader.VersionMadeBy;
}

void ZipArchiveEntry::SetVersionMadeBy(uint16_t value)
{
  _centralDirectoryFileHeader.VersionMadeBy = value;
}

int32_t ZipArchiveEntry::GetOffsetOfLocalHeader() const
{
  return _centralDirectoryFileHeader.RelativeOffsetOfLocalHeader;
}

void ZipArchiveEntry::SetOffsetOfLocalHeader(int32_t value)
{
  _centralDirectoryFileHeader.RelativeOffsetOfLocalHeader = static_cast<int32_t>(value);
}

bool ZipArchiveEntry::HasCompressionStream() const
{
  return _compressionStream != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// private working methods

void ZipArchiveEntry::FetchLocalFileHeader()
{
  if (!_hasLocalFileHeader && _originallyInArchive && _archive != nullptr)
  {
    _archive->_zipStream->seekg(this->GetOffsetOfLocalHeader(), std::ios::beg);
    _localFileHeader.Deserialize(*_archive->_zipStream);

    _offsetOfCompressedData = _archive->_zipStream->tellg();
  }

  // sync data
  this->SyncLFH_with_CDFH();
  _hasLocalFileHeader = true;
}

void ZipArchiveEntry::CheckFilenameCorrection()
{
  // this forces recheck of the filename.
  // this is useful when the check is needed after
  // deserialization
  this->SetFullName(this->GetFullName());
}

void ZipArchiveEntry::FixVersionToExtractAtLeast(ZipVersionNeeded value)
{
  if (this->GetVersionToExtract() < value)
  {
    this->SetVersionToExtract(value);
  }
}

void ZipArchiveEntry::SyncLFH_with_CDFH()
{
  _localFileHeader.VersionNeededToExtract = _centralDirectoryFileHeader.VersionNeededToExtract;
  _localFileHeader.GeneralPurposeBitFlag  = _centralDirectoryFileHeader.GeneralPurposeBitFlag;
  _localFileHeader.CompressionMethod      = _centralDirectoryFileHeader.CompressionMethod;
  _localFileHeader.LastModificationTime   = _centralDirectoryFileHeader.LastModificationTime;
  _localFileHeader.LastModificationDate   = _centralDirectoryFileHeader.LastModificationDate;
  _localFileHeader.Crc32                  = _isNewOrChanged ? 0 : _centralDirectoryFileHeader.Crc32;
  _localFileHeader.CompressedSize         = _isNewOrChanged ? 0 : _centralDirectoryFileHeader.CompressedSize;
  _localFileHeader.UncompressedSize       = _isNewOrChanged ? 0 : _centralDirectoryFileHeader.UncompressedSize;

  _localFileHeader.Filename               = _centralDirectoryFileHeader.Filename;
  _localFileHeader.FilenameLength         = static_cast<uint16_t>(_localFileHeader.Filename.length());
}

void ZipArchiveEntry::SyncCDFH_with_LFH()
{
  _centralDirectoryFileHeader.Crc32            = _localFileHeader.Crc32;
  _centralDirectoryFileHeader.CompressedSize   = _localFileHeader.CompressedSize;
  _centralDirectoryFileHeader.UncompressedSize = _localFileHeader.UncompressedSize;

  _centralDirectoryFileHeader.FilenameLength   = static_cast<uint16_t>(_centralDirectoryFileHeader.Filename.length());
  _centralDirectoryFileHeader.FileCommentLength = static_cast<uint16_t>(_centralDirectoryFileHeader.FileComment.length());

  this->FixVersionToExtractAtLeast(this->GetCompressionMethod() == CompressionMethod::Deflate || this->IsDirectory()
    ? ZipVersionNeeded::DeflateAndExplicitDirectory
    : ZipVersionNeeded::Default);
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
    if (_compressionStream == nullptr)
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
      compressedDataStream = _compressionStream;
    }
  }

  if (!_hasLocalFileHeader)
  {
    this->FetchLocalFileHeader();
  }


  // save offset of stream here
  _offsetOfSerializedLocalFileHeader = stream.tellp();      

  if (this->IsUsingDataDescriptor())
  {
    _localFileHeader.CompressedSize = 0;
    _localFileHeader.UncompressedSize = 0;
    _localFileHeader.Crc32 = 0;
  }

  _localFileHeader.Serialize(stream);

  // if this entry is a directory, it should not contain any data
  // nor crc.
  assert(
    this->IsDirectory()
    ? !GetCrc32() && !GetCompressedSize() && !GetSize() && !_compressionStream
    : true
  );


  if (!this->IsDirectory() && compressedDataStream != nullptr)
  {
    if (_isNewOrChanged)
    {
      this->InternalCompressStream(*compressedDataStream, stream);

      if (this->IsUsingDataDescriptor())
      {
        _localFileHeader.SerializeAsDataDescriptor(stream);
      }
      else
      {
        // actualize local file header
        // make non-seekable version?
        stream.seekp(_offsetOfSerializedLocalFileHeader);
        _localFileHeader.Serialize(stream);
        stream.seekp(this->GetCompressedSize(), std::ios::cur);
      }
    }
    else
    {
      CopyStream(*compressedDataStream, stream);
    }
  }
}

void ZipArchiveEntry::SerializeCentralDirectoryFileHeader(std::ostream& stream)
{
  _centralDirectoryFileHeader.RelativeOffsetOfLocalHeader = static_cast<int32_t>(_offsetOfSerializedLocalFileHeader);
  _centralDirectoryFileHeader.Serialize(stream);
}

void ZipArchiveEntry::UnloadCompressionData()
{
  // unload stream
  _compressionImmediateBuffer.clear();
  _compressionStream = nullptr;

  _centralDirectoryFileHeader.CompressedSize = 0;
  _centralDirectoryFileHeader.UncompressedSize = 0;
  _centralDirectoryFileHeader.Crc32 = 0;
}

void ZipArchiveEntry::InternalCompressStream(std::istream& inputStream, std::ostream& outputStream)
{
  std::ostream*    intermediateStream = &outputStream;
  zip_cryptostream cryptoStream;
  defstream        deflateStream;
  crc32stream      crc32Stream;
  teestream        multistream;

  // TODO: make write for non-seekable output streams?
  std::ios::pos_type dataOrigin = outputStream.tellp();

  if (!_password.empty())
  {
    this->SetGeneralPurposeBitFlag(BitFlag::Encrypted);

    cryptoStream.init(outputStream, _password.c_str());
    cryptoStream.set_final_byte(this->GetLastByteOfEncryptionHeader());
    intermediateStream = &cryptoStream;
  }

  if (this->GetCompressionMethod() != CompressionMethod::Stored)
  {
    deflateStream.init(*intermediateStream, static_cast<int>(_compressionLevel));
    intermediateStream = &deflateStream;
  }

  multistream.bind(crc32Stream);
  multistream.bind(*intermediateStream);

  // compress the stream
  CopyStream(inputStream, multistream);

  // flushing is not needed

  if (this->GetCompressionMethod() != CompressionMethod::Stored)
  {
    deflateStream.flush();
    _localFileHeader.UncompressedSize = static_cast<uint32_t>(deflateStream.get_bytes_read());
    _localFileHeader.CompressedSize   = static_cast<uint32_t>(outputStream.tellp() - dataOrigin);
  }
  else
  {
    _localFileHeader.UncompressedSize = static_cast<uint32_t>(outputStream.tellp() - dataOrigin);
    _localFileHeader.CompressedSize   = static_cast<uint32_t>(outputStream.tellp() - dataOrigin);
  }

  _localFileHeader.Crc32 = crc32Stream.get_crc32();

  this->SyncCDFH_with_LFH();
}

void ZipArchiveEntry::FigureCrc32()
{
  if (this->IsDirectory() || _compressionStream == nullptr || !_isNewOrChanged)
  {
    return;
  }

  // stream must be seekable
  auto position = _compressionStream->gcount();

  // compute crc32
  crc32stream crc32Stream;
  CopyStream(*_compressionStream, crc32Stream);
  crc32Stream.flush();

  // seek back
  _compressionStream->clear();
  _compressionStream->seekg(position);

  _centralDirectoryFileHeader.Crc32 = crc32Stream.get_crc32();
}

uint8_t ZipArchiveEntry::GetLastByteOfEncryptionHeader()
{
  if (!!(this->GetGeneralPurposeBitFlag() & BitFlag::DataDescriptor))
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
    return uint8_t((_centralDirectoryFileHeader.LastModificationTime >> 8) & 0xff);
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
