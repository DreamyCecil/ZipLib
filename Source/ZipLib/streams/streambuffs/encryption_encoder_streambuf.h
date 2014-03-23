#pragma once
#include <streambuf>
#include <ostream>
#include <cstdint>
#include <memory>

#include "../../encryption/encryption_interface.h"

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class encryption_encoder_streambuf
  : public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::char_type char_type;
    typedef typename base_type::int_type  int_type;
    typedef typename base_type::pos_type  pos_type;
    typedef typename base_type::off_type  off_type;

    typedef std::basic_ios<ELEM_TYPE, TRAITS_TYPE>     stream_type;
    typedef std::basic_istream<ELEM_TYPE, TRAITS_TYPE> istream_type;
    typedef std::basic_ostream<ELEM_TYPE, TRAITS_TYPE> ostream_type;

    typedef encryption_encoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>  iencryption_encoder_type;
    typedef std::shared_ptr<iencryption_encoder_type>                   iencryption_encoder_ptr_type;

    encryption_encoder_streambuf()
    {

    }

    encryption_encoder_streambuf(iencryption_encoder_ptr_type encryptionEncoder, ostream_type& stream)
    {
      init(encryptionEncoder, stream);
    }

    encryption_encoder_streambuf(iencryption_encoder_ptr_type encryptionEncoder, encryption_properties_interface& props, ostream_type& stream)
    {
      init(encryptionEncoder, props, stream);
    }

    virtual ~encryption_encoder_streambuf()
    {
      sync();
    }

    void init(iencryption_encoder_ptr_type encryptionEncoder, ostream_type& stream)
    {
      _encryptionEncoder = encryptionEncoder;

      // encryption encoder init
      _encryptionEncoder->init(stream);

      // set stream buffer
      this->setp(_encryptionEncoder->get_buffer_begin(), _encryptionEncoder->get_buffer_end() - 1);
    }

    void init(iencryption_encoder_ptr_type encryptionEncoder, encryption_properties_interface& props, ostream_type& stream)
    {
      _encryptionEncoder = encryptionEncoder;

      // encryption encoder init
      _encryptionEncoder->init(stream, props);

      // set stream buffer
      this->setp(_encryptionEncoder->get_buffer_begin(), _encryptionEncoder->get_buffer_end() - 1);
    }

    bool is_init() const
    {
      return _encryptionEncoder->is_init();
    }

  protected:
    int_type overflow(int_type c = traits_type::eof()) override
    {
      bool is_eof = traits_type::eq_int_type(c, traits_type::eof());

      // if the input is not EOF, just put it into the buffer
      // and increment current pointer
      if (!is_eof)
      {
        *this->pptr() = traits_type::to_char_type(c);
        this->pbump(1);
      }

      // if input buffer is full or input is EOF,
      // compress the buffer
      if (this->pptr() >= this->epptr() || is_eof)
      {
        process();
      }

      // not eof
      return ~traits_type::eof();
    }

    int sync() override
    {
      process();
      _encryptionEncoder->sync();

      return 0;
    }

  private:
    void process()
    {
      std::ptrdiff_t inputLength = this->pptr() - this->pbase();
      _encryptionEncoder->encrypt_next(inputLength);

      // set pointers for new buffer
      this->setp(_encryptionEncoder->get_buffer_begin(), _encryptionEncoder->get_buffer_end() - 1);
    }

    iencryption_encoder_ptr_type _encryptionEncoder;
};
