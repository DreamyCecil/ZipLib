#pragma once
#include <streambuf>
#include <ostream>
#include <cstdint>
#include <memory>

#include "../../compression/compression_interface.h"

class compression_encoder_streambuf
  : public std::streambuf
{
  public:
    typedef std::shared_ptr<compression_encoder_interface> compression_encoder_interface_ptr;

    compression_encoder_streambuf()
    {

    }

    compression_encoder_streambuf(compression_encoder_interface_ptr compressionEncoder, std::ostream& stream)
    {
      init(compressionEncoder, stream);
    }

    compression_encoder_streambuf(compression_encoder_interface_ptr compressionEncoder, compression_properties_interface& props, std::ostream& stream)
    {
      init(compressionEncoder, props, stream);
    }

    virtual ~compression_encoder_streambuf()
    {
      sync();
    }

    void init(compression_encoder_interface_ptr compressionEncoder, std::ostream& stream)
    {
      _compressionEncoder = compressionEncoder;

      // compression encoder init
      _compressionEncoder->init(stream);

      // set stream buffer
      this->setp(get_buffer_begin(), get_buffer_end() - 1);
    }

    void init(compression_encoder_interface_ptr compressionEncoder, compression_properties_interface& props, std::ostream& stream)
    {
      _compressionEncoder = compressionEncoder;

      // compression encoder init
      _compressionEncoder->init(stream, props);

      // set stream buffer
      this->setp(get_buffer_begin(), get_buffer_end() - 1);
    }

    bool is_init() const
    {
      return _compressionEncoder->is_init();
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
      _compressionEncoder->sync();

      return 0;
    }

  private:
    void process()
    {
      std::ptrdiff_t inputLength = this->pptr() - this->pbase();
      _compressionEncoder->encode_next(inputLength);

      // set pointers for new buffer
      this->setp(get_buffer_begin(), get_buffer_end() - 1);
    }

    char_type* get_buffer_begin()
    {
      return reinterpret_cast<char_type*>(_compressionEncoder->get_buffer_begin());
    }

    char_type* get_buffer_end()
    {
      return reinterpret_cast<char_type*>(_compressionEncoder->get_buffer_end());
    }

    compression_encoder_interface_ptr _compressionEncoder;
};
