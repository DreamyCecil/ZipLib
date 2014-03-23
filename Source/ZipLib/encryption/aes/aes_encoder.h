#pragma once
#include "../encryption_interface.h"
#include "../../streams/serialization.h"
#include "aes_properties.h"
#include "detail/aes_impl.h"

#include <cstdint>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_aes_encoder
  : public encryption_encoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    basic_aes_encoder()
    {

    }

    ~basic_aes_encoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(ostream_type& stream) override
    {
      aes_properties props;
      init(stream, props);
    }

    void init(ostream_type& stream, encryption_properties_interface& props) override
    {
      // init buffers
      aes_properties& aesProps = static_cast<aes_properties&>(props);
      _bufferCapacity = aesProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new ELEM_TYPE[_bufferCapacity];

      // init stream
      _stream = &stream;

      // init aes
      _encryptionHeaderWritten = false;
      const uint8_t* passwordData = reinterpret_cast<const uint8_t*>(aesProps.Password.c_str());

      _aesContext.salt = detail::aes_salt::generate(aesProps.EncryptionMode);
      _aesContext.passverify = _aes.init(passwordData, _aesContext.salt);
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

      _aes.encrypt(reinterpret_cast<uint8_t*>(_inputBuffer), length);
      _stream->write(_inputBuffer, length);
    }

    void sync() override
    {
      serialize(*_stream, _aes.finish());
      _stream->rdbuf()->pubsync();
    }

  private:
    void write_encryption_header()
    {
      serialize(*_stream, _aesContext.salt.data(), _aesContext.salt.size());
      serialize(*_stream, _aesContext.passverify);

      _encryptionHeaderWritten = true;
    }

    void uninit_buffers()
    {
      delete[] _inputBuffer;
    }

    ELEM_TYPE* _inputBuffer = nullptr;
    size_t     _bufferCapacity = 0;

    ostream_type* _stream = nullptr;

    struct aes_context
    {
      detail::aes_salt        salt;
      detail::aes_passverify  passverify;
    };

    detail::aes_impl _aes;
    aes_context _aesContext;

    bool _encryptionHeaderWritten = false;
};

typedef basic_aes_encoder<uint8_t, std::char_traits<uint8_t>> byte_aes_encoder;
typedef basic_aes_encoder<char, std::char_traits<char>>       aes_encoder;
typedef basic_aes_encoder<wchar_t, std::char_traits<wchar_t>> waes_encoder;
