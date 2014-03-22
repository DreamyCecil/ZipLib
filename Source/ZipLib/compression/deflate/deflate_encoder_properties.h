#pragma once
#include "../compression_interface.h"

#include <algorithm>

struct deflate_encoder_properties
  : compression_encoder_properties_interface
{
  void normalize() override
  {
    CompressionLevel = clamp(0, 9, CompressionLevel);
  }

  int CompressionLevel = 6;
};
