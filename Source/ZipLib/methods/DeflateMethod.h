#pragma once
#include "ICompressionMethod.h"
#include "../compression/deflate/deflate_encoder.h"
#include "../compression/deflate/deflate_decoder.h"

#include <memory>

#ifndef ZIPLIB_NO_ZLIB

class DeflateMethod :
  public ICompressionMethod
{
  public:
    ZIP_METHOD_CLASS_PROLOGUE(
      DeflateMethod,
      deflate_encoder, deflate_decoder,
      _encoderProps, _decoderProps,
      /* CompressionMethod */ 8,
      /* VersionNeededToExtract */ 20
    );

    ZIP_METHOD_CLASS_CONSTRUCTOR(DeflateMethod, deflate_encoder, deflate_decoder);

    enum class CompressionLevel : int
    {
      L1 = 1,
      L2 = 2,
      L3 = 3,
      L4 = 4,
      L5 = 5,
      L6 = 6,
      L7 = 7,
      L8 = 8,
      L9 = 9,

      Fastest = L1,
      Default = L6,
      Best = L9
    };

    size_t GetBufferCapacity() const { return _encoderProps.BufferCapacity; }
    void SetBufferCapacity(size_t bufferCapacity) { _encoderProps.BufferCapacity = bufferCapacity; }

    CompressionLevel GetCompressionLevel() const { return static_cast<CompressionLevel>(_encoderProps.CompressionLevel); }
    void SetCompressionLevel(CompressionLevel compressionLevel) { _encoderProps.CompressionLevel = static_cast<int>(compressionLevel); }

  private:
    deflate_encoder_properties _encoderProps;
    deflate_decoder_properties _decoderProps;
};

#else

// [Cecil] Replace with a dummy method in case zlib is disabled
#include "DummyMethod.h"

typedef DummyMethod DeflateMethod;

#endif // ZIPLIB_NO_ZLIB
