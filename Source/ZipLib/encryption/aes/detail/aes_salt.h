#pragma once
#include "aes_types.h"
#include "../common.h"
#include "../../../extlibs/aes/fileenc.h"
#include "../../../extlibs/aes/prng.h"
#include "../../../extlibs/aes/entropy.h"

#include <cstdint>
#include <cassert>

namespace detail {

  class aes_salt
  {
    public:
      typedef uint8_t         value_type;
      typedef size_t          size_type;
      typedef ptrdiff_t       difference_type;
      typedef uint8_t*        pointer;
      typedef const uint8_t*  const_pointer;
      typedef uint8_t&        reference;
      typedef const uint8_t&  const_reference;

      aes_salt()
        : _saltLength(0)
        , _encryptionMode(aes_encryption_mode(-1))
      {
        memset(_salt, 0, sizeof(_salt));
      }

      aes_salt(aes_encryption_mode mode)
        : _saltLength(SALT_LENGTH(uint8_t(mode)))
        , _encryptionMode(mode)
      {
        assert(uint8_t(mode) >= 1 && uint8_t(mode) <= 3);

        memset(_salt, 0, sizeof(_salt));
      }

      aes_salt(const aes_salt& other) = default;
      aes_salt& operator = (const aes_salt&) = default;

      void init(aes_encryption_mode mode)
      {
        _saltLength = SALT_LENGTH(uint8_t(mode));
        _encryptionMode = mode;
        memset(_salt, 0, sizeof(_salt));
      }

      const value_type* data() const { return _salt; }
      value_type* data() { return _salt; }

      const value_type& at(size_type index) const { return _salt[index]; }
      value_type& at(size_type index) { return _salt[index]; }

      const value_type& operator[](size_type index) const { return at(index); }
      value_type& operator[](size_type index) { return at(index); }

      size_type size() const { return _saltLength; }

      aes_encryption_mode get_encryption_mode() const { return _encryptionMode; }

      static aes_salt generate(aes_encryption_mode mode)
      {
        aes_salt result(mode);

        prng_ctx rng[1];
        prng_init(entropy_fun, rng);
        prng_rand(
          static_cast<unsigned char*>(result._salt),
          static_cast<unsigned int>(result._saltLength),
          rng);
        prng_end(rng);

        return result;
      }

    private:
      value_type  _salt[AES_MAX_SALT_LENGTH];
      size_type   _saltLength;

      aes_encryption_mode _encryptionMode;
  };

}