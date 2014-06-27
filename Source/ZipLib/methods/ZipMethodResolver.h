#pragma once
#include "MethodTemplate.h"

#include "compression/StoreMethod.h"
#include "compression/DeflateMethod.h"
#include "compression/Bzip2Method.h"
#include "compression/LzmaMethod.h"

#include "encryption/ZipCryptoMethod.h"
#include "encryption/AesCryptoMethod.h"

#include <cstdint>
#include <memory>

#define ZIP_METHOD_TABLE          \
  ZIP_METHOD_ADD(StoreMethod);    \
  ZIP_METHOD_ADD(DeflateMethod);  \
  ZIP_METHOD_ADD(Bzip2Method);    \
  ZIP_METHOD_ADD(LzmaMethod);

#define ZIP_ENCRYPTION_METHOD_TABLE           \
  ZIP_ENCRYPTION_METHOD_ADD(AesCryptoMethod); \
  ZIP_ENCRYPTION_METHOD_ADD(ZipCryptoMethod); 

#define ZIP_METHOD_ADD(method_class)                                                                                \
  if ((encryptionMethod                                                                                             \
    && encryptionMethod->GetAlternativeCompressionMethodNumber() != ZipMethodDescriptor::INVALID_COMPRESSION_METHOD \
    && encryptionMethod->GetAlternativeCompressionMethodNumber() == method_class::CompressionMethod)                \
    || archiveEntry->GetCompressionMethod() == method_class::CompressionMethod)                                     \
    {                                                                                                               \
      compressionMethod = method_class::Create();                                                                   \
      break;                                                                                                        \
    }

#define ZIP_ENCRYPTION_METHOD_ADD(method_class)                                                   \
  if (archiveEntry->IsEncrypted() &&                                                              \
      ((method_class::CompressionMethod != ZipMethodDescriptor::INVALID_COMPRESSION_METHOD &&    \
        method_class::CompressionMethod == archiveEntry->GetCompressionMethod())                  \
    || (method_class::CompressionMethod == ZipMethodDescriptor::INVALID_COMPRESSION_METHOD)))     \
    {                                                                                             \
      encryptionMethod = method_class::Create();                                                  \
      encryptionMethod->OnDecryptionBegin(archiveEntry);                                          \
      break;                                                                                      \
    }


class ZipMethodResolver
{
  public:
    static CompressionMethod::Ptr GetZipMethodInstance(ZipArchiveEntry::Ptr archiveEntry)
    {
      CompressionMethod::Ptr compressionMethod;
      EncryptionMethod::Ptr encryptionMethod;

      do { ZIP_ENCRYPTION_METHOD_TABLE; } while (0);
      do { ZIP_METHOD_TABLE;            } while (0);

      if (compressionMethod && encryptionMethod)
        compressionMethod->SetEncryptionMethod(encryptionMethod);

      return compressionMethod;
    }
};
