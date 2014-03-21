#pragma once
#include "../encryption_interface.h"
#include "aes_decoder_properties.h"
#include "detail/aes_impl.h"

#include <cstdint>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_aes_decoder
  : public encryption_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    basic_aes_decoder()
      : _inputBuffer(nullptr)
      , _stream(nullptr)
      , _encryptionHeaderRead(false)
      , _encryptionFooterRead(false)
    {

    }

    ~basic_aes_decoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(istream_type& stream) override
    {
      aes_decoder_properties props;
      init(stream, props);
    }

    void init(istream_type& stream, encryption_decoder_properties_interface& props) override
    {
      // init buffers
      aes_decoder_properties& aesProps = static_cast<aes_decoder_properties&>(props);
      _bufferCapacity = aesProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new ELEM_TYPE[_bufferCapacity];

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
      _encryptedStream->read(_inputBuffer, _bufferCapacity);

      size_t n = static_cast<size_t>(_encryptedStream->gcount());
      _aes.decrypt(reinterpret_cast<uint8_t*>(_inputBuffer), n);

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
      deserialize(*_encryptedStream, _aesStoredContext.salt.data(), _aesStoredContext.salt.size());
      deserialize(*_encryptedStream, _aesStoredContext.passverify);

      _encryptionHeaderRead = true;
    }

    void read_encryption_footer()
    {
      deserialize(*_authCodeStream, _aesStoredContext.authcode);

      _encryptionFooterRead = true;
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

    bool _encryptionHeaderRead;
    bool _encryptionFooterRead;
};

typedef basic_aes_decoder<uint8_t, std::char_traits<uint8_t>> byte_aes_decoder;
typedef basic_aes_decoder<char, std::char_traits<char>>       aes_decoder;
typedef basic_aes_decoder<wchar_t, std::char_traits<wchar_t>> waes_decoder;
