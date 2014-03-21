#pragma once
#include "../../../detail/ZipGenericExtraField.h"
#include "../../../streams/memstream.h"
#include "../../../streams/serialization.h"
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

      byte_imemstream extraFieldDataStream(extraField.Data.data(), extraField.Data.size());

      uint16_t dummyVendorID;

      deserialize(extraFieldDataStream, AesVersion);
      deserialize(extraFieldDataStream, dummyVendorID);
      deserialize(extraFieldDataStream, EncryptionMode);
      deserialize(extraFieldDataStream, CompressionMethod);
    }

    void ToExtraField(detail::ZipGenericExtraField& extraField)
    {
      extraField.Tag = AES_TAG;
      extraField.Size = SIZE_IN_BYTES;

      extraField.Data.resize(SIZE_IN_BYTES);

      byte_omemstream extraFieldDataStream(extraField.Data.data(), extraField.Data.size());

      serialize(extraFieldDataStream, AesVersion);
      serialize(extraFieldDataStream, VendorID);
      serialize(extraFieldDataStream, EncryptionMode);
      serialize(extraFieldDataStream, CompressionMethod);
    }
  };

}
