#pragma once
#include "ZipMethod.h"
#include <string>

class ZipArchiveEntry;
class CompressionMethod;

using EncryptionMethodBase = ZipMethod<
  encryption_encoder_interface, encryption_decoder_interface,
  encryption_encoder_properties_interface, encryption_decoder_properties_interface
>;

class EncryptionMethod
  : public EncryptionMethodBase
  , public std::enable_shared_from_this<EncryptionMethod>
{
  friend class CompressionMethod;
  friend class ZipArchiveEntry;
  friend class ZipMethodResolver;

  public:
    typedef std::shared_ptr<EncryptionMethod>  Ptr;
    typedef std::shared_ptr<CompressionMethod> CompressionMethodPtr;
    typedef std::shared_ptr<ZipArchiveEntry>   ZipArchiveEntryPtr;

    typedef EncryptionMethodBase::encoder_t             encoder_t;
    typedef EncryptionMethodBase::decoder_t             decoder_t;
    typedef EncryptionMethodBase::encoder_properties_t  encoder_properties_t;
    typedef EncryptionMethodBase::decoder_properties_t  decoder_properties_t;
    
    CompressionMethodPtr GetCompressionMethod() const;

    size_t GetBufferCapacity() const { return _encoderProps.BufferCapacity; }
    void SetBufferCapacity(size_t bufferCapacity) { _encoderProps.BufferCapacity = _decoderProps.BufferCapacity = bufferCapacity; }

    virtual void SetPassword(const std::string& password) = 0;
    virtual const std::string& GetPassword() const = 0;

  protected:
    // Overloadable callbacks.
    virtual void OnEncryptionBegin(ZipArchiveEntryPtr entry);
    virtual void OnDecryptionBegin(ZipArchiveEntryPtr entry);

    // If some encryption method overrides CompressionMethod number (like AES),
    // this method returns underlying compression method
    // (or ZipMethodDescriptor::INVALID_COMPRESSION_METHOD otherwise).
    virtual uint16_t GetAlternativeCompressionMethodNumber() const;

  private:
    typedef std::weak_ptr<CompressionMethod> CompressionMethodWeakPtr;
    typedef std::weak_ptr<ZipArchiveEntry>   ZipArchiveEntryWeakPtr;

    CompressionMethodWeakPtr _compressionMethod;
};
