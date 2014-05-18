#include "ZipCentralDirectoryFileHeader.h"
#include "ZipLocalFileHeader.h"

#include "../utils/stream/serialization.h"

#include <cstring>
#include <ctime>

namespace detail {

ZipCentralDirectoryFileHeader::ZipCentralDirectoryFileHeader()
{
  memset(this, 0, sizeof(ZipCentralDirectoryFileHeaderBase));
  Signature = SignatureConstant;
}

void ZipCentralDirectoryFileHeader::SyncWithLocalFileHeader(ZipLocalFileHeader& lfh)
{
  Crc32 = lfh.Crc32;
  CompressedSize = lfh.CompressedSize;
  UncompressedSize = lfh.UncompressedSize;

  FilenameLength = static_cast<uint16_t>(Filename.length());
  FileCommentLength = static_cast<uint16_t>(FileComment.length());
}

bool ZipCentralDirectoryFileHeader::Deserialize(std::istream& stream)
{
  if (sizeof(ZipCentralDirectoryFileHeaderBase) == ZipCentralDirectoryFileHeaderBase::SIZE_IN_BYTES)
  {
    utils::stream::deserialize<ZipCentralDirectoryFileHeaderBase>(stream, *this);
  }
  else
  {
    utils::stream::deserialize(stream, Signature);
    utils::stream::deserialize(stream, VersionMadeBy);
    utils::stream::deserialize(stream, VersionNeededToExtract);
    utils::stream::deserialize(stream, GeneralPurposeBitFlag);
    utils::stream::deserialize(stream, CompressionMethod);
    utils::stream::deserialize(stream, LastModificationTime);
    utils::stream::deserialize(stream, LastModificationDate);
    utils::stream::deserialize(stream, Crc32);
    utils::stream::deserialize(stream, CompressedSize);
    utils::stream::deserialize(stream, UncompressedSize);
    utils::stream::deserialize(stream, FilenameLength);
    utils::stream::deserialize(stream, ExtraFieldLength);
    utils::stream::deserialize(stream, FileCommentLength);
    utils::stream::deserialize(stream, DiskNumberStart);
    utils::stream::deserialize(stream, InternalFileAttributes);
    utils::stream::deserialize(stream, ExternalFileAttributes);
    utils::stream::deserialize(stream, RelativeOffsetOfLocalHeader);
  }

  // If there is not any other entry.
  if (stream.fail() || Signature != SignatureConstant)
  {
    stream.clear();
    stream.seekg(static_cast<std::ios::off_type>(static_cast<std::ios::streamoff>(stream.tellg()) - stream.gcount()), std::istream::beg);
    return false;
  }

  utils::stream::deserialize(stream, Filename, FilenameLength);

  if (ExtraFieldLength > 0)
  {
    ZipGenericExtraField extraField;

    auto extraFieldEnd = ExtraFieldLength + stream.tellg();

    while (extraField.Deserialize(stream, extraFieldEnd))
    {
      ExtraFields.push_back(extraField);
    }
  }

  utils::stream::deserialize(stream, FileComment, FileCommentLength);

  return true;
}

void ZipCentralDirectoryFileHeader::Serialize(std::ostream& stream)
{
  FilenameLength = static_cast<uint16_t>(Filename.length());
  FileCommentLength = static_cast<uint16_t>(FileComment.length());
  ExtraFieldLength = 0;

  for (auto& extraField : ExtraFields)
  {
    ExtraFieldLength += static_cast<uint16_t>(ZipGenericExtraField::HEADER_SIZE + extraField.Data.size());
  }

  if (sizeof(ZipCentralDirectoryFileHeaderBase) == ZipCentralDirectoryFileHeaderBase::SIZE_IN_BYTES)
  {
    utils::stream::serialize<ZipCentralDirectoryFileHeaderBase>(stream, *this);
  }
  else
  {
    utils::stream::serialize(stream, Signature);
    utils::stream::serialize(stream, VersionMadeBy);
    utils::stream::serialize(stream, VersionNeededToExtract);
    utils::stream::serialize(stream, GeneralPurposeBitFlag);
    utils::stream::serialize(stream, CompressionMethod);
    utils::stream::serialize(stream, LastModificationTime);
    utils::stream::serialize(stream, LastModificationDate);
    utils::stream::serialize(stream, Crc32);
    utils::stream::serialize(stream, CompressedSize);
    utils::stream::serialize(stream, UncompressedSize);
    utils::stream::serialize(stream, FilenameLength);
    utils::stream::serialize(stream, ExtraFieldLength);
    utils::stream::serialize(stream, FileCommentLength);
    utils::stream::serialize(stream, DiskNumberStart);
    utils::stream::serialize(stream, InternalFileAttributes);
    utils::stream::serialize(stream, ExternalFileAttributes);
    utils::stream::serialize(stream, RelativeOffsetOfLocalHeader);
  }

  utils::stream::serialize(stream, Filename);

  if (ExtraFieldLength > 0)
  {
    for (auto& extraField : ExtraFields)
    {
      extraField.Serialize(stream);
    }
  }

  utils::stream::serialize(stream, FileComment);
}

}
