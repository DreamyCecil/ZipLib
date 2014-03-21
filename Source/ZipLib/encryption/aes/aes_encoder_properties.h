#pragma once
#include "common.h"
#include "../encryption_interface.h"

struct aes_encoder_properties
  : encryption_encoder_properties_interface
{
  aes_encoder_properties()
    : BufferCapacity(1 << 15)
    , Version(aes_version::AE_1)
    , EncryptionMode(aes_encryption_mode::AES256)
  {

  }

  void normalize() override
  {

  }

  size_t      BufferCapacity;
  aes_version Version;
  aes_encryption_mode EncryptionMode;
};
