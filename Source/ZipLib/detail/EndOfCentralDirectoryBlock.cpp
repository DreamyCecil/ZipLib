#include "EndOfCentralDirectoryBlock.h"
#include "../utils/stream/serialization.h"
#include <cstring>

namespace detail {

EndOfCentralDirectoryBlock::EndOfCentralDirectoryBlock()
{
  memset(this, 0, sizeof(EndOfCentralDirectoryBlockBase));
  Signature = SignatureConstant;
}

bool EndOfCentralDirectoryBlock::Deserialize(std::istream& stream)
{
  // this condition should be optimized out :)
  if (sizeof(EndOfCentralDirectoryBlockBase) == EndOfCentralDirectoryBlockBase::SIZE_IN_BYTES)
  {
    utils::stream::deserialize<EndOfCentralDirectoryBlockBase>(stream, *this);
  }
  else
  {
    utils::stream::deserialize(stream, Signature);
    utils::stream::deserialize(stream, NumberOfThisDisk);
    utils::stream::deserialize(stream, NumberOfTheDiskWithTheStartOfTheCentralDirectory);
    utils::stream::deserialize(stream, NumberOfEntriesInTheCentralDirectoryOnThisDisk);
    utils::stream::deserialize(stream, NumberOfEntriesInTheCentralDirectory);
    utils::stream::deserialize(stream, SizeOfCentralDirectory);
    utils::stream::deserialize(stream, OffsetOfStartOfCentralDirectoryWithRespectToTheStartingDiskNumber);
    utils::stream::deserialize(stream, CommentLength);
  }

  utils::stream::deserialize(stream, Comment, CommentLength);

  return true;
}

void EndOfCentralDirectoryBlock::Serialize(std::ostream& stream)
{
  CommentLength = static_cast<uint16_t>(Comment.length());
 
  if (sizeof(EndOfCentralDirectoryBlockBase) == EndOfCentralDirectoryBlockBase::SIZE_IN_BYTES)
  {
    utils::stream::serialize<EndOfCentralDirectoryBlockBase>(stream, *this);
  }
  else
  {
    utils::stream::serialize(stream, Signature);
    utils::stream::serialize(stream, NumberOfThisDisk);
    utils::stream::serialize(stream, NumberOfTheDiskWithTheStartOfTheCentralDirectory);
    utils::stream::serialize(stream, NumberOfEntriesInTheCentralDirectoryOnThisDisk);
    utils::stream::serialize(stream, NumberOfEntriesInTheCentralDirectory);
    utils::stream::serialize(stream, SizeOfCentralDirectory);
    utils::stream::serialize(stream, OffsetOfStartOfCentralDirectoryWithRespectToTheStartingDiskNumber);
    utils::stream::serialize(stream, CommentLength);
  }

  utils::stream::serialize(stream, Comment);
}

}
