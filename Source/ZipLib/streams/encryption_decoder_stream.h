#pragma once
#include <istream>
#include "streambuffs/encryption_decoder_streambuf.h"

/**
 * \brief Basic generic encryption decoder stream.
 */
class encryption_decoder_stream
  : public std::istream
{
  public:
    typedef encryption_decoder_streambuf::encryption_decoder_interface_ptr encryption_decoder_interface_ptr;

    encryption_decoder_stream()
      : std::istream(&_encryptionDecoderStreambuf)
    {

    }

    encryption_decoder_stream(encryption_decoder_interface_ptr encryptionDecoder, std::istream& stream)
      : std::istream(&_encryptionDecoderStreambuf)
      , _encryptionDecoderStreambuf(encryptionDecoder, stream)
    {

    }

    encryption_decoder_stream(encryption_decoder_interface_ptr encryptionDecoder, encryption_properties_interface& props, std::istream& stream)
      : std::istream(&_encryptionDecoderStreambuf)
      , _encryptionDecoderStreambuf(encryptionDecoder, props, stream)
    {

    }

    void init(encryption_decoder_interface_ptr encryptionDecoder, std::istream& stream)
    {
      _encryptionDecoderStreambuf.init(encryptionDecoder, stream);
    }

    void init(encryption_decoder_interface_ptr encryptionDecoder, encryption_properties_interface& props, std::istream& stream)
    {
      _encryptionDecoderStreambuf.init(encryptionDecoder, props, stream);
    }

    bool is_init() const
    {
      return _encryptionDecoderStreambuf.is_init();
    }

  private:
    encryption_decoder_streambuf _encryptionDecoderStreambuf;
};
