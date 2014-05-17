#pragma once
#include "../compression/compression_interface.h"
#include "../encryption/encryption_interface.h"
#include "ZipMethodDescriptor.h"

#include <memory>

class ZipMethodBase
{
  public:
    static const uint16_t StoredCompressionMethod = 0;
    static const uint16_t StoredVersionNeededToExtract = 10;

    virtual const ZipMethodDescriptor& GetZipMethodDescriptor() const = 0;
    static const ZipMethodDescriptor& GetZipMethodDescriptorStatic()
    {
      // Default "Stored method" descriptor.
      static ZipMethodDescriptor zmd {
        StoredCompressionMethod,
        StoredVersionNeededToExtract
      };
      return zmd;
    }

  protected:
    ZipMethodBase() { }

  private:
    ZipMethodBase(const ZipMethodBase&) = delete;
    ZipMethodBase& operator = (const ZipMethodBase&) = delete;
};

template <
  typename ENCODER_TYPE, typename DECODER_TYPE, typename PROPS_TYPE
>
class ZipMethod
  : public ZipMethodBase
{
  public:
    typedef std::shared_ptr<ENCODER_TYPE> encoder_type;
    typedef std::shared_ptr<DECODER_TYPE> decoder_type;
    typedef PROPS_TYPE                    properties_type;

    void SetEncoder(encoder_type encoder) { _encoder = encoder; }
    void SetDecoder(decoder_type decoder) { _decoder = decoder; }

    encoder_type GetEncoder() const { return _encoder; }
    decoder_type GetDecoder() const { return _decoder; }

    virtual properties_type& GetProperties() = 0;

  protected:
    ZipMethod() { }

  private:
    encoder_type _encoder;
    decoder_type _decoder;
};
