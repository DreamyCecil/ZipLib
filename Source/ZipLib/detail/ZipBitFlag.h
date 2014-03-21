#pragma once
#include "../utils/enum_utils.h"
#include <cstdint>

namespace detail {

  enum class ZipBitFlag : uint16_t
  {
    None = 0,
    Encrypted = 1,
    DataDescriptor = 8,
    UnicodeFileName = 0x800
  };

  MARK_AS_TYPED_ENUMFLAGS(ZipBitFlag);

}
