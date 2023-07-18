#include "ZipGenericExtraField.h"
#include "../streams/serialization.h"

namespace detail {

bool ZipGenericExtraField::Deserialize(std::istream& stream, std::istream::pos_type extraFieldEnd)
{
  if ((extraFieldEnd - stream.tellg()) < HEADER_SIZE)
  {
    return false;
  }

  if (!deserialize(stream, Tag) || !deserialize(stream, Size)) return false;

  if ((extraFieldEnd - stream.tellg()) < Size)
  {
    return false;
  }

  if (!deserialize(stream, Data, Size) && Size > 0) return false;

  return true;
}

void ZipGenericExtraField::Serialize(std::ostream& stream)
{
  Size = static_cast<uint16_t>(Data.size());

  serialize(stream, Tag);
  serialize(stream, Size);
  serialize(stream, Data);
}

}
