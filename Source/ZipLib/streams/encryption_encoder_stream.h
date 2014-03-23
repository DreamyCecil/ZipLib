#pragma once
#include <ostream>
#include "streambuffs/encryption_encoder_streambuf.h"

/**
 * \brief Basic generic encryption encoder stream.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_encryption_encoder_stream
  : public std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename encryption_encoder_streambuf<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename encryption_encoder_streambuf<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    typedef typename encryption_encoder_streambuf<ELEM_TYPE, TRAITS_TYPE>::iencryption_encoder_type iencryption_encoder_type;
    typedef typename encryption_encoder_streambuf<ELEM_TYPE, TRAITS_TYPE>::iencryption_encoder_ptr_type iencryption_encoder_ptr_type;

    basic_encryption_encoder_stream()
      : std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_encryptionEncoderStreambuf)
    {

    }

    basic_encryption_encoder_stream(iencryption_encoder_ptr_type encryptionEncoder, ostream_type& stream)
      : std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_encryptionEncoderStreambuf)
      , _encryptionEncoderStreambuf(encryptionEncoder, stream)
    {

    }

    basic_encryption_encoder_stream(iencryption_encoder_ptr_type encryptionEncoder, encryption_properties_interface& props, ostream_type& stream)
      : std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_encryptionEncoderStreambuf)
      , _encryptionEncoderStreambuf(encryptionEncoder, props, stream)
    {

    }

    void init(iencryption_encoder_ptr_type encryptionEncoder, ostream_type& stream)
    {
      _encryptionEncoderStreambuf.init(encryptionEncoder, stream);
    }

    void init(iencryption_encoder_ptr_type encryptionEncoder, encryption_properties_interface& props, ostream_type& stream)
    {
      _encryptionEncoderStreambuf.init(encryptionEncoder, props, stream);
    }
      
    bool is_init() const
    {
      return _encryptionEncoderStreambuf.is_init();
    }

  private:
    encryption_encoder_streambuf<ELEM_TYPE, TRAITS_TYPE> _encryptionEncoderStreambuf;
};

//////////////////////////////////////////////////////////////////////////

typedef basic_encryption_encoder_stream<uint8_t, std::char_traits<uint8_t>> byte_encryption_encoder_stream;
typedef basic_encryption_encoder_stream<char, std::char_traits<char>>       encryption_encoder_stream;
typedef basic_encryption_encoder_stream<wchar_t, std::char_traits<wchar_t>> wencryption_encoder_stream;
