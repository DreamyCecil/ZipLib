#include "stdafx.h"
#include "Config.h"

namespace ZipLibTest
{
  
  ZIPLIB_TEST_GROUP_BEGIN(ZipLibCore)
    
    ZIPLIB_TEST_CASE_BEGIN(ZipLibWrites_7zipReads)
    {
      static const size_t FILE_SIZES[] = {
        16 * 1000,      // Below BufferCapacity
        32 * 1024,      // Exact BufferCapacity
        64 * 1000,      // Above BufferCapacity
  //      1000 * 1000,    // 1 MB
  //      10000 * 1000,   // 10 MB
  //      50000 * 1000    // 50 MB
      };

      static const char ARCHIVE_NAME[] = "archive.zip";
      static const char FILE_NAME[] = "file.dat";

      for (ZipCompressionMethod compressionMethod = ZipCompressionMethod::Store; compressionMethod <= ZipCompressionMethod::Lzma; compressionMethod = (ZipCompressionMethod)((int)compressionMethod + 1))
      {
        for (ZipEncryptionMethod encryptionMethod = ZipEncryptionMethod::None; encryptionMethod <= ZipEncryptionMethod::Aes256; encryptionMethod = (ZipEncryptionMethod)((int)encryptionMethod + 1))
        {
          for (size_t fileSize : FILE_SIZES)
          {
            for (ZipArchiveEntry::CompressionMode compressionMode = ZipArchiveEntry::CompressionMode::Immediate; compressionMode <= ZipArchiveEntry::CompressionMode::Deferred; compressionMode = (ZipArchiveEntry::CompressionMode)((int) compressionMode + 1))
            {
              std::cout
                << "Test:" << std::endl
                << "  - Compression method: " << compressionMethod << std::endl
                << "  - Encryption method:  " << encryptionMethod << std::endl
                << "  - Compression mode:   " << compressionMode << std::endl
                << "  - File size:          " << fileSize << std::endl;

              // Init.
              std::cout
                << "    - Init..." << std::endl;

              EmptyTestDirectory();

              GenerateFile(FILE_NAME, fileSize);

              // Test.
              auto method = FromZipCompressionMetod(compressionMethod);
              method->SetEncryptionMethod(FromZipEncryptionMethod(encryptionMethod));

              {
                auto archive = ZipArchive::Create();
                std::ifstream fileToAdd;
                fileToAdd.open(FILE_NAME, std::ios::binary);

                auto fileEntry = archive->CreateEntry(FILE_NAME);
                fileEntry->SetCompressionStream(fileToAdd, method, compressionMode);

                std::ofstream outFile;
                outFile.open(ARCHIVE_NAME, std::ios::binary);

                archive->WriteToStream(outFile);
                outFile.close();
              }

              // Evaluation.
              SevenZipTool szt;
              szt.Open(ARCHIVE_NAME);

              const ZipEntryInfo& originalEntry = szt.GetEntries()[0];

              std::cout
                << "    - Test..." << std::endl;

              // Test.
              {
                ZipArchive::Ptr archive = ZipFile::Open(ARCHIVE_NAME);
                ZIPLIB_TEST_IS_TRUE(!!archive);
                ZIPLIB_TEST_IS_TRUE(archive->GetEntriesCount() == 1);

                ZipArchiveEntry::Ptr entry = archive->GetEntry(FILE_NAME);
                ZIPLIB_TEST_IS_TRUE(!!entry);

                ZIPLIB_TEST_IS_TRUE(entry == archive->GetEntry(0));

                CompressionMethod::Ptr zipMethod = ZipMethodResolver::GetZipMethodInstance(entry);
                ZIPLIB_TEST_IS_TRUE(!!zipMethod);

                ZIPLIB_TEST_IS_TRUE(entry->GetFullName() == FILE_NAME);
                ZIPLIB_TEST_IS_TRUE(entry->GetName() == FILE_NAME);
                ZIPLIB_TEST_IS_TRUE(entry->GetComment() == "");
                ZIPLIB_TEST_IS_TRUE(entry->GetAttributes() == ZipArchiveEntry::Attributes::Archive);
                ZIPLIB_TEST_IS_TRUE(entry->GetCompressionMethod() == zipMethod->GetFinalMethodDescriptor().CompressionMethod);
                ZIPLIB_TEST_IS_TRUE(entry->IsEncrypted() == (encryptionMethod != ZipEncryptionMethod::None));
                ZIPLIB_TEST_IS_TRUE(entry->GetCrc32() == originalEntry.Crc32);
                ZIPLIB_TEST_IS_TRUE(entry->GetSize() == originalEntry.UncompressedSize);
                ZIPLIB_TEST_IS_TRUE(entry->GetCompressedSize() == originalEntry.CompressedSize);
                ZIPLIB_TEST_IS_TRUE(entry->CanExtract());
                ZIPLIB_TEST_IS_TRUE((entry->IsUsingDataDescriptor() == (encryptionMethod == ZipEncryptionMethod::ZipCrypto)));
                ZIPLIB_TEST_IS_FALSE(entry->IsDirectory());
                ZIPLIB_TEST_IS_FALSE(entry->IsDecompressionStreamOpened());
                ZIPLIB_TEST_IS_FALSE(entry->IsRawStreamOpened());

                // Check if file is same.
                {
                  std::cout
                    << "      - File is same..." << std::endl;

                  std::istream* decompressionStream = entry->GetDecompressionStream(entry->IsEncrypted() ? "abcd" : "");
                  std::ifstream fileStream(FILE_NAME, std::ios::binary);

                  ZIPLIB_TEST_IS_TRUE(!!decompressionStream);
                  ZIPLIB_TEST_IS_TRUE(fileStream.is_open());
                  ZIPLIB_TEST_IS_TRUE(StreamContentEquals(*decompressionStream, fileStream));

                  entry->CloseDecompressionStream();

                  if (!IsAesEncryption(encryptionMethod))
                  {
                    std::cout
                      << "      - Verifying CRC32..." << std::endl;

                    decompressionStream = entry->GetDecompressionStream(entry->IsEncrypted() ? "abcd" : "");
                    crc32stream crcStream(*decompressionStream);
                    nullstream nulldev;
                    utils::stream::copy(crcStream, nulldev);

                    ZIPLIB_TEST_IS_TRUE(entry->GetCrc32() == crcStream.get_crc32());
                  }
                }

                // Extract using 7zip
                {
                  std::cout
                    << "      - Renaming file..." << std::endl;

                  rename(FILE_NAME, "file2.dat");

                  std::cout
                    << "      - Extracting using 7zip..." << std::endl;

                  ZIPLIB_TEST_IS_TRUE(szt.ExtractFile(FILE_NAME, encryptionMethod == ZipEncryptionMethod::None ? "" : "abcd"));

                  // Check if file is same.
                  std::cout
                    << "      - File is same..." << std::endl;

                  std::ifstream fileStream1("file2.dat", std::ios::binary);
                  std::ifstream fileStream2(FILE_NAME, std::ios::binary);

                  ZIPLIB_TEST_IS_TRUE(fileStream1.is_open());
                  ZIPLIB_TEST_IS_TRUE(fileStream2.is_open());
                  ZIPLIB_TEST_IS_TRUE(StreamContentEquals(fileStream1, fileStream2));
                }
              }

              // Clean.
              std::cout
                << "    - Cleanup..." << std::endl << std::endl;
              EmptyTestDirectory();
            }
          }
        }
      }
    }
    ZIPLIB_TEST_CASE_END()

    ZIPLIB_TEST_CASE_BEGIN(SevenZipWrites_ZipLibReads)
    {
      static const size_t FILE_SIZES[] = {
        16 * 1000,      // Below BufferCapacity
        32 * 1024,      // Exact BufferCapacity
        64 * 1000,      // Above BufferCapacity
  //      1000 * 1000,    // 1 MB
  //      10000 * 1000,   // 10 MB
  //      50000 * 1000    // 50 MB
      };

      static const char ARCHIVE_NAME[] = "archive.zip";
      static const char FILE_NAME[] = "file.dat";

      for (ZipCompressionMethod compressionMethod = ZipCompressionMethod::Store; compressionMethod != ZipCompressionMethod::Lzma; compressionMethod = (ZipCompressionMethod)((int)compressionMethod + 1))
      {
        for (ZipEncryptionMethod encryptionMethod = ZipEncryptionMethod::None; encryptionMethod != ZipEncryptionMethod::Aes256; encryptionMethod = (ZipEncryptionMethod)((int)encryptionMethod + 1))
        {
          for (size_t fileSize : FILE_SIZES)
          {
            std::cout
              << "Test:" << std::endl
              << "  - Compression method: " << compressionMethod << std::endl
              << "  - Encryption method:  " << encryptionMethod << std::endl
              << "  - File size:          " << fileSize << std::endl;

            // Init.
            std::cout
              << "    - Init..." << std::endl;
            EmptyTestDirectory();

            GenerateFile(FILE_NAME, fileSize);

            SevenZipTool szt;
            szt.Open(ARCHIVE_NAME);
            szt.AddFile(FILE_NAME, compressionMethod, "abcd", encryptionMethod);

            const ZipEntryInfo& originalEntry = szt.GetEntries()[0];

            std::cout
              << "    - Test..." << std::endl;
            // Test.
            {
              ZipArchive::Ptr archive = ZipFile::Open(ARCHIVE_NAME);
              ZIPLIB_TEST_IS_TRUE(!!archive);
              ZIPLIB_TEST_IS_TRUE(archive->GetEntriesCount() == 1);

              ZipArchiveEntry::Ptr entry = archive->GetEntry(FILE_NAME);
              ZIPLIB_TEST_IS_TRUE(!!entry);

              ZIPLIB_TEST_IS_TRUE(entry == archive->GetEntry(0));

              CompressionMethod::Ptr zipMethod = ZipMethodResolver::GetZipMethodInstance(entry);
              ZIPLIB_TEST_IS_TRUE(!!zipMethod);

              ZIPLIB_TEST_IS_TRUE(entry->GetFullName() == FILE_NAME);
              ZIPLIB_TEST_IS_TRUE(entry->GetName() == FILE_NAME);
              ZIPLIB_TEST_IS_TRUE(entry->GetComment() == "");
              ZIPLIB_TEST_IS_TRUE(entry->GetAttributes() == ZipArchiveEntry::Attributes::Archive);
              ZIPLIB_TEST_IS_TRUE(entry->GetCompressionMethod() == zipMethod->GetFinalMethodDescriptor().CompressionMethod);
              ZIPLIB_TEST_IS_TRUE(entry->IsEncrypted() == (encryptionMethod != ZipEncryptionMethod::None));
              ZIPLIB_TEST_IS_TRUE(entry->GetCrc32() == originalEntry.Crc32);
              ZIPLIB_TEST_IS_TRUE(entry->GetSize() == originalEntry.UncompressedSize);
              ZIPLIB_TEST_IS_TRUE(entry->GetCompressedSize() == originalEntry.CompressedSize);
              ZIPLIB_TEST_IS_TRUE(entry->CanExtract());
              ZIPLIB_TEST_IS_FALSE(entry->IsUsingDataDescriptor());
              ZIPLIB_TEST_IS_FALSE(entry->IsDirectory());
              ZIPLIB_TEST_IS_FALSE(entry->IsDecompressionStreamOpened());
              ZIPLIB_TEST_IS_FALSE(entry->IsRawStreamOpened());

              // Check if file is same.
              std::cout
                << "      - File is same..." << std::endl;

              std::istream* decompressionStream = entry->GetDecompressionStream(entry->IsEncrypted() ? "abcd" : "");
              std::ifstream fileStream(FILE_NAME, std::ios::binary);

              ZIPLIB_TEST_IS_TRUE(!!decompressionStream);
              ZIPLIB_TEST_IS_TRUE(StreamContentEquals(*decompressionStream, fileStream));

              entry->CloseDecompressionStream();

              if (!IsAesEncryption(encryptionMethod))
              {
                std::cout
                  << "      - Verifying CRC32..." << std::endl;

                decompressionStream = entry->GetDecompressionStream(entry->IsEncrypted() ? "abcd" : "");
                crc32stream crcStream(*decompressionStream);
                nullstream nulldev;
                utils::stream::copy(crcStream, nulldev);

                ZIPLIB_TEST_IS_TRUE(entry->GetCrc32() == crcStream.get_crc32());
              }
            }

            // Clean.
            std::cout
              << "    - Cleanup..." << std::endl << std::endl;
            EmptyTestDirectory();
          }
        }
      }
    }
    ZIPLIB_TEST_CASE_END()

  ZIPLIB_TEST_GROUP_END()
}
