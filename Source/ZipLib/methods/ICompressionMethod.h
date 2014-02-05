#pragma once
#include "../compression/compression_interface.h"

#include <memory>

#define ZIP_METHOD_CLASS_PROLOGUE(method_class, encoder_class, decoder_class, compression_method, version_needed_to_extract)\
  typedef std::shared_ptr<method_class> Ptr;                                                                                \
                                                                                                                            \
  static const uint16_t CompressionMethod = compression_method;                                                             \
  static const uint16_t VersionNeededToExtract = version_needed_to_extract;                                                 \
                                                                                                                            \
  method_class()                                                                                                            \
  {                                                                                                                         \
    this->SetEncoder(std::make_shared<encoder_class>());                                                                    \
    this->SetDecoder(std::make_shared<decoder_class>());                                                                    \
  }                                                                                                                         \
                                                                                                                            \
  static Ptr Create()                                                                                                       \
  {                                                                                                                         \
    return std::make_shared<method_class>();                                                                                \
  }                                                                                                                         \
                                                                                                                            \
  const ZipMethodDescriptor* GetZipMethodDescriptor() const override                                                        \
  {                                                                                                                         \
    return GetZipMethodDescriptorStatic();                                                                                  \
  }                                                                                                                         \
                                                                                                                            \
  static const ZipMethodDescriptor* GetZipMethodDescriptorStatic()                                                          \
  {                                                                                                                         \
    static ZipMethodDescriptor zmd = {                                                                                      \
      CompressionMethod,                                                                                                    \
      VersionNeededToExtract                                                                                                \
    };                                                                                                                      \
    return &zmd;                                                                                                            \
  }

struct ZipMethodDescriptor
{
  uint16_t CompressionMethod;
  uint16_t VersionNeededToExtract;
};

class ICompressionMethod
{
  public:
    typedef std::shared_ptr<ICompressionMethod> Ptr;

    typedef std::shared_ptr<compression_encoder_interface> encoder_t;
    typedef std::shared_ptr<compression_decoder_interface> decoder_t;

    encoder_t GetEncoder() const { return _encoder; }
    decoder_t GetDecoder() const { return _decoder; }

    virtual const ZipMethodDescriptor* GetZipMethodDescriptor() const = 0;
    static const ZipMethodDescriptor* GetZipMethodDescriptorStatic() { return nullptr; }

  protected:
    void SetEncoder(encoder_t encoder) { _encoder = encoder; }
    void SetDecoder(decoder_t decoder) { _decoder = decoder; }

  private:
    encoder_t _encoder;
    decoder_t _decoder;
};
