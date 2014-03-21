#pragma once
#include <cstdint>
#include <cassert>

struct ZipMethodDescriptor
{
  const uint16_t VersionNeededToExtract;
  const uint16_t CompressionMethod;

  // unspecified by APPNOTE.TXT
  static const uint16_t INVALID_COMPRESSION_METHOD = 0xFFFF;

  bool HasCompressionMethod() const { return CompressionMethod != INVALID_COMPRESSION_METHOD; }
};
