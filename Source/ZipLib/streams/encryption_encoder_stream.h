#pragma once
#include <ostream>
#include "streambuffs/encryption_encoder_streambuf.h"

/**
 * \brief Basic generic encryption encoder stream.
 */
class encryption_encoder_stream
  : public std::ostream
{
  public:
    typedef encryption_encoder_streambuf::encryption_encoder_interface_ptr encryption_encoder_interface_ptr;

    encryption_encoder_stream()
      : std::ostream(&_encryptionEncoderStreambuf)
    {

    }

    encryption_encoder_stream(encryption_encoder_interface_ptr encryptionEncoder, std::ostream& stream)
      : std::ostream(&_encryptionEncoderStreambuf)
      , _encryptionEncoderStreambuf(encryptionEncoder, stream)
    {

    }

    encryption_encoder_stream(encryption_encoder_interface_ptr encryptionEncoder, encryption_properties_interface& props, std::ostream& stream)
      : std::ostream(&_encryptionEncoderStreambuf)
      , _encryptionEncoderStreambuf(encryptionEncoder, props, stream)
    {

    }

    void init(encryption_encoder_interface_ptr encryptionEncoder, std::ostream& stream)
    {
      _encryptionEncoderStreambuf.init(encryptionEncoder, stream);
    }

    void init(encryption_encoder_interface_ptr encryptionEncoder, encryption_properties_interface& props, std::ostream& stream)
    {
      _encryptionEncoderStreambuf.init(encryptionEncoder, props, stream);
    }
      
    bool is_init() const
    {
      return _encryptionEncoderStreambuf.is_init();
    }

  private:
    encryption_encoder_streambuf _encryptionEncoderStreambuf;
};
