#pragma once
#include "../compression_interface.h"

#include <algorithm>

struct bzip2_encoder_properties
  : compression_encoder_properties_interface
{
  void normalize() override
  {
    BlockSize = clamp(1, 9, BlockSize);
    WorkFactor = clamp(0, 4, WorkFactor);
  }

  int BlockSize   = 6;
  int WorkFactor  = 30;
};
