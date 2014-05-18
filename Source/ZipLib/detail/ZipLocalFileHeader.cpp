#include "ZipLocalFileHeader.h"
#include "ZipCentralDirectoryFileHeader.h"

#include "../utils/stream/storage.h"

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
    utils::stream::load<ZipLocalFileHeaderBase>(stream, *this);
  }
  else
  {
    utils::stream::load(stream, Signature);
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
  }

  // If there is not any other entry.
  if (stream.fail() || Signature != SignatureConstant)
  {
    stream.clear();
    stream.seekg(static_cast<std::ios::off_type>(static_cast<std::ios::streamoff>(stream.tellg()) - stream.gcount()), std::ios::beg);
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
    utils::stream::store<ZipLocalFileHeaderBase>(stream, *this);
  }
  else
  {
    utils::stream::store(stream, Signature);
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
  }

  utils::stream::store(stream, Filename);

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
  utils::stream::load(stream, firstWord);

  // the signature is optional, if it's missing,
  // we're starting with crc32
  if (firstWord != DataDescriptorSignature)
  {
    utils::stream::load(stream, Crc32);
  }
  else
  {
    Crc32 = firstWord;
  }

  utils::stream::load(stream, CompressedSize);
  utils::stream::load(stream, UncompressedSize);
}

void ZipLocalFileHeader::SerializeAsDataDescriptor(std::ostream& stream)
{
  utils::stream::store(stream, DataDescriptorSignature);
  utils::stream::store(stream, Crc32);
  utils::stream::store(stream, CompressedSize);
  utils::stream::store(stream, UncompressedSize);
}

}
