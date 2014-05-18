#include "ZipLocalFileHeader.h"
#include "ZipCentralDirectoryFileHeader.h"

#include "../utils/stream/serialization.h"

#include <cstring>

namespace detail {

ZipLocalFileHeader::ZipLocalFileHeader()
{
  memset(this, 0, sizeof(ZipLocalFileHeaderBase));
  Signature = SignatureConstant;
}

void ZipLocalFileHeader::SyncWithCentralDirectoryFileHeader(ZipCentralDirectoryFileHeader& cdfh)
{
  VersionNeededToExtract = cdfh.VersionNeededToExtract;
  GeneralPurposeBitFlag = cdfh.GeneralPurposeBitFlag;
  CompressionMethod = cdfh.CompressionMethod;
  LastModificationTime = cdfh.LastModificationTime;
  LastModificationDate = cdfh.LastModificationDate;
  Crc32 = cdfh.Crc32;
  CompressedSize = cdfh.CompressedSize;
  UncompressedSize = cdfh.UncompressedSize;

  Filename = cdfh.Filename;
  FilenameLength = static_cast<uint16_t>(Filename.length());
}

bool ZipLocalFileHeader::Deserialize(std::istream& stream)
{
  if (sizeof(ZipLocalFileHeaderBase) == ZipLocalFileHeaderBase::SIZE_IN_BYTES)
  {
    utils::stream::deserialize<ZipLocalFileHeaderBase>(stream, *this);
  }
  else
  {
    utils::stream::deserialize(stream, Signature);
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
  }

  // If there is not any other entry.
  if (stream.fail() || Signature != SignatureConstant)
  {
    stream.clear();
    stream.seekg(static_cast<std::ios::off_type>(static_cast<std::ios::streamoff>(stream.tellg()) - stream.gcount()), std::ios::beg);
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

  return true;
}

void ZipLocalFileHeader::Serialize(std::ostream& stream)
{
  FilenameLength = static_cast<uint16_t>(Filename.length());
  ExtraFieldLength = 0;

  for (auto& extraField : ExtraFields)
  {
    ExtraFieldLength += static_cast<uint16_t>(ZipGenericExtraField::HEADER_SIZE + extraField.Data.size());
  }

  if (sizeof(ZipLocalFileHeaderBase) == ZipLocalFileHeaderBase::SIZE_IN_BYTES)
  {
    utils::stream::serialize<ZipLocalFileHeaderBase>(stream, *this);
  }
  else
  {
    utils::stream::serialize(stream, Signature);
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
  }

  utils::stream::serialize(stream, Filename);

  if (ExtraFieldLength > 0)
  {
    for (auto& extraField : ExtraFields)
    {
      extraField.Serialize(stream);
    }
  }
}

void ZipLocalFileHeader::DeserializeAsDataDescriptor(std::istream& stream)
{
  uint32_t firstWord;
  utils::stream::deserialize(stream, firstWord);

  // the signature is optional, if it's missing,
  // we're starting with crc32
  if (firstWord != DataDescriptorSignature)
  {
    utils::stream::deserialize(stream, Crc32);
  }
  else
  {
    Crc32 = firstWord;
  }

  utils::stream::deserialize(stream, CompressedSize);
  utils::stream::deserialize(stream, UncompressedSize);
}

void ZipLocalFileHeader::SerializeAsDataDescriptor(std::ostream& stream)
{
  utils::stream::serialize(stream, DataDescriptorSignature);
  utils::stream::serialize(stream, Crc32);
  utils::stream::serialize(stream, CompressedSize);
  utils::stream::serialize(stream, UncompressedSize);
}

}
