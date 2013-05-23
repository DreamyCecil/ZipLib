#include "../ZipLib/ZipFile.h"

static const char* zipFilename = "archive.zip";
static const char* fileIn1     = "in1.jpg";
static const char* fileIn2     = "in2.png";
static const char* fileOut1    = "out1.jpg";
static const char* fileOut2    = "out2.png";

int main()
{
  ZipFile::AddFile(zipFilename, fileIn1);
  ZipFile::AddEncryptedFile(zipFilename, fileIn2, "pass");

  ZipFile::ExtractFile(zipFilename, fileIn1, fileOut1);
  ZipFile::ExtractEncryptedFile(zipFilename, fileIn2, fileOut2, "pass");

  return 0;
}
