#pragma once
#include "common.h"
#include "../encryption_interface.h"

struct aes_decoder_properties
  : encryption_decoder_properties_interface
{
  aes_decoder_properties()
    : BufferCapacity(1 << 15)
    , Version(aes_version::AE_1)
    , EncryptionMode(aes_encryption_mode::AES256)
  {

  }

  void normalize() override
  {
    Version = clamp(aes_version::AE_1, aes_version::AE_2, Version);
    EncryptionMode = clamp(aes_encryption_mode::AES128, aes_encryption_mode::AES256, EncryptionMode);
  }

  size_t      BufferCapacity;
  aes_version Version;
  aes_encryption_mode EncryptionMode;
};
