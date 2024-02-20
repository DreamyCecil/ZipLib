#pragma once
#include "../compression_interface.h"

#include <algorithm>

#ifndef ZIPLIB_NO_ZLIB

struct store_encoder_properties
  : compression_encoder_properties_interface
{
  store_encoder_properties()
    : BufferCapacity(1 << 15)
  {

  }

  void normalize() override
  {

  }

  size_t BufferCapacity;
};

#endif // ZIPLIB_NO_ZLIB
