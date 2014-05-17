#pragma once
#include "../encryption_interface.h"
#include "zipcrypto_properties.h"
#include "detail/zipcrypto.h"

#include <cstdint>

class zipcrypto_encoder
  : public encryption_encoder_interface
{
  public:
    zipcrypto_encoder()
    {

    }

    ~zipcrypto_encoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(std::ostream& stream) override
    {
      zipcrypto_properties props;
      init(stream, props);
    }

    void init(std::ostream& stream, encryption_properties_interface& props) override
    {
      // init buffers
      zipcrypto_properties& zipcryptoProps = static_cast<zipcrypto_properties&>(props);
      _bufferCapacity = zipcryptoProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new char_type[_bufferCapacity];

      // init stream
      _stream = &stream;

      // init zipcrypto
      const uint8_t* passwordData = reinterpret_cast<const uint8_t*>(zipcryptoProps.Password.c_str());

      _encryptionHeaderWritten = false;
      _zipcrypto.set_final_byte(zipcryptoProps.LastByteOfEncryptionHeader);
      _zipcrypto.init(passwordData);
    }

    bool is_init() const override
    {
      return _stream != nullptr;
    }

    char_type* get_buffer_begin() override
    {
      return _inputBuffer;
    }

    char_type* get_buffer_end() override
    {
      return _inputBuffer + _bufferCapacity;
    }

    void encrypt_next(size_t length) override
    {
      if (!_encryptionHeaderWritten)
      {
        write_encryption_header();
      }

      _zipcrypto.encrypt_buffer(reinterpret_cast<uint8_t*>(_inputBuffer), length);
      _stream->write(_inputBuffer, length);
    }

    void sync() override
    {
      _stream->rdbuf()->pubsync();
    }

  private:
    void write_encryption_header()
    {
      _zipcrypto.encrypt_header();
      _stream->write(
        reinterpret_cast<const char_type*>(&_zipcrypto.get_encryption_header()),
        _zipcrypto.get_encryption_header_size());

      _encryptionHeaderWritten = true;
    }

    void uninit_buffers()
    {
      delete[] _inputBuffer;
    }

    char_type* _inputBuffer       = nullptr;
    size_t     _bufferCapacity    = 0;

    std::ostream* _stream         = nullptr;
    detail::zipcrypto _zipcrypto;
    bool _encryptionHeaderWritten = false;
};
