#pragma once
#include "encryption_header.h"

#include "../../../extlibs/zlib/zlib.h"

namespace detail {

  namespace {
    static uint32_t zipcrypto_crc32_byte(uint32_t prevCrc32, uint8_t c)
    {
      return uint32_t(get_crc_table()[(prevCrc32 ^ c) & 0xff] ^ (prevCrc32 >> 8));
    }
  }

  struct zipcrypto_keys
    : encryption_header_base
  {
    zipcrypto_keys()
    {
      header.u32[0] = 0x12345678;
      header.u32[1] = 0x23456789;
      header.u32[2] = 0x34567890;
    }

    void update(uint8_t c)
    {
      header.u32[0] = zipcrypto_crc32_byte(header.u32[0], c);
      header.u32[1] = header.u32[1] + uint8_t(header.u32[0] & 0xff);
      header.u32[1] = header.u32[1] * 0x08088405 + 1;
      header.u32[2] = zipcrypto_crc32_byte(header.u32[2], header.u32[1] >> 24);
    }

    uint8_t encrypt_byte(uint8_t c)
    {
      uint8_t result = uint8_t(c ^ get_magic_byte());
      update(c);
      return result;
    }

    uint8_t decrypt_byte(uint8_t c)
    {
      uint8_t result = uint8_t(c ^ get_magic_byte());
      update(result);
      return result;
    }

    void inplace_encrypt_byte(uint8_t& c)
    {
      c = encrypt_byte(c);
    }

    void inplace_decrypt_byte(uint8_t& c)
    {
      c = decrypt_byte(c);
    }

    private:
      uint8_t get_magic_byte() const
      {
        uint16_t t = uint16_t(uint16_t(header.u32[2] & 0xFFFF) | 2);
        return uint8_t((t * (t ^ 1)) >> 8);
      }
  };

}
