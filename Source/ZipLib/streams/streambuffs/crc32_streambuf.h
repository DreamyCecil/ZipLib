#pragma once
#include <streambuf>
#include <cstdint>

#include "streambuf_conf.h"
#include "../../extlibs/zlib/zlib.h"

template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class crc32_streambuf
  : public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::int_type int_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;

    crc32_streambuf()
      : _crc32(0)
    {

    }

    uint32_t get_crc32() const
    {
      return _crc32;
    }

  protected:
    int_type overflow(int_type c = traits_type::eof()) override
    {
      bool is_eof = traits_type::eq_int_type(c, traits_type::eof());

      // buffering would be great, maybe?
      if (!is_eof)
      {
        _crc32 = crc32(_crc32, reinterpret_cast<Bytef*>(&c), static_cast<uInt>(sizeof(ELEM_TYPE)));
        return c;
      }

      return traits_type::eof();
    }

  private:
    uint32_t _crc32;
};
