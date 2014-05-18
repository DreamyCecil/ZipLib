#include "ZipGenericExtraField.h"
#include "../utils/stream/storage.h"

namespace detail {

bool ZipGenericExtraField::Deserialize(std::istream& stream, std::istream::pos_type extraFieldEnd)
{
  if ((extraFieldEnd - stream.tellg()) < HEADER_SIZE)
  {
    return false;
  }

  utils::stream::load(stream, Tag);
  utils::stream::load(stream, Size);

  if ((extraFieldEnd - stream.tellg()) < Size)
  {
    return false;
  }

  utils::stream::load(stream, Data, Size);

  return true;
}

void ZipGenericExtraField::Serialize(std::ostream& stream)
{
  Size = static_cast<uint16_t>(Data.size());

  utils::stream::store(stream, Tag);
  utils::stream::store(stream, Size);
  utils::stream::store(stream, Data);
}

}
