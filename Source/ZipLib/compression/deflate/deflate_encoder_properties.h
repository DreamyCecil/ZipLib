#pragma once
#include "../compression_interface.h"

#include <algorithm>

struct deflate_encoder_properties
  : compression_encoder_properties_interface
{
  deflate_encoder_properties()
    : BufferCapacity(1 << 15)
    , CompressionLevel(6)
  {

  }

  void normalize() override
  {
    CompressionLevel = std::min(std::max(0, CompressionLevel), 9);
  }

  size_t BufferCapacity;
  int    CompressionLevel;
};
