#include "stdafx.h"

namespace ZipLibTest {

void GenerateFile(const std::string& fileName, size_t fileSize)
{
  const size_t bufferSize = 1024 * 1024;

  std::ofstream of(fileName, std::ios::binary);
  std::vector<char> buffer(bufferSize);

  auto localGenerate = [&]()
  {
    std::generate(buffer.begin(), buffer.end(), []() -> char
    {
      const size_t repeatBufferSize = 7 * 1024;
      const size_t repeatBufferFrequency = 10000;
      static size_t generated = 0;
      static size_t repeatCount = 0;
      static size_t inRepeat = false;

      if (inRepeat)
      {
        if (++repeatCount >= repeatBufferSize)
        {
          repeatCount = 0;
          inRepeat = false;
        }

        return 'A';
      }
      else
      {
        if (!(++generated % repeatBufferFrequency))
        {
          inRepeat = true;
        }

        return rand();
      }
    });
  };

  size_t toWrite = fileSize;
  while (toWrite)
  {
    localGenerate();
    size_t blockSize = std::min(bufferSize, toWrite);
    of.write(buffer.data(), blockSize);
    toWrite -= blockSize;
  }

  of.close();
}

void SetCurrentWorkingDirectory(const std::string& dir)
{
  SetCurrentDirectoryA(dir.c_str());
}

void EmptyTestDirectory()
{
  system("del /Q .\\*");
}

bool StreamContentEquals(std::istream& stream1, std::istream& stream2)
{
  std::array<char, 32 * 1024> buff1;
  std::array<char, 32 * 1024> buff2;

  do
  {
    stream1.read(buff1.data(), buff1.size());
    stream2.read(buff2.data(), buff2.size());
  } while (
    stream1.gcount() == stream2.gcount() &&
    (stream1.gcount() != 0 && stream2.gcount() != 0) &&
    buff1 == buff2
  );

  return stream1.eof() && stream2.eof();
}

std::ostream& operator << (std::ostream& os, ZipCompressionMethod compressionMethod)
{
  switch (compressionMethod)
  {
    case ZipCompressionMethod::Store:   os << "Store";    break;
    case ZipCompressionMethod::Deflate: os << "Deflate";  break;
    case ZipCompressionMethod::Bzip2:   os << "Bzip2";    break;
    case ZipCompressionMethod::Lzma:    os << "Lzma";     break;
  }

  return os;
}

std::ostream& operator << (std::ostream& os, ZipEncryptionMethod encryptionMethod)
{
  switch (encryptionMethod)
  {
    case ZipEncryptionMethod::None:       os << "None";       break;
    case ZipEncryptionMethod::ZipCrypto:  os << "ZipCrypto";  break;
    case ZipEncryptionMethod::Aes128:     os << "AES-128";    break;
    case ZipEncryptionMethod::Aes192:     os << "AES-192";    break;
    case ZipEncryptionMethod::Aes256:     os << "AES-256";    break;
  }

  return os;
}

std::ostream& operator << (std::ostream& os, ZipArchiveEntry::CompressionMode compressionMode)
{
  switch (compressionMode)
  {
    case ZipArchiveEntry::CompressionMode::Immediate: os << "Immediate";  break;
    case ZipArchiveEntry::CompressionMode::Deferred:  os << "Deferred";   break;
  }

  return os;
}

}
