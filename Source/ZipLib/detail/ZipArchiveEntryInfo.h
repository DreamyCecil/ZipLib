#pragma once
#include "ZipLocalFileHeader.h"
#include "ZipCentralDirectoryFileHeader.h"
#include "ZipBitFlag.h"
#include "../utils/time_utils.h"

#include <cstdint>

namespace detail {

  struct ZipArchiveEntryInfo
  {
    ZipLocalFileHeader            LocalFileHeader;
    ZipCentralDirectoryFileHeader CentralDirectoryFileHeader;

    //////////////////////////////////////////////////////////////////////////
    // Getters/Setters

    const std::string& GetComment() const
    {
      return CentralDirectoryFileHeader.FileComment;
    }

    void SetComment(const std::string& comment)
    {
      CentralDirectoryFileHeader.FileComment = comment;
    }

    time_t GetLastWriteTime() const
    {
      return utils::time::datetime_to_timestamp(CentralDirectoryFileHeader.LastModificationDate, CentralDirectoryFileHeader.LastModificationTime);
    }

    void SetLastWriteTime(time_t modTime)
    {
      utils::time::timestamp_to_datetime(modTime, CentralDirectoryFileHeader.LastModificationDate, CentralDirectoryFileHeader.LastModificationTime);
    }

    void SetCompressionMethod(uint16_t value)
    {
      CentralDirectoryFileHeader.CompressionMethod = value;
    }

    ZipBitFlag GetGeneralPurposeBitFlag() const
    {
      return static_cast<ZipBitFlag>(CentralDirectoryFileHeader.GeneralPurposeBitFlag);
    }

    void SetGeneralPurposeBitFlag(ZipBitFlag value, bool set = true)
    {
      if (set)
      {
        CentralDirectoryFileHeader.GeneralPurposeBitFlag |= static_cast<uint16_t>(value);
      }
      else
      {
        CentralDirectoryFileHeader.GeneralPurposeBitFlag &= static_cast<uint16_t>(~value);
      }
    }

    uint16_t GetVersionMadeBy() const
    {
      return CentralDirectoryFileHeader.VersionMadeBy;
    }

    void SetVersionMadeBy(uint16_t value)
    {
      CentralDirectoryFileHeader.VersionMadeBy = value;
    }

    int32_t GetOffsetOfLocalHeader() const
    {
      return CentralDirectoryFileHeader.RelativeOffsetOfLocalHeader;
    }

    void SetOffsetOfLocalHeader(int32_t value)
    {
      CentralDirectoryFileHeader.RelativeOffsetOfLocalHeader = static_cast<int32_t>(value);
    }

    void FixVersionToExtractAtLeast(uint16_t value)
    {
      if (CentralDirectoryFileHeader.VersionNeededToExtract < value)
      {
        CentralDirectoryFileHeader.VersionNeededToExtract = value;
      }
    }

    //////////////////////////////////////////////////////////////////////////
    // Sync methods

    void SyncWithCentralDirectoryFileHeader()
    {
      LocalFileHeader.SyncWithCentralDirectoryFileHeader(CentralDirectoryFileHeader);
    }

    void SyncWithLocalFileHeader()
    {
      CentralDirectoryFileHeader.SyncWithLocalFileHeader(LocalFileHeader);
    }
  };

}
