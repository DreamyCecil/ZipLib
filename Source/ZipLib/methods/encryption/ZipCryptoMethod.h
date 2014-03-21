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
    zipcrypto_encoder, zipcrypto_decoder,
    zipcrypto_encoder_properties, zipcrypto_decoder_properties,
    /* CompressionMethod */ ZipMethodDescriptor::INVALID_COMPRESSION_METHOD,
    /* VersionNeededToExtract */ 20
  >
{
  public:
    size_t GetBufferCapacity() const { return _encoderProps.BufferCapacity; }
    void SetBufferCapacity(size_t bufferCapacity) { _encoderProps.BufferCapacity = _decoderProps.BufferCapacity = bufferCapacity; }

    const std::string& GetPassword() const override { return _encoderProps.Password; }
    void SetPassword(const std::string& password) override { _encoderProps.Password = _decoderProps.Password = password; }

  protected:
    void OnEncryptionBegin(ZipArchiveEntryPtr entry) override
    {
      _encoderProps.LastByteOfEncryptionHeader = get_last_byte_of_encryption_header(entry);
    }

    void OnDecryptionBegin(ZipArchiveEntryPtr entry) override
    {
      _decoderProps.LastByteOfEncryptionHeader = get_last_byte_of_encryption_header(entry);
    }
};
