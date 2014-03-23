#pragma once
#include "../compression_interface.h"

#include "detail/lzma_handle.h"

#include "../../extlibs/lzma/LzmaEnc.h"

struct lzma_properties
  : compression_properties_interface
{
  void normalize() override
  {
    CLzmaEncProps props;
    props.level = CompressionLevel;
    props.numThreads = IsMultithreaded ? 2 : 1;

    LzmaEncProps_Normalize(&props);

    CompressionLevel = props.level;
    IsMultithreaded = props.numThreads != 1;
  }

  void apply(detail::lzma_handle& handle)
  {
    CLzmaEncProps props;
    props.level = CompressionLevel;
    props.numThreads = IsMultithreaded ? 2 : 1;

    LzmaEnc_SetProps(handle.get_native_handle(), &props);
  }
  
  bool IsMultithreaded = true;
  int  CompressionLevel = 5;
};
