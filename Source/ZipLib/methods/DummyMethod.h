#pragma once

#include "ICompressionMethod.h"

// [Cecil] Unusable dummy compression method to substitute for disabled modules
class DummyMethod : public ICompressionMethod
{
  private:
    // Dummy encoder
    struct DummyEncoder : compression_encoder_properties_interface {
      void normalize() override {};
    };

    // Dummy decoder
    struct DummyDecoder : compression_decoder_properties_interface {
      void normalize() override {};
    };

    DummyEncoder _encoderProps;
    DummyDecoder _decoderProps;

  public:
    // Dummy constructor
    DummyMethod()
    {
    };

    // Don't create any method (causes access violation)
    static Ptr Create() {
      return nullptr;
    };

    // Define other dummy methods
    ZIP_METHOD_CLASS_PROLOGUE(DummyMethod, store_encoder, store_decoder,
      _encoderProps, _decoderProps, 0, 0);
};
