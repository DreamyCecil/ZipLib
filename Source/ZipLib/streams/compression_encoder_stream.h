#pragma once
#include <ostream>
#include "streambuffs/compression_encoder_streambuf.h"

/**
 * \brief Basic generic compression encoder stream.
 */
class compression_encoder_stream
  : public std::ostream
{
  public:
    typedef compression_encoder_streambuf::compression_encoder_interface_ptr compression_encoder_interface_ptr;

    compression_encoder_stream()
      : std::ostream(&_compressionEncoderStreambuf)
    {

    }

    compression_encoder_stream(compression_encoder_interface_ptr compressionEncoder, std::ostream& stream)
      : std::ostream(&_compressionEncoderStreambuf)
      , _compressionEncoderStreambuf(compressionEncoder, stream)
    {

    }

    compression_encoder_stream(compression_encoder_interface_ptr compressionEncoder, compression_properties_interface& props, std::ostream& stream)
      : std::ostream(&_compressionEncoderStreambuf)
      , _compressionEncoderStreambuf(compressionEncoder, props, stream)
    {

    }

    void init(compression_encoder_interface_ptr compressionEncoder, std::ostream& stream)
    {
      _compressionEncoderStreambuf.init(compressionEncoder, stream);
    }

    void init(compression_encoder_interface_ptr compressionEncoder, compression_properties_interface& props, std::ostream& stream)
    {
      _compressionEncoderStreambuf.init(compressionEncoder, props, stream);
    }
      
    bool is_init() const
    {
      return _compressionEncoderStreambuf.is_init();
    }

  private:
    compression_encoder_streambuf _compressionEncoderStreambuf;
};
