#include "aes_impl.h"
#include <cassert>

namespace detail {

  aes_impl::aes_impl()
    : _isInitialized(false)
  {

  }

  aes_impl::aes_impl(const uint8_t* password, const aes_salt& salt, aes_passverify& passverify)
    : _isInitialized(false)
  {
    passverify = init(password, salt);
  }

  aes_passverify aes_impl::init(const uint8_t* password, const aes_salt& salt)
  {
    aes_passverify passverify;

    fcrypt_init(
      static_cast<int>(salt.get_encryption_mode()),
      static_cast<const unsigned char*>(password),
      static_cast<unsigned int>(strlen(reinterpret_cast<const char*>(password))),
      static_cast<const unsigned char*>(salt.data()),
      static_cast<unsigned char*>(passverify.data()),
      &_ctx);

    _isInitialized = true;

    return passverify;
  }

  void aes_impl::encrypt(uint8_t* data, size_t size)
  {
    assert(_isInitialized);

    fcrypt_encrypt(
      static_cast<unsigned char*>(data),
      static_cast<unsigned int>(size),
      &_ctx);
  }

  void aes_impl::decrypt(uint8_t* data, size_t size)
  {
    assert(_isInitialized);

    fcrypt_decrypt(
      static_cast<unsigned char*>(data),
      static_cast<unsigned int>(size),
      &_ctx);
  }

  aes_authcode aes_impl::finish()
  {
    assert(_isInitialized);

    aes_authcode result;

    fcrypt_end(
      static_cast<unsigned char*>(result.data()),
      &_ctx);

    return result;
  }

}
