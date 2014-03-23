#pragma once
#include "common.h"
#include "../encryption_interface.h"

struct aes_properties
  : encryption_properties_interface
{
  void normalize() override
  {
    Version = clamp(aes_version::AE_1, aes_version::AE_2, Version);
    EncryptionMode = clamp(aes_encryption_mode::AES128, aes_encryption_mode::AES256, EncryptionMode);
  }

  aes_version Version = aes_version::AE_1;
  aes_encryption_mode EncryptionMode = aes_encryption_mode::AES256;
};
