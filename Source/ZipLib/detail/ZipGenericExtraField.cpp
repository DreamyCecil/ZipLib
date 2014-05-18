#include "ZipGenericExtraField.h"
#include "../utils/stream/serialization.h"

namespace detail {

bool ZipGenericExtraField::Deserialize(std::istream& stream, std::istream::pos_type extraFieldEnd)
{
  if ((extraFieldEnd - stream.tellg()) < HEADER_SIZE)
  {
    return false;
  }

  utils::stream::deserialize(stream, Tag);
  utils::stream::deserialize(stream, Size);

  if ((extraFieldEnd - stream.tellg()) < Size)
  {
    return false;
  }

  utils::stream::deserialize(stream, Data, Size);

  return true;
}

void ZipGenericExtraField::Serialize(std::ostream& stream)
{
  Size = static_cast<uint16_t>(Data.size());

  utils::stream::serialize(stream, Tag);
  utils::stream::serialize(stream, Size);
  utils::stream::serialize(stream, Data);
}

}
