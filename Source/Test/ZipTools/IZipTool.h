#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ZipLibTest {

enum class ZipCompressionMethod
{
  Store,
  Deflate,
  Bzip2,
  Lzma
};

enum class ZipEncryptionMethod
{
  None,
  ZipCrypto,
  Aes128,
  Aes192,
  Aes256
};

struct ZipEntryInfo
{
  std::string           FileName;
  size_t                UncompressedSize;
  size_t                CompressedSize;
  ZipCompressionMethod  CompressionMethod;
  ZipEncryptionMethod   EncryptionMethod;
  uint32_t              Crc32;
  std::string           Comment;
  bool                  IsDirectory;

  ZipEntryInfo()
    : UncompressedSize(0)
    , CompressedSize(0)
    , CompressionMethod(ZipCompressionMethod::Store)
    , EncryptionMethod(ZipEncryptionMethod::None)
    , Crc32(0)
    , IsDirectory(false)
  {

  }
};

class IZipTool
{
  public:
    virtual ~IZipTool() { }

    virtual bool Open(
      const std::string& archivePath) = 0;

    virtual bool AddFile(
      const std::string& filePath,
      const std::string& inArchiveName,
      ZipCompressionMethod compressionMethod,
      const std::string& password,
      ZipEncryptionMethod encryptionMethod) = 0;

    virtual bool ExtractFile(
      const std::string& inArchiveName,
      const std::string& destinationPath,
      const std::string& password) = 0;

    virtual bool RemoveFile(
      const std::string& inArchiveName) = 0;

    virtual const std::string& GetComment() const = 0;

    virtual const std::vector<ZipEntryInfo>& GetEntries() const = 0;
};

}