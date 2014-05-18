#include "ZipCentralDirectoryFileHeader.h"
#include "ZipLocalFileHeader.h"

#include "../utils/stream/storage.h"

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
    utils::stream::load<ZipCentralDirectoryFileHeaderBase>(stream, *this);
  }
  else
  {
    utils::stream::load(stream, Signature);
    utils::stream::load(stream, VersionMadeBy);
    utils::stream::load(stream, VersionNeededToExtract);
    utils::stream::load(stream, GeneralPurposeBitFlag);
    utils::stream::load(stream, CompressionMethod);
    utils::stream::load(stream, LastModificationTime);
    utils::stream::load(stream, LastModificationDate);
    utils::stream::load(stream, Crc32);
    utils::stream::load(stream, CompressedSize);
    utils::stream::load(stream, UncompressedSize);
    utils::stream::load(stream, FilenameLength);
    utils::stream::load(stream, ExtraFieldLength);
    utils::stream::load(stream, FileCommentLength);
    utils::stream::load(stream, DiskNumberStart);
    utils::stream::load(stream, InternalFileAttributes);
    utils::stream::load(stream, ExternalFileAttributes);
    utils::stream::load(stream, RelativeOffsetOfLocalHeader);
  }

  // If there is not any other entry.
  if (stream.fail() || Signature != SignatureConstant)
  {
    stream.clear();
    stream.seekg(static_cast<std::ios::off_type>(static_cast<std::ios::streamoff>(stream.tellg()) - stream.gcount()), std::istream::beg);
    return false;
  }

  utils::stream::load(stream, Filename, FilenameLength);

  if (ExtraFieldLength > 0)
  {
    ZipGenericExtraField extraField;

    auto extraFieldEnd = ExtraFieldLength + stream.tellg();

    while (extraField.Deserialize(stream, extraFieldEnd))
    {
      ExtraFields.push_back(extraField);
    }
  }

  utils::stream::load(stream, FileComment, FileCommentLength);

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
    utils::stream::store<ZipCentralDirectoryFileHeaderBase>(stream, *this);
  }
  else
  {
    utils::stream::store(stream, Signature);
    utils::stream::store(stream, VersionMadeBy);
    utils::stream::store(stream, VersionNeededToExtract);
    utils::stream::store(stream, GeneralPurposeBitFlag);
    utils::stream::store(stream, CompressionMethod);
    utils::stream::store(stream, LastModificationTime);
    utils::stream::store(stream, LastModificationDate);
    utils::stream::store(stream, Crc32);
    utils::stream::store(stream, CompressedSize);
    utils::stream::store(stream, UncompressedSize);
    utils::stream::store(stream, FilenameLength);
    utils::stream::store(stream, ExtraFieldLength);
    utils::stream::store(stream, FileCommentLength);
    utils::stream::store(stream, DiskNumberStart);
    utils::stream::store(stream, InternalFileAttributes);
    utils::stream::store(stream, ExternalFileAttributes);
    utils::stream::store(stream, RelativeOffsetOfLocalHeader);
  }

  utils::stream::store(stream, Filename);

  if (ExtraFieldLength > 0)
  {
    for (auto& extraField : ExtraFields)
    {
      extraField.Serialize(stream);
    }
  }

  utils::stream::store(stream, FileComment);
}

}
