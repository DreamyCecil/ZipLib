#include "CompressionMethod.h"
#include "EncryptionMethod.h"

CompressionMethod::EncryptionMethodPtr CompressionMethod::GetEncryptionMethod() const
{
  return _encryptionMethod;
}

void CompressionMethod::SetEncryptionMethod(CompressionMethod::EncryptionMethodPtr encryptionMethod)
{
  _encryptionMethod = encryptionMethod;
  _encryptionMethod->_compressionMethod = this->shared_from_this();
}

ZipMethodDescriptor CompressionMethod::GetFinalMethodDescriptor() const
{
  if (this->GetEncryptionMethod() == nullptr)
  {
    return this->GetZipMethodDescriptor();
  }

  const auto& compressionDescriptor = this->GetZipMethodDescriptor();
  const auto& encryptionDescriptor = this->GetEncryptionMethod()->GetZipMethodDescriptor();

  uint16_t versionNeededToExtract = std::max(
    compressionDescriptor.VersionNeededToExtract,
    encryptionDescriptor.VersionNeededToExtract);

  uint16_t compressionMethod = encryptionDescriptor.HasCompressionMethod()
    ? encryptionDescriptor.CompressionMethod
    : compressionDescriptor.CompressionMethod;

  ZipMethodDescriptor result{
    versionNeededToExtract,
    compressionMethod
  };

  return result;
}
