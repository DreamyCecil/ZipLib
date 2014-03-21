#pragma once
#include "CompressionMethod.h"
#include "EncryptionMethod.h"

template <
  typename BASE_METHOD_CLASS,
  typename METHOD_CLASS,
  typename ENCODER_CLASS, typename DECODER_CLASS,
  typename ENCODER_PROPS_CLASS, typename DECODER_PROPS_CLASS,
  uint16_t COMPRESSION_METHOD, uint16_t VERSION_NEEDED_TO_EXTRACT
>
class MethodTemplate
  : public BASE_METHOD_CLASS
{    
  public:
    typedef std::shared_ptr<METHOD_CLASS> Ptr;
    typedef typename BASE_METHOD_CLASS::encoder_t encoder_t;
    typedef typename BASE_METHOD_CLASS::decoder_t decoder_t;
    typedef typename BASE_METHOD_CLASS::encoder_properties_t encoder_properties_t;
    typedef typename BASE_METHOD_CLASS::decoder_properties_t decoder_properties_t;
                                                                                
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
                                                                                
    encoder_properties_t& GetEncoderProperties() override
    {                                                                             
      _encoderProps.normalize();
      return _encoderProps;
    }                                                                             
                                                                                
    decoder_properties_t& GetDecoderProperties() override
    {                                                                             
      _decoderProps.normalize();
      return _decoderProps;
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
    ENCODER_PROPS_CLASS _encoderProps;
    DECODER_PROPS_CLASS _decoderProps;
};

template <
  typename METHOD_CLASS,
  typename ENCODER_CLASS, typename DECODER_CLASS,
  typename ENCODER_PROPS_CLASS, typename DECODER_PROPS_CLASS,
  uint16_t COMPRESSION_METHOD, uint16_t VERSION_NEEDED_TO_EXTRACT
>
using CompressionMethodTemplate = MethodTemplate
<
  CompressionMethod,
  METHOD_CLASS,
  ENCODER_CLASS, DECODER_CLASS,
  ENCODER_PROPS_CLASS, DECODER_PROPS_CLASS,
  COMPRESSION_METHOD, VERSION_NEEDED_TO_EXTRACT
>;

template <
  typename METHOD_CLASS,
  typename ENCODER_CLASS, typename DECODER_CLASS,
  typename ENCODER_PROPS_CLASS, typename DECODER_PROPS_CLASS,
  uint16_t COMPRESSION_METHOD, uint16_t VERSION_NEEDED_TO_EXTRACT
>
using EncryptionMethodTemplate = MethodTemplate
<
  EncryptionMethod,
  METHOD_CLASS,
  ENCODER_CLASS, DECODER_CLASS,
  ENCODER_PROPS_CLASS, DECODER_PROPS_CLASS,
  COMPRESSION_METHOD, VERSION_NEEDED_TO_EXTRACT
>;
