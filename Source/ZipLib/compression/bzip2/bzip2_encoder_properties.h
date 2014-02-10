#pragma once
#include "../compression_interface.h"

#include <algorithm>

struct bzip2_encoder_properties
  : compression_encoder_properties_interface
{
  bzip2_encoder_properties()
    : BufferCapacity(1 << 15)
    , BlockSize(6)
    , Verbosity(0)
    , WorkFactor(30)
  {

  }

  void normalize() override
  {
#ifdef min
#undef min
#undef max
#endif
    BlockSize = std::min(std::max(1, BlockSize), 9);
    Verbosity = std::min(std::max(0, Verbosity), 250);
    WorkFactor = std::min(std::max(0, WorkFactor), 4);
  }

  size_t  BufferCapacity;
  int     BlockSize;
  int     Verbosity;
  int     WorkFactor;
};
