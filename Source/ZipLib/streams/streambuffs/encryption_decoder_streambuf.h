#pragma once
#include <streambuf>
#include <istream>
#include <cstdint>
#include <memory>

#include "../../encryption/encryption_interface.h"

class encryption_decoder_streambuf
  : public std::streambuf
{
  public:
    typedef std::shared_ptr<encryption_decoder_interface> encryption_decoder_interface_ptr;

    encryption_decoder_streambuf()
    {

    }

    encryption_decoder_streambuf(encryption_decoder_interface_ptr encryptionDecoder, std::istream& stream)
    {
      init(encryptionDecoder, stream);
    }

    encryption_decoder_streambuf(encryption_decoder_interface_ptr encryptionDecoder, encryption_properties_interface& props, std::istream& stream)
    {
      init(encryptionDecoder, props, stream);
    }

    void init(encryption_decoder_interface_ptr encryptionDecoder, std::istream& stream)
    {
      _encryptionDecoder = encryptionDecoder;

      // encryption decoder init
      _encryptionDecoder->init(stream);

      // set stream buffer
      this->setg(_encryptionDecoder->get_buffer_end(), _encryptionDecoder->get_buffer_end(), _encryptionDecoder->get_buffer_end());
    }

    void init(encryption_decoder_interface_ptr encryptionDecoder, encryption_properties_interface& props, std::istream& stream)
    {
      _encryptionDecoder = encryptionDecoder;

      // encryption decoder init
      _encryptionDecoder->init(stream, props);

      // set stream buffer
      this->setg(_encryptionDecoder->get_buffer_end(), _encryptionDecoder->get_buffer_end(), _encryptionDecoder->get_buffer_end());
    }

    bool is_init() const
    {
      return _encryptionDecoder->is_init();
    }

  protected:
    int_type underflow() override
    {
      // buffer exhausted
      if (this->gptr() >= this->egptr())
      {
        char_type* base = _encryptionDecoder->get_buffer_begin();

        // how many bytes has been read
        size_t n = _encryptionDecoder->decrypt_next();

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
    encryption_decoder_interface_ptr _encryptionDecoder;
};
