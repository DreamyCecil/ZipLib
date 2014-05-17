#pragma once
#include <istream>
#include "streambuffs/crc32_streambuf.h"

/**
 * \brief Basic CRC32 output stream. Computes CRC32 of input data.
 */
class crc32stream
  : public std::istream
{
  public:
    crc32stream()
      : std::istream(&_crc32Streambuf)
    {

    }

    crc32stream(std::istream& stream)
      : std::istream(&_crc32Streambuf)
      , _crc32Streambuf(stream)
    {

    }

    void init(std::istream& stream)
    {
      _crc32Streambuf.init(stream);
    }

    uint32_t get_crc32() const
    {
      return _crc32Streambuf.get_crc32();
    }

  private:
    crc32_streambuf _crc32Streambuf;
};
