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
    typedef std::shared_ptr<CompressionMethod>      Ptr;
    typedef std::shared_ptr<EncryptionMethod>       EncryptionMethodPtr;
    typedef std::shared_ptr<ZipArchiveEntry>        ZipArchiveEntryPtr;

    typedef CompressionMethodBase::encoder_type     encoder_type;
    typedef CompressionMethodBase::decoder_type     decoder_type;
    typedef CompressionMethodBase::properties_type  properties_type;

    EncryptionMethodPtr GetEncryptionMethod() const;
    void SetEncryptionMethod(EncryptionMethodPtr encryptionMethod);

    ZipMethodDescriptor GetFinalMethodDescriptor() const;

  private:
    EncryptionMethodPtr _encryptionMethod;
};
