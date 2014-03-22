#pragma once
#include "common.h"
#include "../encryption_interface.h"

struct aes_decoder_properties
  : encryption_decoder_properties_interface
{
  aes_version Version = aes_version::AE_1;
  aes_encryption_mode EncryptionMode = aes_encryption_mode::AES256;
};
