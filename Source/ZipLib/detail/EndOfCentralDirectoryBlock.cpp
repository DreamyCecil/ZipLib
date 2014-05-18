#include "EndOfCentralDirectoryBlock.h"
#include "../utils/stream/storage.h"
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
    utils::stream::load<EndOfCentralDirectoryBlockBase>(stream, *this);
  }
  else
  {
    utils::stream::load(stream, Signature);
    utils::stream::load(stream, NumberOfThisDisk);
    utils::stream::load(stream, NumberOfTheDiskWithTheStartOfTheCentralDirectory);
    utils::stream::load(stream, NumberOfEntriesInTheCentralDirectoryOnThisDisk);
    utils::stream::load(stream, NumberOfEntriesInTheCentralDirectory);
    utils::stream::load(stream, SizeOfCentralDirectory);
    utils::stream::load(stream, OffsetOfStartOfCentralDirectoryWithRespectToTheStartingDiskNumber);
    utils::stream::load(stream, CommentLength);
  }

  utils::stream::load(stream, Comment, CommentLength);

  return true;
}

void EndOfCentralDirectoryBlock::Serialize(std::ostream& stream)
{
  CommentLength = static_cast<uint16_t>(Comment.length());
 
  if (sizeof(EndOfCentralDirectoryBlockBase) == EndOfCentralDirectoryBlockBase::SIZE_IN_BYTES)
  {
    utils::stream::store<EndOfCentralDirectoryBlockBase>(stream, *this);
  }
  else
  {
    utils::stream::store(stream, Signature);
    utils::stream::store(stream, NumberOfThisDisk);
    utils::stream::store(stream, NumberOfTheDiskWithTheStartOfTheCentralDirectory);
    utils::stream::store(stream, NumberOfEntriesInTheCentralDirectoryOnThisDisk);
    utils::stream::store(stream, NumberOfEntriesInTheCentralDirectory);
    utils::stream::store(stream, SizeOfCentralDirectory);
    utils::stream::store(stream, OffsetOfStartOfCentralDirectoryWithRespectToTheStartingDiskNumber);
    utils::stream::store(stream, CommentLength);
  }

  utils::stream::store(stream, Comment);
}

}
