#pragma once
#include "aes_types.h"
#include "aes_salt.h"
#include "../common.h"
#include "../../../extlibs/aes/fileenc.h"
#include "../../../extlibs/aes/prng.h"
#include "../../../extlibs/aes/entropy.h"

#include <array>
#include <cstring>
#include <cstdint>
#include <cassert>

namespace detail {

  class aes_impl
  {
    public:
      // constructor
      aes_impl();
      aes_impl(const uint8_t* password, const aes_salt& salt, aes_passverify& passverify);

      // init
      aes_passverify init(const uint8_t* password, const aes_salt& salt);

      // encryption/decryption
      void encrypt(uint8_t* data, size_t size);
      void decrypt(uint8_t* data, size_t size);
      aes_authcode finish();

    private:
      fcrypt_ctx _ctx;
      prng_ctx _rng[1];

      bool _isInitialized;
  };

}
