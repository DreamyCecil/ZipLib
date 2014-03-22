#pragma once
#include "../MethodTemplate.h"
#include "detail/AesExtraField.h"
#include "../../encryption/aes/aes_encoder.h"
#include "../../encryption/aes/aes_decoder.h"
#include "../../streams/memstream.h"

#include "../../ZipArchiveEntry.h"
#include "../../detail/ZipArchiveEntryInfo.h"

#include <memory>

class AesCryptoMethod :
  public EncryptionMethodTemplate
  <
    AesCryptoMethod,
    aes_encoder, aes_decoder,
    aes_encoder_properties, aes_decoder_properties,
    /* CompressionMethod */ 99,
    /* VersionNeededToExtract */ 51
  >
{
  public:
    typedef aes_encryption_mode EncryptionMode;
    typedef aes_version Version;

    const std::string& GetPassword() const override { return _encoderProps.Password; }
    void SetPassword(const std::string& password) override { _encoderProps.Password = _decoderProps.Password = password; }

    EncryptionMode GetEncryptionMode() const { return _decoderProps.EncryptionMode; }
    void SetEncryptionMode(EncryptionMode encryptionMode) { _encoderProps.EncryptionMode = _decoderProps.EncryptionMode = encryptionMode; }

    Version GetVersion() const { return _decoderProps.Version; }
    void SetVersion(Version version) { _encoderProps.Version = _decoderProps.Version = version; }

  protected:
    void OnEncryptionBegin(ZipArchiveEntryPtr entry) override
    {
      detail::AesExtraField aef;
      detail::ZipGenericExtraField ef;
      
      aef.CompressionMethod = this->GetCompressionMethod()->GetZipMethodDescriptor().CompressionMethod;
      aef.AesVersion = static_cast<uint16_t>(this->GetVersion());
      aef.EncryptionMode = static_cast<uint8_t>(this->GetEncryptionMode());

      aef.ToExtraField(ef);

      entry->GetInternalInfo().CentralDirectoryFileHeader.ExtraFields.push_back(ef);
      entry->GetInternalInfo().LocalFileHeader.ExtraFields.push_back(ef);
    }
    
    void OnDecryptionBegin(ZipArchiveEntryPtr entry) override
    {
      for (auto& ef : entry->GetInternalInfo().CentralDirectoryFileHeader.ExtraFields)
      {
        if (ef.Tag == detail::AesExtraField::AES_TAG)
        {
          detail::AesExtraField aef;
          aef.FromExtraField(ef);

          _alternativeCompressionMethod = aef.CompressionMethod;
          this->SetVersion(static_cast<Version>(aef.AesVersion));
          this->SetEncryptionMode(static_cast<EncryptionMode>(aef.EncryptionMode));
          break;
        }
      }
    }

    uint16_t GetAlternativeCompressionMethodNumber() const override
    {
      return _alternativeCompressionMethod;
    }

  private:
    uint16_t _alternativeCompressionMethod;
};
