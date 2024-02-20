#pragma once
#include "../compression_interface.h"

#ifndef ZIPLIB_NO_ZLIB

struct deflate_decoder_properties
  : compression_decoder_properties_interface
{
  deflate_decoder_properties()
    : BufferCapacity(1 << 15)
  {

  }

  void normalize() override
  {

  }

  size_t BufferCapacity;
};

#endif // ZIPLIB_NO_ZLIB
