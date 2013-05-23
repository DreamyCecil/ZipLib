#pragma once
#include <string>
#include "ZipArchive.h"

class ZipFile
{
  public:
    // TODO:
    // CreateFromDirectory + compression level + opening to/from stream support
    // ExtractToDirectory
    // AddFile - support overwriting of file

    static ZipArchive Open(const std::string& zipName);

    static bool IsInArchive(const std::string& zipName, const std::string& fileName);

    static void AddFile(const std::string& zipName, const std::string& fileName, ZipArchiveEntry::CompressionLevel level = ZipArchiveEntry::CompressionLevel::Default);
    static void AddFile(const std::string& zipName, const std::string& fileName, const std::string& inArchiveName, ZipArchiveEntry::CompressionLevel level = ZipArchiveEntry::CompressionLevel::Default);

    static void AddEncryptedFile(const std::string& zipName, const std::string& fileName, const std::string& password, ZipArchiveEntry::CompressionLevel level = ZipArchiveEntry::CompressionLevel::Default);
    static void AddEncryptedFile(const std::string& zipName, const std::string& fileName, const std::string& inArchiveName, const std::string& password, ZipArchiveEntry::CompressionLevel level = ZipArchiveEntry::CompressionLevel::Default);

    static void ExtractFile(const std::string& zipName, const std::string& fileName);
    static void ExtractFile(const std::string& zipName, const std::string& fileName, const std::string& destinationPath);

    static void ExtractEncryptedFile(const std::string& zipName, const std::string& fileName, const std::string& password);
    static void ExtractEncryptedFile(const std::string& zipName, const std::string& fileName, const std::string& destinationPath, const std::string& password);
};
