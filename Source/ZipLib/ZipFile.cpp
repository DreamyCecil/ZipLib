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

  std::string MakeTempFilename(const std::string& fileName)
  {
    return fileName + ".tmp";
  }
}

ZipArchive ZipFile::Open(const std::string& zipPath)
{
  std::ifstream* zipFile = new std::ifstream();
  zipFile->open(zipPath, std::ios::binary);

  if (!zipFile->is_open())
  {
    // if file does not exist, try to create it
    std::ofstream tmpFile;
    tmpFile.open(zipPath, std::ios::binary);
    tmpFile.close();

    zipFile->open(zipPath, std::ios::binary);

    // if attempt to create file failed, throw an exception
    if (!zipFile->is_open())
    {
      throw std::runtime_error("cannot open zip file");
    }
  }

  return ZipArchive(zipFile, true);
}

void ZipFile::Save(ZipArchive& zipArchive, const std::string& zipPath)
{
  ZipFile::SaveAndClose(zipArchive, zipPath);

  // the operator = (ZipArchive&&) method will be called
  zipArchive = ZipFile::Open(zipPath);
}

void ZipFile::SaveAndClose(ZipArchive& zipArchive, const std::string& zipPath)
{
  // check if file exist
  std::string tempZipPath = MakeTempFilename(zipPath);
  std::ofstream outZipFile;
  outZipFile.open(tempZipPath, std::ios::binary | std::ios::trunc);

  if (!outZipFile.is_open())
  {
    throw std::runtime_error("cannot save zip file");
  }

  zipArchive.WriteToStream(outZipFile);
  outZipFile.close();

  {
    // force destroying original archive
    // (it may hold the handle on the original zip archive)
    ZipArchive tempZipArchive;
    zipArchive.Swap(tempZipArchive);
  }

  remove(zipPath.c_str());
  rename(tempZipPath.c_str(), zipPath.c_str());
}

bool ZipFile::IsInArchive(const std::string& zipPath, const std::string& fileName)
{
  ZipArchive zipArchive = ZipFile::Open(zipPath);
  return zipArchive.GetEntry(fileName) != nullptr;
}

void ZipFile::AddFile(const std::string& zipPath, const std::string& fileName, ZipArchiveEntry::CompressionLevel level)
{
  AddFile(zipPath, fileName, GetFilenameFromPath(fileName), level);
}

void ZipFile::AddFile(const std::string& zipPath, const std::string& fileName, const std::string& inArchiveName, ZipArchiveEntry::CompressionLevel level)
{
  AddEncryptedFile(zipPath, fileName, inArchiveName, std::string(), level);
}

void ZipFile::AddEncryptedFile(const std::string& zipPath, const std::string& fileName, const std::string& password, ZipArchiveEntry::CompressionLevel level)
{
  AddEncryptedFile(zipPath, fileName, GetFilenameFromPath(fileName), std::string(), level);
}

void ZipFile::AddEncryptedFile(const std::string& zipPath, const std::string& fileName, const std::string& inArchiveName, const std::string& password, ZipArchiveEntry::CompressionLevel level)
{
  std::string tmpName = MakeTempFilename(zipPath);

  {
    ZipArchive zipArchive = ZipFile::Open(zipPath);

    std::ifstream fileToAdd;
    fileToAdd.open(fileName, std::ios::binary);

    if (!fileToAdd.is_open())
    {
      throw std::runtime_error("cannot open input file");
    }

    auto* fileEntry = zipArchive.CreateEntry(inArchiveName);

    if (fileEntry == nullptr)
    {
      //throw std::runtime_error("input file already exist in the archive");
      zipArchive.RemoveEntry(inArchiveName);
      fileEntry = zipArchive.CreateEntry(inArchiveName);
    }

    if (!password.empty())
    {
      fileEntry->SetPassword(password);
      fileEntry->UseDataDescriptor();
    }

    fileEntry->SetCompressionStream(&fileToAdd, level);

    //////////////////////////////////////////////////////////////////////////

    std::ofstream outFile;
    outFile.open(tmpName, std::ios::binary);

    if (!outFile.is_open())
    {
      throw std::runtime_error("cannot open output file");
    }

    zipArchive.WriteToStream(outFile);
    outFile.close();
  
    // force closing the input zip stream
  }

  remove(zipPath.c_str());
  rename(tmpName.c_str(), zipPath.c_str());
}

void ZipFile::ExtractFile(const std::string& zipPath, const std::string& fileName)
{
  ExtractFile(zipPath, fileName, GetFilenameFromPath(fileName));
}

void ZipFile::ExtractFile(const std::string& zipPath, const std::string& fileName, const std::string& destinationPath)
{
  ExtractEncryptedFile(zipPath, fileName, destinationPath, std::string());
}

void ZipFile::ExtractEncryptedFile(const std::string& zipPath, const std::string& fileName, const std::string& password)
{
  ExtractEncryptedFile(zipPath, fileName, GetFilenameFromPath(fileName), password);
}

void ZipFile::ExtractEncryptedFile(const std::string& zipPath, const std::string& fileName, const std::string& destinationPath, const std::string& password)
{
  ZipArchive zipArchive = ZipFile::Open(zipPath);

  std::ofstream destFile;
  destFile.open(destinationPath, std::ios::binary | std::ios::trunc);

  if (!destFile.is_open())
  {
    throw std::runtime_error("cannot create destination file");
  }

  ZipArchiveEntry* entry = zipArchive.GetEntry(fileName);

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

  ZipArchiveEntry::CopyStream(*dataStream, destFile);

  destFile.flush();
  destFile.close();
}

void ZipFile::RemoveEntry(const std::string& zipPath, const std::string& fileName)
{
  std::string tmpName = MakeTempFilename(zipPath);

  {
    ZipArchive zipArchive = ZipFile::Open(zipPath);
    zipArchive.RemoveEntry(fileName);

    //////////////////////////////////////////////////////////////////////////

    std::ofstream outFile;

    outFile.open(tmpName, std::ios::binary);

    if (!outFile.is_open())
    {
      throw std::runtime_error("cannot open output file");
    }

    zipArchive.WriteToStream(outFile);
    outFile.close();

    // force closing the input zip stream
  }

  remove(zipPath.c_str());
  rename(tmpName.c_str(), zipPath.c_str());
}
