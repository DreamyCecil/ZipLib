#pragma once
#include <ostream>
#include "streambuffs/deflate_streambuf.h"

/**
 * \brief Basic deflate stream.
 *        Compresses input data using DEFLATE algorithm by given compression level.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class basic_defstream
  : public std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    basic_defstream()
      : std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_deflateStreambuf)
    {

    }

    basic_defstream(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, int compressionLevel = Z_DEFAULT_COMPRESSION)
      : _deflateStreambuf(stream, compressionLevel)
      , std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_deflateStreambuf)
    {

    }

    bool init(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& output, int compressionLevel = Z_DEFAULT_COMPRESSION)
    {
      return _deflateStreambuf.init(output, compressionLevel);
    }
      
    bool is_init() const
    {
      return _deflateStreambuf.is_init();
    }

    size_t get_bytes_read() const
    {
      return _deflateStreambuf.get_bytes_read();
    }

    size_t get_bytes_written() const
    {
      return _deflateStreambuf.get_bytes_written();
    }

  private:
    deflate_streambuf<ELEM_TYPE, TRAITS_TYPE> _deflateStreambuf;
};

//////////////////////////////////////////////////////////////////////////

typedef basic_defstream<uint8_t, std::char_traits<uint8_t>>  byte_defstream;
typedef basic_defstream<char, std::char_traits<char>>        defstream;
typedef basic_defstream<wchar_t, std::char_traits<wchar_t>>  wdefstream;
