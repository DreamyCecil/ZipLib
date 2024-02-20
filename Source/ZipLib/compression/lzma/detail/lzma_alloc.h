#pragma once

#ifndef ZIPLIB_NO_LZMA

#include "../../../extlibs/lzma/Types.h"

namespace detail
{
  class lzma_alloc
    : public ISzAlloc
  {
    public:
      lzma_alloc()
      {
        this->Alloc = [](void*, size_t size)   { return malloc(size); };
        this->Free  = [](void*, void* address) { free(address); };
      }
  };
}

#endif // ZIPLIB_NO_LZMA
