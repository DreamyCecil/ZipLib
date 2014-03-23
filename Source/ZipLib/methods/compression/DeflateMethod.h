#pragma once
#include "../MethodTemplate.h"
#include "../../compression/deflate/deflate_encoder.h"
#include "../../compression/deflate/deflate_decoder.h"

#include <memory>

class DeflateMethod :
  public CompressionMethodTemplate
  <
    DeflateMethod,
    deflate_encoder, deflate_decoder, deflate_properties,
    /* CompressionMethod */ 8,
    /* VersionNeededToExtract */ 20
  >
{
  public:
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

    CompressionLevel GetCompressionLevel() const { return static_cast<CompressionLevel>(_properties.CompressionLevel); }
    void SetCompressionLevel(CompressionLevel compressionLevel) { _properties.CompressionLevel = static_cast<int>(compressionLevel); }
};
