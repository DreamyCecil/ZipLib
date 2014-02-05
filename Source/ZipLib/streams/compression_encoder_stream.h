#pragma once
#include <ostream>
#include "streambuffs/compression_encoder_streambuf.h"

/**
 * \brief Basic generic compression encoder stream.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_compression_encoder_stream
  : public std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    basic_compression_encoder_stream()
      : std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_compressionEncoderStreambuf)
    {

    }

    basic_compression_encoder_stream(std::shared_ptr<compression_encoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>> compressionEncoder, std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream)
      : std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>(&_compressionEncoderStreambuf)
      , _compressionEncoderStreambuf(compressionEncoder, stream)
    {

    }

    void init(std::shared_ptr<compression_encoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>> compressionEncoder, std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& output)
    {
      _compressionEncoderStreambuf.init(compressionEncoder, output);
    }
      
    bool is_init() const
    {
      return _compressionEncoderStreambuf.is_init();
    }

    size_t get_bytes_read() const
    {
      return _compressionEncoderStreambuf.get_bytes_read();
    }

    size_t get_bytes_written() const
    {
      return _compressionEncoderStreambuf.get_bytes_written();
    }

  private:
    compression_encoder_streambuf<ELEM_TYPE, TRAITS_TYPE> _compressionEncoderStreambuf;
};

//////////////////////////////////////////////////////////////////////////

typedef basic_compression_encoder_stream<uint8_t, std::char_traits<uint8_t>>  byte_compression_encoder_stream;
typedef basic_compression_encoder_stream<char, std::char_traits<char>>        compression_encoder_stream;
typedef basic_compression_encoder_stream<wchar_t, std::char_traits<wchar_t>>  wcompression_encoder_stream;
