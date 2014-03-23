#pragma once
#include "../MethodTemplate.h"
#include "../../encryption/zipcrypto/zipcrypto_encoder.h"
#include "../../encryption/zipcrypto/zipcrypto_decoder.h"

#include <memory>

namespace
{
  static uint8_t get_last_byte_of_encryption_header(ZipArchiveEntry::Ptr entry)
  {
    return entry->GetLastByteOfEncryptionHeader();
  }
}

class ZipCryptoMethod :
  public EncryptionMethodTemplate
  <
    ZipCryptoMethod,
    zipcrypto_encoder, zipcrypto_decoder, zipcrypto_properties,
    /* CompressionMethod */ ZipMethodDescriptor::INVALID_COMPRESSION_METHOD,
    /* VersionNeededToExtract */ 20
  >
{
  public:
    const std::string& GetPassword() const override { return _properties.Password; }
    void SetPassword(const std::string& password) override { _properties.Password = password; }

    void SetUseDataDescriptor(bool use) { _properties.UseDataDescriptor = use; }
    bool GetUseDataDescriptor() const { return _properties.UseDataDescriptor; }

  protected:
    void OnEncryptionBegin(ZipArchiveEntryPtr entry) override
    {
      entry->UseDataDescriptor(_properties.UseDataDescriptor);
      _properties.LastByteOfEncryptionHeader = get_last_byte_of_encryption_header(entry);
    }

    void OnDecryptionBegin(ZipArchiveEntryPtr entry) override
    {
      _properties.LastByteOfEncryptionHeader = get_last_byte_of_encryption_header(entry);
      _properties.UseDataDescriptor = entry->IsUsingDataDescriptor();
    }
};
