#pragma once
#include "../MethodTemplate.h"
#include "../../compression/store/store_encoder.h"
#include "../../compression/store/store_decoder.h"

#include <memory>

class StoreMethod :
  public CompressionMethodTemplate
  <
      StoreMethod,
      store_encoder, store_decoder, store_properties,
      /* CompressionMethod */ 0,
      /* VersionNeededToExtract */ 10
  >
{

};
