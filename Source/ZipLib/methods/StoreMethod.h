#pragma once
#include "ICompressionMethod.h"
#include "../compression/store/store_encoder.h"
#include "../compression/store/store_decoder.h"

#include <memory>

#ifndef ZIPLIB_NO_ZLIB

class StoreMethod :
  public ICompressionMethod
{
  public:
    ZIP_METHOD_CLASS_PROLOGUE(
      StoreMethod,
      store_encoder, store_decoder,
      _encoderProps, _decoderProps,
      /* CompressionMethod */ 0,
      /* VersionNeededToExtract */ 10
    );

    ZIP_METHOD_CLASS_CONSTRUCTOR(StoreMethod, store_encoder, store_decoder);

  private:
    store_encoder_properties _encoderProps;
    store_decoder_properties _decoderProps;
};

#else

// [Cecil] Replace with a dummy method in case zlib is disabled
#include "DummyMethod.h"

typedef DummyMethod StoreMethod;

#endif // ZIPLIB_NO_ZLIB
