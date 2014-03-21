#pragma once
#include <cstdint>

namespace detail {

  union encryption_header
  {
    uint8_t  u8[12];
    uint32_t u32[3];
  };

  struct encryption_header_base
  {
    encryption_header header;
  };

}
