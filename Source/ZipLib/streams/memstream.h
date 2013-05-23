#pragma once
#include <iostream>
#include <cstdint>
#include "streambuffs/mem_streambuf.h"

#ifdef _MSC_VER
# pragma warning(disable : 4250)
#endif

template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class basic_imemstream
  : public std::basic_istream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    basic_imemstream(ELEM_TYPE* buffer, size_t length)
      : _memStreambuf(buffer, length)
      , std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_memStreambuf)
    {

    }

  private:
    mem_streambuf<ELEM_TYPE, TRAITS_TYPE> _memStreambuf;
};

template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class basic_omemstream
  : public std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    basic_omemstream(ELEM_TYPE* buffer, size_t length)
      : _memStreambuf(buffer, length)
      , std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_memStreambuf)
    {

    }

  private:
    mem_streambuf<ELEM_TYPE, TRAITS_TYPE> _memStreambuf;
};

template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class basic_iomemstream
  : public std::basic_iostream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    basic_iomemstream(ELEM_TYPE* buffer, size_t length)
      : _memStreambuf(buffer, length)
      , std::basic_iostream<ELEM_TYPE, TRAITS_TYPE>(&_memStreambuf)
    {

    }

  private:
    mem_streambuf<ELEM_TYPE, TRAITS_TYPE> _memStreambuf;
};

//////////////////////////////////////////////////////////////////////////

typedef basic_imemstream<uint8_t, std::char_traits<uint8_t>>  byte_imemstream;
typedef basic_imemstream<char, std::char_traits<char>>        imemstream;
typedef basic_imemstream<wchar_t, std::char_traits<wchar_t>>  wimemstream;

typedef basic_omemstream<uint8_t, std::char_traits<uint8_t>>  byte_omemstream;
typedef basic_omemstream<char, std::char_traits<char>>        omemstream;
typedef basic_omemstream<wchar_t, std::char_traits<wchar_t>>  womemstream;

typedef basic_iomemstream<uint8_t, std::char_traits<uint8_t>> byte_iomemstream;
typedef basic_iomemstream<char, std::char_traits<char>>       iomemstream;
typedef basic_iomemstream<wchar_t, std::char_traits<wchar_t>> wiomemstream;
