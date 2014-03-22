#pragma once
#include "../encryption_interface.h"
#include "zipcrypto_encoder_properties.h"
#include "detail/zipcrypto.h"

#include <cstdint>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_zipcrypto_encoder
  : public encryption_encoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    basic_zipcrypto_encoder()
    {

    }

    ~basic_zipcrypto_encoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(ostream_type& stream) override
    {
      zipcrypto_encoder_properties props;
      init(stream, props);
    }

    void init(ostream_type& stream, encryption_encoder_properties_interface& props) override
    {
      // init buffers
      zipcrypto_encoder_properties& zipcryptoProps = static_cast<zipcrypto_encoder_properties&>(props);
      _bufferCapacity = zipcryptoProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new ELEM_TYPE[_bufferCapacity];

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

    ELEM_TYPE* get_buffer_begin() override
    {
      return _inputBuffer;
    }

    ELEM_TYPE* get_buffer_end() override
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
        reinterpret_cast<const ELEM_TYPE*>(&_zipcrypto.get_encryption_header()),
        _zipcrypto.get_encryption_header_size());

      _encryptionHeaderWritten = true;
    }

    void uninit_buffers()
    {
      delete[] _inputBuffer;
    }

    ELEM_TYPE* _inputBuffer       = nullptr;
    size_t     _bufferCapacity    = 0;

    ostream_type* _stream         = nullptr;
    detail::zipcrypto _zipcrypto;
    bool _encryptionHeaderWritten = false;
};

typedef basic_zipcrypto_encoder<uint8_t, std::char_traits<uint8_t>> byte_zipcrypto_encoder;
typedef basic_zipcrypto_encoder<char, std::char_traits<char>>       zipcrypto_encoder;
typedef basic_zipcrypto_encoder<wchar_t, std::char_traits<wchar_t>> wzipcrypto_encoder;
