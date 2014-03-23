#pragma once
#include "../compression_interface.h"

#include <algorithm>

struct deflate_properties
  : compression_properties_interface
{
  void normalize() override
  {
    CompressionLevel = clamp(0, 9, CompressionLevel);
  }

  int CompressionLevel = 6;
};
