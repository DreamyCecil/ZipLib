#pragma once
#include <streambuf>
#include <ostream>
#include <cstdint>
#include <memory>

#include "../../encryption/encryption_interface.h"

class encryption_encoder_streambuf
  : public std::streambuf
{
  public:
    typedef std::shared_ptr<encryption_encoder_interface> encryption_encoder_interface_ptr;

    encryption_encoder_streambuf()
    {

    }

    encryption_encoder_streambuf(encryption_encoder_interface_ptr encryptionEncoder, std::ostream& stream)
    {
      init(encryptionEncoder, stream);
    }

    encryption_encoder_streambuf(encryption_encoder_interface_ptr encryptionEncoder, encryption_properties_interface& props, std::ostream& stream)
    {
      init(encryptionEncoder, props, stream);
    }

    virtual ~encryption_encoder_streambuf()
    {
      sync();
    }

    void init(encryption_encoder_interface_ptr encryptionEncoder, std::ostream& stream)
    {
      _encryptionEncoder = encryptionEncoder;

      // encryption encoder init
      _encryptionEncoder->init(stream);

      // set stream buffer
      this->setp(_encryptionEncoder->get_buffer_begin(), _encryptionEncoder->get_buffer_end() - 1);
    }

    void init(encryption_encoder_interface_ptr encryptionEncoder, encryption_properties_interface& props, std::ostream& stream)
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

    encryption_encoder_interface_ptr _encryptionEncoder;
};
