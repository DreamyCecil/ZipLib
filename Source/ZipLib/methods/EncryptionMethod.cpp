#include "CompressionMethod.h"
#include "EncryptionMethod.h"
#include <memory>

EncryptionMethod::CompressionMethodPtr EncryptionMethod::GetCompressionMethod() const
{
  return _compressionMethod.lock();
}

uint16_t EncryptionMethod::GetAlternativeCompressionMethodNumber() const
{
  return ZipMethodDescriptor::INVALID_COMPRESSION_METHOD;
}

void EncryptionMethod::OnEncryptionBegin(EncryptionMethod::ZipArchiveEntryPtr entry)
{

}

void EncryptionMethod::OnDecryptionBegin(EncryptionMethod::ZipArchiveEntryPtr entry)
{

}
