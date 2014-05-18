#pragma once
#include "aes_properties.h"
#include "detail/aes_impl.h"
#include "../encryption_interface.h"
#include "../../utils/stream/storage.h"

#include <cstdint>

class aes_decoder
  : public encryption_decoder_interface
{
  public:
    aes_decoder()
    {

    }

    ~aes_decoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(std::istream& stream) override
    {
      aes_properties props;
      init(stream, props);
    }

    void init(std::istream& stream, encryption_properties_interface& props) override
    {
      // init buffers
      aes_properties& aesProps = static_cast<aes_properties&>(props);
      _bufferCapacity = aesProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new uint8_t[_bufferCapacity];

      // init stream
      _stream = &stream;
      init_stream();

      // init aes
      _encryptionHeaderRead = false;
      _encryptionFooterRead = false;
      _aesStoredContext.salt.init(aesProps.EncryptionMode);

      read_encryption_header();
      read_encryption_footer();

      const uint8_t* passwordData = reinterpret_cast<const uint8_t*>(aesProps.Password.c_str());
      _aesContext.passverify = _aes.init(passwordData, _aesStoredContext.salt);

      // check if stream has not set failbit
    }

    bool is_init() const override
    {
      return _stream != nullptr && _encryptedStream != nullptr && _authCodeStream != nullptr;
    }

    uint8_t* get_buffer_begin() override
    {
      return _inputBuffer;
    }

    uint8_t* get_buffer_end() override
    {
      return _inputBuffer + _bufferCapacity;
    }

    size_t decrypt_next() override
    {
      utils::stream::load(*_encryptedStream, _inputBuffer, _bufferCapacity);

      size_t n = static_cast<size_t>(_encryptedStream->gcount());
      _aes.decrypt(_inputBuffer, n);

      _aesContext.authcode = _aes.finish();

      return n;
    }

    bool has_correct_password() const override
    {
      return _aesContext.passverify == _aesStoredContext.passverify;
    }

    bool has_matching_authcode() const
    {
      return _aesContext.authcode == _aesStoredContext.authcode;
    }

  private:
    void init_stream()
    {
      // get size of stream
      auto curPos = _stream->tellg();

      _stream->seekg(0, std::ios::end);
      auto streamSize = _stream->tellg() - curPos;

      _stream->seekg(curPos);

      if (streamSize < _aesStoredContext.salt.size() + _aesStoredContext.passverify.size() + _aesStoredContext.authcode.size())
      {
        return;
      }

      _encryptedStream = std::unique_ptr<isubstream>(new isubstream(*_stream, 0, static_cast<size_t>(streamSize) - _aesStoredContext.authcode.size()));
      _authCodeStream = std::unique_ptr<isubstream>(new isubstream(*_stream, static_cast<size_t>(streamSize) - _aesStoredContext.authcode.size()));
    }

    void read_encryption_header()
    {
      utils::stream::load(*_encryptedStream, _aesStoredContext.salt.data(), _aesStoredContext.salt.size());
      utils::stream::load(*_encryptedStream, _aesStoredContext.passverify);

      _encryptionHeaderRead = true;
    }

    void read_encryption_footer()
    {
      utils::stream::load(*_authCodeStream, _aesStoredContext.authcode);

      _encryptionFooterRead = true;
    }

    void uninit_buffers()
    {
      delete[] _inputBuffer;
    }

    uint8_t*    _inputBuffer    = nullptr;
    size_t     _bufferCapacity  = 0;

    std::istream* _stream       = nullptr;
    std::unique_ptr<isubstream> _encryptedStream;
    std::unique_ptr<isubstream> _authCodeStream;

    struct aes_context
    {
      detail::aes_passverify  passverify;
      detail::aes_authcode    authcode;
    };

    struct aes_stored_context
    {
      detail::aes_salt        salt;
      detail::aes_passverify  passverify;
      detail::aes_authcode    authcode;
    };

    detail::aes_impl    _aes;
    aes_context         _aesContext;
    aes_stored_context  _aesStoredContext;

    bool _encryptionHeaderRead  = false;
    bool _encryptionFooterRead  = false;
};
