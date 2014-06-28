#pragma once
#include "IZipTool.h"

namespace ZipLibTest {

class SevenZipTool
  : public IZipTool
{
  public:
    bool Open(
      const std::string& archivePath) override;

    bool AddFile(
      const std::string& filePath,
      ZipCompressionMethod compressionMethod = ZipCompressionMethod::Deflate,
      const std::string& password = "",
      ZipEncryptionMethod encryptionMethod = ZipEncryptionMethod::None);

    bool AddFile(
      const std::string& filePath,
      const std::string& inArchiveName = "",
      ZipCompressionMethod compressionMethod = ZipCompressionMethod::Deflate,
      const std::string& password = "",
      ZipEncryptionMethod encryptionMethod = ZipEncryptionMethod::None) override;

    bool ExtractFile(
      const std::string& inArchiveName,
      const std::string& password = "");

    bool ExtractFile(
      const std::string& inArchiveName,
      const std::string& destinationPath,
      const std::string& password) override;

    bool RemoveFile(
      const std::string& inArchiveName) override;

    const std::string& GetComment() const override;

    const std::vector<ZipEntryInfo>& GetEntries() const override;

  private:
    std::string _archivePath;
    std::string _comment;
    std::vector<ZipEntryInfo> _entries;
};

}
