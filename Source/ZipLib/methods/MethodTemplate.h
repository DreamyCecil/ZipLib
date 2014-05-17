#pragma once
#include "CompressionMethod.h"
#include "EncryptionMethod.h"

template <
  typename BASE_METHOD_CLASS,
  typename METHOD_CLASS,
  typename ENCODER_CLASS, typename DECODER_CLASS, typename PROPS_CLASS,
  uint16_t COMPRESSION_METHOD, uint16_t VERSION_NEEDED_TO_EXTRACT
>
class MethodTemplate
  : public BASE_METHOD_CLASS
{
  public:
    typedef std::shared_ptr<METHOD_CLASS>               Ptr;
    typedef typename BASE_METHOD_CLASS::encoder_type    encoder_type;
    typedef typename BASE_METHOD_CLASS::decoder_type    decoder_type;
    typedef typename BASE_METHOD_CLASS::properties_type properties_type;

    static const uint16_t CompressionMethod      = COMPRESSION_METHOD;
    static const uint16_t VersionNeededToExtract = VERSION_NEEDED_TO_EXTRACT;

    MethodTemplate()
    {
      this->SetEncoder(std::make_shared<ENCODER_CLASS>());
      this->SetDecoder(std::make_shared<DECODER_CLASS>());
    }

    static Ptr Create()
    {
      return std::make_shared<METHOD_CLASS>();
    }

    properties_type& GetProperties() override
    {
      _properties.normalize();
      return _properties;
    }

    size_t GetBufferCapacity() const
    {
      return _properties.BufferCapacity;
    }

    void SetBufferCapacity(size_t bufferCapacity)
    {
      _properties.BufferCapacity = bufferCapacity;
    }

    const ZipMethodDescriptor& GetZipMethodDescriptor() const override
    {
      return GetZipMethodDescriptorStatic();
    }

    static const ZipMethodDescriptor& GetZipMethodDescriptorStatic()
    {
      static ZipMethodDescriptor zmd {
        VersionNeededToExtract,
        CompressionMethod
      };
      return zmd;
    }

  protected:
    PROPS_CLASS _properties;
};

template <
  typename METHOD_CLASS,
  typename ENCODER_CLASS, typename DECODER_CLASS, typename PROPS_CLASS,
  uint16_t COMPRESSION_METHOD, uint16_t VERSION_NEEDED_TO_EXTRACT
>
using CompressionMethodTemplate = MethodTemplate
<
  CompressionMethod,
  METHOD_CLASS,
  ENCODER_CLASS, DECODER_CLASS, PROPS_CLASS,
  COMPRESSION_METHOD, VERSION_NEEDED_TO_EXTRACT
>;

template <
  typename METHOD_CLASS,
  typename ENCODER_CLASS, typename DECODER_CLASS, typename PROPS_CLASS,
  uint16_t COMPRESSION_METHOD, uint16_t VERSION_NEEDED_TO_EXTRACT
>
using EncryptionMethodTemplate = MethodTemplate
<
  EncryptionMethod,
  METHOD_CLASS,
  ENCODER_CLASS, DECODER_CLASS, PROPS_CLASS,
  COMPRESSION_METHOD, VERSION_NEEDED_TO_EXTRACT
>;
