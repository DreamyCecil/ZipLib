#ifndef WIN32
#include <unistd.h>
#else
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <fstream>
#include <cassert>
#include <stdexcept>
#include "ZipFile.h"

namespace
{
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
}

ZipArchive ZipFile::Open(const std::string& zipName)
{
  std::ifstream* zipFile = new std::ifstream();
  zipFile->open(zipName, std::ios::binary);

  if (!zipFile->is_open())
  {
    // if file does not exist, try to create it
    std::ofstream tmpFile;
    tmpFile.open(zipName, std::ios::binary);
    tmpFile.close();

    zipFile->open(zipName, std::ios::binary);

    // if attempt to create file failed, throw an exception
    if (!zipFile->is_open())
    {
      throw std::runtime_error("cannot open zip file");
    }
  }

  return ZipArchive(zipFile, true);
}

bool ZipFile::IsInArchive(const std::string& zipName, const std::string& fileName)
{
  ZipArchive zipArchive = ZipFile::Open(zipName);
  return zipArchive->GetEntry(fileName) != nullptr;
}

void ZipFile::AddFile(const std::string& zipName, const std::string& fileName, ZipArchiveEntry::CompressionLevel level)
{
  AddFile(zipName, fileName, GetFilenameFromPath(fileName), level);
}

void ZipFile::AddFile(const std::string& zipName, const std::string& fileName, const std::string& inArchiveName, ZipArchiveEntry::CompressionLevel level)
{
  AddEncryptedFile(zipName, fileName, inArchiveName, std::string(), level);
}

void ZipFile::AddEncryptedFile(const std::string& zipName, const std::string& fileName, const std::string& password, ZipArchiveEntry::CompressionLevel level)
{
  AddEncryptedFile(zipName, fileName, GetFilenameFromPath(fileName), std::string(), level);
}

void ZipFile::AddEncryptedFile(const std::string& zipName, const std::string& fileName, const std::string& inArchiveName, const std::string& password, ZipArchiveEntry::CompressionLevel level)
{
  std::string tmpName(L_tmpnam, '\0');

  {
    ZipArchive zipArchive = ZipFile::Open(zipName);

    std::ifstream fileToAdd;
    fileToAdd.open(fileName, std::ios::binary);

    if (!fileToAdd.is_open())
    {
      throw std::runtime_error("cannot open input file");
    }

    auto* fileEntry = zipArchive->CreateEntry(inArchiveName);

    if (fileEntry == nullptr)
    {
      //throw std::runtime_error("input file already exist in the archive");
      zipArchive.RemoveEntry(inArchiveName);
      fileEntry = zipArchive->CreateEntry(inArchiveName);
    }

    if (!password.empty())
    {
      fileEntry->SetPassword(password);
      fileEntry->UseDataDescriptor();
    }

    fileEntry->SetCompressionStream(&fileToAdd, level);

    //////////////////////////////////////////////////////////////////////////

    std::ofstream outFile;

#ifndef WIN32
    tmpName = "/tmp/.zlXXXXX";
    mkstemp(&tmpName[0]);
#else
    tmpnam(&tmpName[0]);
#endif

    outFile.open(tmpName, std::ios::binary);

    if (!outFile.is_open())
    {
      throw std::runtime_error("cannot open output file");
    }

    zipArchive.WriteToStream(outFile);
    outFile.close();
  
    // force closing the input zip stream
  }


  remove(zipName.c_str());
  rename(tmpName.c_str(), zipName.c_str());
}

void ZipFile::ExtractFile(const std::string& zipName, const std::string& fileName)
{
  ExtractFile(zipName, fileName, GetFilenameFromPath(fileName));
}

void ZipFile::ExtractFile(const std::string& zipName, const std::string& fileName, const std::string& destinationPath)
{
  ExtractEncryptedFile(zipName, fileName, destinationPath, std::string());
}

void ZipFile::ExtractEncryptedFile(const std::string& zipName, const std::string& fileName, const std::string& password)
{
  ExtractEncryptedFile(zipName, fileName, GetFilenameFromPath(fileName), password);
}

void ZipFile::ExtractEncryptedFile(const std::string& zipName, const std::string& fileName, const std::string& destinationPath, const std::string& password)
{
  ZipArchive zipArchive = ZipFile::Open(zipName);

  std::ofstream destFile;
  destFile.open(destinationPath, std::ios::binary | std::ios::trunc);

  if (!destFile.is_open())
  {
    throw std::runtime_error("cannot create destination file");
  }

  ZipArchiveEntry* entry = zipArchive->GetEntry(fileName);

  if (entry == nullptr)
  {
    throw std::runtime_error("file is not contained in zip file");
  }

  if (!password.empty())
  {
    entry->SetPassword(password);
  }

  std::istream* dataStream = entry->GetDecompressionStream();

  if (dataStream == nullptr)
  {
    throw std::runtime_error("wrong password");
  }

  std::copy(
    std::istreambuf_iterator<char>(*dataStream),
    std::istreambuf_iterator<char>(), // eof
    std::ostreambuf_iterator<char>(destFile)
  );

  destFile.flush();
  destFile.close();
}

