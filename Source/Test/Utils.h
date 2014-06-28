#pragma once
#include <istream>
#include <ostream>
#include <string>
#include "ZipTools/IZipTool.h"
#include "../ZipLib/ZipArchiveEntry.h"

namespace ZipLibTest {

void GenerateFile(const std::string& fileName, size_t fileSize);
void SetCurrentWorkingDirectory(const std::string& dir);
void EmptyTestDirectory();

bool StreamContentEquals(std::istream& stream1, std::istream& stream2);

std::ostream& operator << (std::ostream& os, ZipCompressionMethod compressionMethod);
std::ostream& operator << (std::ostream& os, ZipEncryptionMethod encryptionMethod);
std::ostream& operator << (std::ostream& os, ZipArchiveEntry::CompressionMode compressionMode);

}
