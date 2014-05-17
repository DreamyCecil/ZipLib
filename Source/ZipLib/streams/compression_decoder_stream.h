#pragma once
#include <istream>
#include "streambuffs/compression_decoder_streambuf.h"

/**
 * \brief Basic generic compression decoder stream.
 */
class compression_decoder_stream
  : public std::istream
{
  public:
    typedef compression_decoder_streambuf::compression_decoder_interface_ptr compression_decoder_interface_ptr;

    compression_decoder_stream()
      : std::istream(&_compressionDecoderStreambuf)
    {

    }

    compression_decoder_stream(compression_decoder_interface_ptr compressionDecoder, std::istream& stream)
      : std::istream(&_compressionDecoderStreambuf)
      , _compressionDecoderStreambuf(compressionDecoder, stream)
    {

    }

    compression_decoder_stream(compression_decoder_interface_ptr compressionDecoder, compression_properties_interface& props, std::istream& stream)
      : std::istream(&_compressionDecoderStreambuf)
      , _compressionDecoderStreambuf(compressionDecoder, props, stream)
    {

    }

    void init(compression_decoder_interface_ptr compressionDecoder, std::istream& stream)
    {
      _compressionDecoderStreambuf.init(compressionDecoder, stream);
    }

    void init(compression_decoder_interface_ptr compressionDecoder, compression_properties_interface& props, std::istream& stream)
    {
      _compressionDecoderStreambuf.init(compressionDecoder, props, stream);
    }

    bool is_init() const
    {
      return _compressionDecoderStreambuf.is_init();
    }

  private:
    compression_decoder_streambuf _compressionDecoderStreambuf;
};
