#pragma once
#include "../encryption_interface.h"

#include <algorithm>

struct zipcrypto_encoder_properties
  : encryption_encoder_properties_interface
{
  uint8_t LastByteOfEncryptionHeader = uint8_t(-1);
  bool    UseDataDescriptor          = true;
};
