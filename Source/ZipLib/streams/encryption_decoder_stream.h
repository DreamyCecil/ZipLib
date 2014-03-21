#pragma once
#include <istream>
#include "streambuffs/encryption_decoder_streambuf.h"

/**
 * \brief Basic generic encryption decoder stream.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_encryption_decoder_stream
  : public std::basic_istream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename encryption_decoder_streambuf<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename encryption_decoder_streambuf<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    typedef typename encryption_decoder_streambuf<ELEM_TYPE, TRAITS_TYPE>::iencryption_decoder_type iencryption_decoder_type;
    typedef typename encryption_decoder_streambuf<ELEM_TYPE, TRAITS_TYPE>::iencryption_decoder_ptr_type iencryption_decoder_ptr_type;

    basic_encryption_decoder_stream()
      : std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_encryptionDecoderStreambuf)
    {

    }

    basic_encryption_decoder_stream(iencryption_decoder_ptr_type encryptionDecoder, istream_type& stream)
      : std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_encryptionDecoderStreambuf)
      , _encryptionDecoderStreambuf(encryptionDecoder, stream)
    {

    }

    basic_encryption_decoder_stream(iencryption_decoder_ptr_type encryptionDecoder, encryption_decoder_properties_interface& props, istream_type& stream)
      : std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_encryptionDecoderStreambuf)
      , _encryptionDecoderStreambuf(encryptionDecoder, props, stream)
    {

    }

    bool init(iencryption_decoder_ptr_type encryptionDecoder, istream_type& stream)
    {
      return _encryptionDecoderStreambuf.init(encryptionDecoder, stream);
    }

    bool init(iencryption_decoder_ptr_type encryptionDecoder, encryption_decoder_properties_interface& props, istream_type& stream)
    {
      return _encryptionDecoderStreambuf.init(encryptionDecoder, props, stream);
    }

    bool is_init() const
    {
      return _encryptionDecoderStreambuf.is_init();
    }

  private:
    encryption_decoder_streambuf<ELEM_TYPE, TRAITS_TYPE> _encryptionDecoderStreambuf;
};

//////////////////////////////////////////////////////////////////////////

typedef basic_encryption_decoder_stream<uint8_t, std::char_traits<uint8_t>> byte_encryption_decoder_stream;
typedef basic_encryption_decoder_stream<char, std::char_traits<char>>       encryption_decoder_stream;
typedef basic_encryption_decoder_stream<wchar_t, std::char_traits<wchar_t>> wencryption_decoder_stream;
