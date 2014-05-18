#pragma once
#include "../../../detail/ZipGenericExtraField.h"
#include "../../../streams/memstream.h"
#include "../../../utils/stream/serialization.h"
#include <cstdint>

namespace detail {

  struct AesExtraField
  {
    enum : uint16_t { AES_TAG = 0x9901 };
    enum : size_t   { SIZE_IN_BYTES = 7 };

    uint16_t AesVersion;
    const uint16_t VendorID;
    uint8_t  EncryptionMode;
    uint16_t CompressionMethod;

    AesExtraField()
      : AesVersion(0)
      , VendorID('EA')
      , EncryptionMode(0)
      , CompressionMethod(0)
    {

    }

    void FromExtraField(const detail::ZipGenericExtraField& extraField)
    {
      if (extraField.Tag != AES_TAG || extraField.Size != SIZE_IN_BYTES)
      {
        return;
      }

      imemstream extraFieldDataStream((const char*)extraField.Data.data(), extraField.Data.size());

      uint16_t dummyVendorID;

      utils::stream::deserialize(extraFieldDataStream, AesVersion);
      utils::stream::deserialize(extraFieldDataStream, dummyVendorID);
      utils::stream::deserialize(extraFieldDataStream, EncryptionMode);
      utils::stream::deserialize(extraFieldDataStream, CompressionMethod);
    }

    void ToExtraField(detail::ZipGenericExtraField& extraField)
    {
      extraField.Tag = AES_TAG;
      extraField.Size = SIZE_IN_BYTES;

      extraField.Data.resize(SIZE_IN_BYTES);

      omemstream extraFieldDataStream((char*)extraField.Data.data(), extraField.Data.size());

      utils::stream::serialize(extraFieldDataStream, AesVersion);
      utils::stream::serialize(extraFieldDataStream, VendorID);
      utils::stream::serialize(extraFieldDataStream, EncryptionMode);
      utils::stream::serialize(extraFieldDataStream, CompressionMethod);
    }
  };

}
