#pragma once
#include <streambuf>
#include <istream>
#include <cstdint>
#include <memory>

#include "../../compression/compression_interface.h"

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class compression_decoder_streambuf
  : public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::int_type int_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;
    
    compression_decoder_streambuf()
    {

    }

    compression_decoder_streambuf(std::shared_ptr<compression_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>> compressionDecoder, std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& input)
    {
      init(compressionDecoder, input);
    }

    void init(std::shared_ptr<compression_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>> compressionDecoder, std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& input)
    {
      _compressionDecoder = compressionDecoder;

      // compression decoder init
      _compressionDecoder->init(input);

      // set stream buffer
      this->setg(_compressionDecoder->get_buffer_end(), _compressionDecoder->get_buffer_end(), _compressionDecoder->get_buffer_end());
    }

    bool is_init() const
    {
      return _compressionDecoder->is_init();
    }

    size_t get_bytes_read() const
    {
      return _compressionDecoder->get_bytes_read();
    }

    size_t get_bytes_written() const
    {
      return _compressionDecoder->get_bytes_written();
    }

  protected:
    int_type underflow() override
    {
      // buffer exhausted
      if (this->gptr() >= this->egptr())
      {
        ELEM_TYPE* base = _compressionDecoder->get_buffer_begin();

        // how many bytes has been read
        size_t n = _compressionDecoder->decode_next();

        if (n == 0)
        {
          return traits_type::eof();
        }

        // set buffer pointers
        this->setg(base, base, base + n);
      }

      return traits_type::to_int_type(*this->gptr());
    }

  private:
    std::shared_ptr<compression_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>> _compressionDecoder;
};
