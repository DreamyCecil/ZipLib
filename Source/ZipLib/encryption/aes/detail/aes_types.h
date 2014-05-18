#pragma once
#include <cstdint>
#include <array>

namespace detail {

  enum
  {
    AES_PASSWORD_VERIFY_SIZE  = 2,
    AES_AUTHCODE_SIZE         = 10,
    AES_MAX_SALT_LENGTH       = 16
  };

  using aes_passverify = std::array<uint8_t, AES_PASSWORD_VERIFY_SIZE>;
  using aes_authcode = std::array<uint8_t, AES_AUTHCODE_SIZE>;

}
