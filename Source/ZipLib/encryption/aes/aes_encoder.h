#pragma once
#include "aes_properties.h"
#include "detail/aes_impl.h"
#include "../encryption_interface.h"
#include "../../utils/stream/serialization.h"

#include <cstdint>

class aes_encoder
  : public encryption_encoder_interface
{
  public:
    aes_encoder()
    {

    }

    ~aes_encoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(std::ostream& stream) override
    {
      aes_properties props;
      init(stream, props);
    }

    void init(std::ostream& stream, encryption_properties_interface& props) override
    {
      // init buffers
      aes_properties& aesProps = static_cast<aes_properties&>(props);
      _bufferCapacity = aesProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new uint8_t[_bufferCapacity];

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

    uint8_t* get_buffer_begin() override
    {
      return _inputBuffer;
    }

    uint8_t* get_buffer_end() override
    {
      return _inputBuffer + _bufferCapacity;
    }

    void encrypt_next(size_t length) override
    {
      if (!_encryptionHeaderWritten)
      {
        write_encryption_header();
      }

      _aes.encrypt(_inputBuffer, length);
      utils::stream::serialize(*_stream, _inputBuffer, length);
    }

    void sync() override
    {
      utils::stream::serialize(*_stream, _aes.finish());
      _stream->rdbuf()->pubsync();
    }

  private:
    void write_encryption_header()
    {
      utils::stream::serialize(*_stream, _aesContext.salt.data(), _aesContext.salt.size());
      utils::stream::serialize(*_stream, _aesContext.passverify);

      _encryptionHeaderWritten = true;
    }

    void uninit_buffers()
    {
      delete[] _inputBuffer;
    }

    uint8_t*   _inputBuffer = nullptr;
    size_t     _bufferCapacity = 0;

    std::ostream* _stream = nullptr;

    struct aes_context
    {
      detail::aes_salt        salt;
      detail::aes_passverify  passverify;
    };

    detail::aes_impl _aes;
    aes_context _aesContext;

    bool _encryptionHeaderWritten = false;
};
