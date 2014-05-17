#pragma once
#include "../encryption_interface.h"
#include "zipcrypto_properties.h"
#include "../../extlibs/zlib/zconf.h"
#include "detail/zipcrypto.h"

#include <cstdint>

class zipcrypto_decoder
  : public encryption_decoder_interface
{
  public:
    zipcrypto_decoder()
    {

    }

    ~zipcrypto_decoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(std::istream& stream) override
    {
      zipcrypto_properties props;
      init(stream, props);
    }

    void init(std::istream& stream, encryption_properties_interface& props) override
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

      _encryptionHeaderRead = false;
      _zipcrypto.set_final_byte(zipcryptoProps.LastByteOfEncryptionHeader);
      _zipcrypto.init(passwordData);
      read_encryption_header();

      // check if stream has not set failbit
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

    size_t decrypt_next() override
    {
      if (!_encryptionHeaderRead)
      {
        read_encryption_header();
      }

      _stream->read(_inputBuffer, _bufferCapacity);

      size_t n = static_cast<size_t>(_stream->gcount());
      _zipcrypto.decrypt_buffer(reinterpret_cast<uint8_t*>(_inputBuffer), n);

      return n;
    }

    bool has_correct_password() const override
    {
      return _encryptionHeaderRead ? _zipcrypto.has_correct_password() : false;
    }

  private:
    void read_encryption_header()
    {
      _stream->read(
        reinterpret_cast<char_type*>(&_zipcrypto.get_encryption_header()),
        _zipcrypto.get_encryption_header_size());

      _zipcrypto.decrypt_header();
      _encryptionHeaderRead = true;
    }

    void uninit_buffers()
    {
      delete[] _inputBuffer;
    }

    char_type* _inputBuffer = nullptr;
    size_t     _bufferCapacity = 0;

    std::istream* _stream = nullptr;
    detail::zipcrypto _zipcrypto;
    bool _encryptionHeaderRead = false;
};
