#include "stdafx.h"
#include "SevenZipTool.h"

#define SEVENZIP_TOOL_PATH "..\\Tools\\7za.exe"

namespace ZipLibTest {
  
namespace {

  std::vector<std::string> GetCommandLineContents(const std::string& cmdline)
  {
    std::vector<std::string> result;

    FILE* p = _popen(cmdline.c_str(), "r");

    char line[128];
    std::string strLine;
    while (fgets(line, 80, p) && !feof(p))
    {
      strLine += line;

      if (strLine.rfind('\n') == strLine.size() - 1)
      {
        result.push_back(std::move(strLine));
      }
    }

    return result;
  }

  std::string GetStringFromRegex(const std::string& input, const std::string& pattern)
  {
    std::regex regex(pattern);
    std::smatch match;

    std::regex_match(input, match, regex);

    return std::string(match[1].first, match[1].second);
  }

}

bool SevenZipTool::Open(const std::string& archivePath)
{
  _archivePath = archivePath;

  auto cmdLineOutput = GetCommandLineContents(SEVENZIP_TOOL_PATH " l -slt " + _archivePath);

  auto it = cmdLineOutput.begin();
  if (cmdLineOutput.size() > 8 && it[8].find("Comment") == 0)
  {
    _comment = GetStringFromRegex(it[8], "Comment = (.*)\n");
    it += 12;
  }
  else if (cmdLineOutput.size() > 6)
  {
    it += 11;
  }
  else
  {
    return false;
  }

  for (; it < cmdLineOutput.end(); it += 15)
  {
    ZipEntryInfo entryInfo;

    entryInfo.FileName          =            GetStringFromRegex(it[0],  "Path = (.*)\n");
    entryInfo.IsDirectory       =            GetStringFromRegex(it[1],  "Folder = (.*)\n") == "+";
    entryInfo.UncompressedSize  = std::stoul(GetStringFromRegex(it[2],  "Size = (.*)\n"));
    entryInfo.CompressedSize    = std::stoul(GetStringFromRegex(it[3],  "Packed Size = (.*)\n"));
    entryInfo.Comment           =            GetStringFromRegex(it[9],  "Comment = (.*)\n");

    std::string crc32str = GetStringFromRegex(it[10], "CRC = (.*)\n");

    if (!crc32str.empty())
    {
      entryInfo.Crc32 = std::stoul(crc32str, nullptr, 16);
    }

    std::string compressionMethod = GetStringFromRegex(it[11], "Method = (.*)\n");
    std::string encryptionMethod;

    std::string::size_type spacePos = compressionMethod.find(' ');

    if (compressionMethod.find(' ') != std::string::npos)
    {
      encryptionMethod = compressionMethod.substr(0, spacePos);
      compressionMethod = compressionMethod.substr(spacePos + 1);
    }

    entryInfo.CompressionMethod =
      compressionMethod == "Deflate" ? ZipCompressionMethod::Deflate :
      compressionMethod == "BZip2"   ? ZipCompressionMethod::Bzip2   :
      compressionMethod == "LZMA"    ? ZipCompressionMethod::Lzma    :
                                       ZipCompressionMethod::Store;
    entryInfo.EncryptionMethod =
      encryptionMethod == "ZipCrypto" ? ZipEncryptionMethod::ZipCrypto:
      encryptionMethod == "AES-128"   ? ZipEncryptionMethod::Aes128 :
      encryptionMethod == "AES-192"   ? ZipEncryptionMethod::Aes192 :
      encryptionMethod == "AES-256"   ? ZipEncryptionMethod::Aes256 :
                                        ZipEncryptionMethod::None;

    _entries.push_back(entryInfo);
  }

  return true;
}

bool SevenZipTool::AddFile(const std::string& filePath, ZipCompressionMethod compressionMethod, const std::string& password, ZipEncryptionMethod encryptionMethod)
{
  return this->AddFile(filePath, "", compressionMethod, password, encryptionMethod);
}

bool SevenZipTool::AddFile(const std::string& filePath, const std::string& inArchiveName, ZipCompressionMethod compressionMethod, const std::string& password, ZipEncryptionMethod encryptionMethod)
{
  bool hasPassword = !password.empty() && encryptionMethod != ZipEncryptionMethod::None;

  char cmd[100];

  if (hasPassword)
  {
      sprintf(cmd, SEVENZIP_TOOL_PATH " a %s %s -mm=%s -p%s -mem=%s", 
        _archivePath.c_str(),
        filePath.c_str(),
        compressionMethod == ZipCompressionMethod::Deflate  ? "Deflate" :
          compressionMethod == ZipCompressionMethod::Bzip2  ? "BZip2"   :
          compressionMethod == ZipCompressionMethod::Lzma   ? "LZMA"    :
                                                              "Copy",
                                                          
        password.c_str(),
        encryptionMethod == ZipEncryptionMethod::Aes128   ? "AES128" :
          encryptionMethod == ZipEncryptionMethod::Aes192 ? "AES192" :
          encryptionMethod == ZipEncryptionMethod::Aes256 ? "AES256" :
                                                            "ZipCrypto"
      );
  }
  else
  {
    sprintf(cmd, SEVENZIP_TOOL_PATH " a %s %s -mm=%s",
      _archivePath.c_str(),
      filePath.c_str(),
      compressionMethod == ZipCompressionMethod::Deflate  ? "Deflate" :
        compressionMethod == ZipCompressionMethod::Bzip2  ? "BZip2"   :
        compressionMethod == ZipCompressionMethod::Lzma   ? "LZMA"    :
                                                            "Copy"
    );
  }

  auto cmdLineOutput = GetCommandLineContents(cmd);

  return cmdLineOutput.back().find("Everything is Ok") == 0 && this->Open(_archivePath);
}

bool SevenZipTool::ExtractFile(const std::string& inArchiveName, const std::string& password)
{
  return this->ExtractFile(inArchiveName, "", password);
}

bool SevenZipTool::ExtractFile(const std::string& inArchiveName, const std::string& destinationPath, const std::string& password)
{
  char cmd[100];
  bool hasPassword = !password.empty();

  if (hasPassword)
  {
    sprintf(cmd, SEVENZIP_TOOL_PATH " e %s %s -p%s",
      _archivePath.c_str(),
      inArchiveName.c_str(),
      password.c_str()
    );
  }
  else
  {
    sprintf(cmd, SEVENZIP_TOOL_PATH " e %s %s",
      _archivePath.c_str(),
      inArchiveName.c_str()
    );
  }

  auto cmdLineOutput = GetCommandLineContents(cmd);

  return cmdLineOutput[cmdLineOutput.size() - 4].find("Everything is Ok") == 0;
}

bool SevenZipTool::RemoveFile(const std::string& inArchiveName)
{
  return false;
}

const std::string& SevenZipTool::GetComment() const
{
  return _comment;
}

const std::vector<ZipEntryInfo>& SevenZipTool::GetEntries() const
{
  // C++11 should call move ctor.
  return _entries;
}

}
