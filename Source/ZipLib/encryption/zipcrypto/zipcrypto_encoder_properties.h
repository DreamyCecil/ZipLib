#pragma once
#include "../encryption_interface.h"

#include <algorithm>

struct zipcrypto_encoder_properties
  : encryption_encoder_properties_interface
{
  zipcrypto_encoder_properties()
    : BufferCapacity(1 << 15)
    , LastByteOfEncryptionHeader(uint8_t(-1))
  {

  }

  void normalize() override
  {

  }

  uint8_t     LastByteOfEncryptionHeader;
  size_t      BufferCapacity;
};
