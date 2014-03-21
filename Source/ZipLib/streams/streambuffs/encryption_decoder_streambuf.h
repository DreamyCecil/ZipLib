#pragma once
#include <streambuf>
#include <istream>
#include <cstdint>
#include <memory>

#include "../../encryption/encryption_interface.h"

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class encryption_decoder_streambuf
  : public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::char_type char_type;
    typedef typename base_type::int_type  int_type;
    typedef typename base_type::pos_type  pos_type;
    typedef typename base_type::off_type  off_type;

    typedef std::basic_istream<ELEM_TYPE, TRAITS_TYPE> istream_type;
    typedef std::basic_ostream<ELEM_TYPE, TRAITS_TYPE> ostream_type;

    typedef encryption_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>  iencryption_decoder_type;
    typedef std::shared_ptr<iencryption_decoder_type>                   iencryption_decoder_ptr_type;

    encryption_decoder_streambuf()
    {

    }

    encryption_decoder_streambuf(iencryption_decoder_ptr_type encryptionDecoder, istream_type& stream)
    {
      init(encryptionDecoder, stream);
    }

    encryption_decoder_streambuf(iencryption_decoder_ptr_type encryptionDecoder, encryption_decoder_properties_interface& props, istream_type& stream)
    {
      init(encryptionDecoder, props, stream);
    }

    void init(iencryption_decoder_ptr_type encryptionDecoder, istream_type& stream)
    {
      _encryptionDecoder = encryptionDecoder;

      // encryption decoder init
      _encryptionDecoder->init(stream);

      // set stream buffer
      this->setg(_encryptionDecoder->get_buffer_end(), _encryptionDecoder->get_buffer_end(), _encryptionDecoder->get_buffer_end());
    }

    void init(iencryption_decoder_ptr_type encryptionDecoder, encryption_decoder_properties_interface& props, istream_type& stream)
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
        ELEM_TYPE* base = _encryptionDecoder->get_buffer_begin();

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
    iencryption_decoder_ptr_type _encryptionDecoder;
};
