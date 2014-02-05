#pragma once
#include <istream>
#include "streambuffs/compression_decoder_streambuf.h"

/**
 * \brief Basic generic compression decoder stream.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_compression_decoder_stream
  : public std::basic_istream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    basic_compression_decoder_stream()
      : std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_compressionDecoderStreambuf)
    {

    }

    basic_compression_decoder_stream(std::shared_ptr<compression_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>> compressionDecoder, std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream)
      : std::basic_istream<ELEM_TYPE, TRAITS_TYPE>(&_compressionDecoderStreambuf)
      , _compressionDecoderStreambuf(compressionDecoder, stream)
    {

    }

    bool init(std::shared_ptr<compression_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>> compressionDecoder, std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& input)
    {
      return _compressionDecoderStreambuf.init(compressionDecoder, input);
    }

    bool is_init() const
    {
      return _compressionDecoderStreambuf.is_init();
    }

    size_t get_bytes_read() const
    {
      return _compressionDecoderStreambuf.get_bytes_read();
    }

    size_t get_bytes_written() const
    {
      return _compressionDecoderStreambuf.get_bytes_written();
    }

  private:
    compression_decoder_streambuf<ELEM_TYPE, TRAITS_TYPE> _compressionDecoderStreambuf;
};

//////////////////////////////////////////////////////////////////////////

typedef basic_compression_decoder_stream<uint8_t, std::char_traits<uint8_t>>  byte_compression_decoder_stream;
typedef basic_compression_decoder_stream<char, std::char_traits<char>>        compression_decoder_stream;
typedef basic_compression_decoder_stream<wchar_t, std::char_traits<wchar_t>>  wcompression_decoder_stream;
