#pragma once
#include "../encryption_interface.h"
#include "zipcrypto_decoder_properties.h"
#include "../../extlibs/zlib/zconf.h"
#include "detail/zipcrypto.h"

#include <cstdint>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_zipcrypto_decoder
  : public encryption_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    basic_zipcrypto_decoder()
      : _inputBuffer(nullptr)
      , _stream(nullptr)
      , _encryptionHeaderRead(false)
    {

    }

    ~basic_zipcrypto_decoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(istream_type& stream) override
    {
      zipcrypto_decoder_properties props;
      init(stream, props);
    }

    void init(istream_type& stream, encryption_decoder_properties_interface& props) override
    {
      // init buffers
      zipcrypto_decoder_properties& zipcryptoProps = static_cast<zipcrypto_decoder_properties&>(props);
      _bufferCapacity = zipcryptoProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new ELEM_TYPE[_bufferCapacity];

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

    ELEM_TYPE* get_buffer_begin() override
    {
      return _inputBuffer;
    }

    ELEM_TYPE* get_buffer_end() override
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
        reinterpret_cast<ELEM_TYPE*>(&_zipcrypto.get_encryption_header()),
        _zipcrypto.get_encryption_header_size());

      _zipcrypto.decrypt_header();
      _encryptionHeaderRead = true;
    }

    void uninit_buffers()
    {
      if (_inputBuffer != nullptr)
      {
        delete[] _inputBuffer;
      }
    }

    ELEM_TYPE* _inputBuffer;
    size_t     _bufferCapacity;

    istream_type* _stream;
    detail::zipcrypto _zipcrypto;
    bool _encryptionHeaderRead;
};

typedef basic_zipcrypto_decoder<uint8_t, std::char_traits<uint8_t>> byte_zipcrypto_decoder;
typedef basic_zipcrypto_decoder<char, std::char_traits<char>>       zipcrypto_decoder;
typedef basic_zipcrypto_decoder<wchar_t, std::char_traits<wchar_t>> wzipcrypto_decoder;
