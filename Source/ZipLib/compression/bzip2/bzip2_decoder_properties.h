#pragma once
#include "../compression_interface.h"

#ifdef ZIPLIB_BZIP2

struct bzip2_decoder_properties
  : compression_decoder_properties_interface
{
  bzip2_decoder_properties()
    : BufferCapacity(1 << 15)
  {

  }

  void normalize() override
  {

  }

  size_t BufferCapacity;
};

#endif // ZIPLIB_BZIP2
