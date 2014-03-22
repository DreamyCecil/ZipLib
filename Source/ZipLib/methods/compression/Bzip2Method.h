#pragma once
#include "../MethodTemplate.h"
#include "../../compression/bzip2/bzip2_encoder.h"
#include "../../compression/bzip2/bzip2_decoder.h"

#include <memory>

class Bzip2Method :
  public CompressionMethodTemplate
  <
      Bzip2Method,
      bzip2_encoder, bzip2_decoder,
      bzip2_encoder_properties, bzip2_decoder_properties,
      /* CompressionMethod */ 12,
      /* VersionNeededToExtract */ 46
  >
{
  public:
    enum class BlockSize : int
    {
      B100 = 1,
      B200 = 2,
      B300 = 3,
      B400 = 4,
      B500 = 5,
      B600 = 6,
      B700 = 7,
      B800 = 8,
      B900 = 9,

      Fastest = B100,
      Default = B600,
      Best = B900
    };

    BlockSize GetBlockSize() const { return static_cast<BlockSize>(_encoderProps.BlockSize); }
    void SetBlockSize(BlockSize compressionLevel) { _encoderProps.BlockSize = static_cast<int>(compressionLevel); }
};
