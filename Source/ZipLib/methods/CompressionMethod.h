#pragma once
#include "ZipMethod.h"

class ZipArchiveEntry;
class EncryptionMethod;

using CompressionMethodBase = ZipMethod<
  compression_encoder_interface, compression_decoder_interface, compression_properties_interface
>;

class CompressionMethod
  : public CompressionMethodBase
  , public std::enable_shared_from_this<CompressionMethod>
{
  public:
    typedef std::shared_ptr<CompressionMethod>  Ptr;
    typedef std::shared_ptr<EncryptionMethod>   EncryptionMethodPtr;
    typedef std::shared_ptr<ZipArchiveEntry>    ZipArchiveEntryPtr;

    typedef CompressionMethodBase::encoder_t    encoder_t;
    typedef CompressionMethodBase::decoder_t    decoder_t;
    typedef CompressionMethodBase::properties_t properties_t;

    EncryptionMethodPtr GetEncryptionMethod() const;
    void SetEncryptionMethod(EncryptionMethodPtr encryptionMethod);

    ZipMethodDescriptor GetFinalMethodDescriptor() const;

  private:
    EncryptionMethodPtr _encryptionMethod;
};
