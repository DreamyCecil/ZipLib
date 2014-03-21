#pragma once
#include "encryption_header.h"

#include <chrono>
#include <random>
#include <cstdint>

namespace detail {

  struct zipcrypto_encryption_header
    : encryption_header_base
  {
    zipcrypto_encryption_header()
    {
      auto seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::mt19937 generator(static_cast<uint32_t>(seed));

      header.u32[0] = generator();
      header.u32[1] = generator();
      header.u32[2] = generator();
    }

    void set_final_byte(uint8_t c)
    {
      header.u8[11] = c;
    }

    uint8_t get_final_byte() const
    {
      return header.u8[11];
    }
  };

}
