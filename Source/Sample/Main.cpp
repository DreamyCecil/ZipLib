#include "../ZipLib/ZipFile.h"
#include "../ZipLib/streams/memstream.h"
#include <fstream>

static const char* zipFilename  = "archive.zip";
static const char* zipFilename2 = "archive_helper.zip";

static const char* fileIn1     = "in1.jpg";
static const char* fileIn2     = "in2.png";
static const char* fileIn3     = "in3.txt";
static const char* fileIn4     = "by_memstream.txt";
static const char* fileIn2Dest = "dir/in2.png";

static const char* fileOut1    = "out1.jpg";
static const char* fileOut2    = "out2.png";
static const char* fileOut3    = "out3.txt";

void Sample_ZipFile()
{
  try
  {
    printf("Compressing '%s'\n", fileIn1);
    ZipFile::AddFile(zipFilename, fileIn1);

    printf("Compressing & encrypting '%s' as '%s'\n", fileIn2, fileIn2Dest);
    ZipFile::AddEncryptedFile(zipFilename, fileIn2, fileIn2Dest, "pass");

    printf("Extracting '%s' as '%s'\n", fileIn1, fileOut1);
    ZipFile::ExtractFile(zipFilename, fileIn1, fileOut1);

    printf("Extracting & decrypting '%s' as '%s'\n", fileIn2Dest, fileOut2);
    ZipFile::ExtractEncryptedFile(zipFilename, fileIn2Dest, fileOut2, "pass");
  }
  catch (std::exception& e)
  {
    printf("An exception occured: '%s'\n", e.what());
  }
}

void Sample_ZipArchive_Listing()
{
  ZipArchive archive = ZipFile::Open(zipFilename);
  printf("\n");

  printf("Listing the content of the archive '%s'\n", zipFilename);
  printf("Count: '%u'\n", archive->GetEntriesCount());
  for (size_t i = 0; i < archive->GetEntriesCount(); ++i)
  {
    printf("Full path: '%s'\n\tCompressed size: %u\n\tUncompressed size: %u\n\tCRC32: %08x\n",
      archive->GetEntry(i)->GetFullName().c_str(),
      archive->GetEntry(i)->GetCompressedSize(),
      archive->GetEntry(i)->GetSize(),
      archive->GetEntry(i)->GetCrc32()
    );
  }
}

void Sample_ZipArchive_StoreFileImmediate()
{
  ZipArchive archive = ZipFile::Open(zipFilename);
  printf("\n");

  printf("Compressing file '%s' immediately\n", fileIn3);
  {
    std::ifstream f(fileIn3, std::ios::binary);

    ZipArchiveEntry* entry = archive->CreateEntry(fileIn3);
    if (entry != nullptr)
    {
      entry->SetCompressionStream(&f,
        ZipArchiveEntry::CompressionLevel::Stored,
        ZipArchiveEntry::CompressionMethod::Stored,
        ZipArchiveEntry::CompressionMode::Immediate
      );
    }
  }

  // thanks to immediate mode, we can save the zip archive
  // even though the original input stream is destroyed here
  std::ofstream z2(zipFilename2, std::ios::binary);
  archive->WriteToStream(z2);
  z2.close();
}

void Sample_ZipArchive_RemoveEntry()
{
  ZipArchive archive = ZipFile::Open(zipFilename2);
  printf("\n");

  printf("Removing file '%s' from '%s' and saving to '%s'\n", fileIn2Dest, zipFilename2, zipFilename);
  archive->RemoveEntry(fileIn2Dest);

  // thanks to immediate mode, we can save the zip archive
  // even though the original input stream is destroyed here
  std::ofstream z2(zipFilename, std::ios::binary);
  archive->WriteToStream(z2);

  z2.close();
}

void Sample_ZipArchive_CompressFileDeferred()
{
  ZipArchive archive = ZipFile::Open(zipFilename);
  printf("\n");

  printf("Compressing file '%s' deferred\n", fileIn2);
  std::ifstream f(fileIn2, std::ios::binary);

  ZipArchiveEntry* entry = archive->CreateEntry(fileIn2Dest);
  if (entry != nullptr)
  {
    entry->SetCompressionStream(&f,
      ZipArchiveEntry::CompressionLevel::BestCompression,
      ZipArchiveEntry::CompressionMethod::Deflate,
      ZipArchiveEntry::CompressionMode::Deferred
    );
  }

  // when deferred mode is chosen, the input stream ("f" here)
  // must be still alive
  printf("Saving into '%s'\n", zipFilename2);
  std::ofstream z2(zipFilename2, std::ios::binary);
  archive->WriteToStream(z2);
  z2.close();
}

void Sample_ZipArchive_CompressFromMemory()
{
  ZipArchive archive = ZipFile::Open(zipFilename2);
  printf("\n");

  printf("Compressing from memory and saving as '%s'\n", fileIn4);
  char buff[] = "Sample text";
  imemstream memstrm(buff);

  ZipArchiveEntry* entry = archive->CreateEntry(fileIn4);
  if (entry != nullptr)
  {
    entry->SetCompressionStream(&memstrm,
      ZipArchiveEntry::CompressionLevel::BestCompression,
      ZipArchiveEntry::CompressionMethod::Deflate,
      ZipArchiveEntry::CompressionMode::Deferred
    );
  }

  // when deferred mode is chosen, the input stream ("f" here)
  // must be still alive
  printf("Saving into '%s'\n", zipFilename);
  std::ofstream z2(zipFilename, std::ios::binary);
  archive->WriteToStream(z2);
  z2.close();
}

void Sample_ZipArchive_ExtractManually()
{
  ZipArchive archive = ZipFile::Open(zipFilename);
  printf("\n");

  printf("Extracting file '%s'\n", fileIn3);

  std::istream* decompressionStream = nullptr;
  ZipArchiveEntry* entry = archive->GetEntry(fileIn3);
  if (entry != nullptr)
  {
    decompressionStream = entry->GetDecompressionStream();
  }

  if (decompressionStream == nullptr)
  {
    return;
  }

  printf("The first line of the content is:\n", fileIn3);
  std::string line;
  std::getline(*decompressionStream, line);
  std::cout << line << std::endl;
}

void Sample_ZipArchive_EncryptFileManually()
{
  ZipArchive archive = ZipFile::Open(zipFilename);
  printf("\n");

  printf("Encrypting file '%s'\n", fileIn3);
  std::ifstream f(fileIn3, std::ios::binary);

  archive->RemoveEntry(fileIn3);

  ZipArchiveEntry* entry = archive->CreateEntry(fileIn3);
  if (entry != nullptr)
  {
    entry->SetComment("Lorem ipsum file");
    entry->SetPassword("pass");
    entry->SetCompressionStream(&f,
      ZipArchiveEntry::CompressionLevel::Stored,
      ZipArchiveEntry::CompressionMethod::Stored,
      ZipArchiveEntry::CompressionMode::Deferred
    );
  }

  // save in2.png immediatelly
  printf("Encrypting file '%s'\n", fileIn2);
  std::ifstream f2(fileIn2, std::ios::binary);
  ZipArchiveEntry* entry2 = archive->CreateEntry(fileIn2Dest);
  if (entry2 != nullptr)
  {
    entry2->SetPassword("pass");
    entry2->SetCompressionStream(&f2,
      ZipArchiveEntry::CompressionLevel::BestCompression,
      ZipArchiveEntry::CompressionMethod::Deflate,
      ZipArchiveEntry::CompressionMode::Immediate
    );
  }

  // when deferred mode is chosen, the input stream ("f" here)
  // must be still alive
  printf("Saving into '%s'\n", zipFilename2);
  std::ofstream z2(zipFilename2, std::ios::binary);
  archive->SetComment("Archive comment");
  archive->WriteToStream(z2);
  z2.close();
}

void Sample_ZipArchive_DecryptFileManually()
{
  ZipArchive archive = ZipFile::Open(zipFilename2);
  printf("\n");

  printf("Decrypting file '%s'\n", fileIn3);

  std::istream* decompressionStream = nullptr;
  ZipArchiveEntry* entry = archive->GetEntry(fileIn3);
  if (entry != nullptr)
  {
    decompressionStream = entry->GetDecompressionStream();

    if (decompressionStream == nullptr)
    {
      printf("# try 1: no password\n");
    }

    entry->SetPassword("wrongpass");
    decompressionStream = entry->GetDecompressionStream();

    if (decompressionStream == nullptr)
    {
      printf("# try 2: wrong password\n");
    }

    entry->SetPassword("pass");
    decompressionStream = entry->GetDecompressionStream();

    if (decompressionStream != nullptr)
    {
      printf("# try 3: correct password\n");
    }
    else
    {
      printf("# try 3: wrong password\n");
    }
  }

  printf("The first line of the content is:\n", fileIn3);
  std::string line;
  std::getline(*decompressionStream, line);
  std::cout << line << std::endl;

  printf("The file comment is '%s'\n", entry->GetComment().c_str());
  printf("The archive comment is '%s'\n", archive->GetComment().c_str());
}

int main()
{
  // delete archive before this sample
  remove(zipFilename);
  remove(zipFilename2);

  Sample_ZipFile();
  Sample_ZipArchive_Listing();
  Sample_ZipArchive_StoreFileImmediate();
  Sample_ZipArchive_RemoveEntry();
  Sample_ZipArchive_CompressFileDeferred();
  Sample_ZipArchive_CompressFromMemory();
  Sample_ZipArchive_ExtractManually();

  Sample_ZipArchive_EncryptFileManually();
  Sample_ZipArchive_DecryptFileManually();

  return 0;
}
