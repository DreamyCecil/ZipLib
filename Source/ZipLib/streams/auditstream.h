#pragma once
#include <iostream>
#include "streambuffs/audit_streambuf.h"

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_iauditstream
  : public std::basic_istream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_ios<ELEM_TYPE, TRAITS_TYPE> stream_type;

    basic_iauditstream()
      : std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_auditStreambuf)
    {

      }

    basic_iauditstream(stream_type& stream)
      : std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_auditStreambuf)
      , _auditStreambuf(stream)
    {

    }

    void set_stream(stream_type& stream)
    {
      _auditStreambuf.set_stream(stream);
    }

    size_t get_bytes_read() const
    {
      return _auditStreambuf.get_bytes_read();
    }

  private:
    audit_streambuf<ELEM_TYPE, TRAITS_TYPE> _auditStreambuf;
};

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_oauditstream
  : public std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_ios<ELEM_TYPE, TRAITS_TYPE> stream_type;

    basic_oauditstream()
      : std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_auditStreambuf)
    {

      }

    basic_oauditstream(stream_type& stream)
      : std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_auditStreambuf)
      , _auditStreambuf(stream)
    {

    }

    void set_stream(stream_type& stream)
    {
      _auditStreambuf.set_stream(stream);
    }

    size_t get_bytes_written() const
    {
      return _auditStreambuf.get_bytes_written();
    }

  private:
    audit_streambuf<ELEM_TYPE, TRAITS_TYPE> _auditStreambuf;
};

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_ioauditstream
  : public std::basic_iostream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_ios<ELEM_TYPE, TRAITS_TYPE> stream_type;

    basic_ioauditstream()
      : std::basic_iostream<ELEM_TYPE, TRAITS_TYPE>(&_auditStreambuf)
    {

      }

    basic_ioauditstream(stream_type& stream)
      : std::basic_iostream<ELEM_TYPE, TRAITS_TYPE>(&_auditStreambuf)
      , _auditStreambuf(stream)
    {

    }

    void set_stream(stream_type& stream)
    {
      _auditStreambuf.set_stream(stream);
    }

    size_t get_bytes_read() const
    {
      return _auditStreambuf.get_bytes_read();
    }

    size_t get_bytes_written() const
    {
      return _auditStreambuf.get_bytes_written();
    }

  private:
    audit_streambuf<ELEM_TYPE, TRAITS_TYPE> _auditStreambuf;
};

//////////////////////////////////////////////////////////////////////////

typedef basic_iauditstream<uint8_t, std::char_traits<uint8_t>>  byte_iauditstream;
typedef basic_iauditstream<char, std::char_traits<char>>        iauditstream;
typedef basic_iauditstream<wchar_t, std::char_traits<wchar_t>>  wiauditstream;

typedef basic_oauditstream<uint8_t, std::char_traits<uint8_t>>  byte_oauditstream;
typedef basic_oauditstream<char, std::char_traits<char>>        oauditstream;
typedef basic_oauditstream<wchar_t, std::char_traits<wchar_t>>  woauditstream;

typedef basic_ioauditstream<uint8_t, std::char_traits<uint8_t>> byte_ioauditstream;
typedef basic_ioauditstream<char, std::char_traits<char>>       ioauditstream;
typedef basic_ioauditstream<wchar_t, std::char_traits<wchar_t>> wioauditstream;
