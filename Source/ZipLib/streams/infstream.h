#pragma once
#include <istream>
#include "streambuffs/inflate_streambuf.h"

/**
 * \brief Basic inflate stream.
 *        Decompresses data using the DEFLATE algorithm.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class basic_infstream
  : public std::basic_istream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    basic_infstream()
      : std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_inflateStreambuff)
    {

    }

    basic_infstream(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream)
      : _inflateStreambuff(stream)
      , std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_inflateStreambuff)
    {

    }

    bool init(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& input)
    {
      return _inflateStreambuff.init(input);
    }

    bool is_init() const
    {
      return _inflateStreambuff.is_init();
    }

    size_t get_bytes_read() const
    {
      return _inflateStreambuff.get_bytes_read();
    }

    size_t get_bytes_written() const
    {
      return _inflateStreambuff.get_bytes_written();
    }

  private:
    inflate_streambuf<ELEM_TYPE, TRAITS_TYPE> _inflateStreambuff;
};

//////////////////////////////////////////////////////////////////////////

typedef basic_infstream<uint8_t, std::char_traits<uint8_t>>  byte_infstream;
typedef basic_infstream<char, std::char_traits<char>>        infstream;
typedef basic_infstream<wchar_t, std::char_traits<wchar_t>>  winfstream;
