#pragma once
#include <streambuf>
#include <istream>
#include <cstdint>
#include <memory>

#include "../../compression/compression_interface.h"

class compression_decoder_streambuf
  : public std::streambuf
{
  public:
    typedef std::shared_ptr<compression_decoder_interface> compression_decoder_interface_ptr;

    compression_decoder_streambuf()
    {

    }

    compression_decoder_streambuf(compression_decoder_interface_ptr compressionDecoder, std::istream& stream)
    {
      init(compressionDecoder, stream);
    }

    compression_decoder_streambuf(compression_decoder_interface_ptr compressionDecoder, compression_properties_interface& props, std::istream& stream)
    {
      init(compressionDecoder, props, stream);
    }

    void init(compression_decoder_interface_ptr compressionDecoder, std::istream& stream)
    {
      _compressionDecoder = compressionDecoder;

      // compression decoder init
      _compressionDecoder->init(stream);

      // set stream buffer
      this->setg(_compressionDecoder->get_buffer_end(), _compressionDecoder->get_buffer_end(), _compressionDecoder->get_buffer_end());
    }

    void init(compression_decoder_interface_ptr compressionDecoder, compression_properties_interface& props, std::istream& stream)
    {
      _compressionDecoder = compressionDecoder;

      // compression decoder init
      _compressionDecoder->init(stream, props);

      // set stream buffer
      this->setg(_compressionDecoder->get_buffer_end(), _compressionDecoder->get_buffer_end(), _compressionDecoder->get_buffer_end());
    }

    bool is_init() const
    {
      return _compressionDecoder->is_init();
    }

  protected:
    int_type underflow() override
    {
      // buffer exhausted
      if (this->gptr() >= this->egptr())
      {
        char_type* base = _compressionDecoder->get_buffer_begin();

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
    compression_decoder_interface_ptr _compressionDecoder;
};
