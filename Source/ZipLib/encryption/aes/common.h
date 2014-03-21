#pragma once
#include <cstdint>

enum class aes_encryption_mode : uint8_t
{
  AES128 = 1,
  AES192 = 2,
  AES256 = 3
};

enum class aes_version : uint16_t
{
  AE_1 = 1,
  AE_2 = 2
};
