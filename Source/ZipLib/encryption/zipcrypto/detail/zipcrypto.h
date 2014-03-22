#pragma once
#include "zipcrypto_keys.h"
#include "zipcrypto_encryption_header.h"

namespace detail {

  class zipcrypto
  {
    public:
      zipcrypto()
      {

      }

      void init(const uint8_t* password)
      {
        do
        {
          _keys.update(uint8_t(*password++));
        } while (*password != 0);
      }

      void encrypt_header()
      {
        //assert(_finalByte != -1);
        _encryptionHeader.set_final_byte(uint8_t(_finalByte & 0xff));

        encrypt_buffer(_encryptionHeader.header.u8, sizeof(_encryptionHeader));
      }

      void decrypt_header()
      {
        decrypt_buffer(_encryptionHeader.header.u8, sizeof(_encryptionHeader));
      }

      void encrypt_buffer(uint8_t* buffer, size_t length)
      {
        for (size_t i = 0; i < length; ++i)
        {
          _keys.inplace_encrypt_byte(buffer[i]);
        }
      }

      void decrypt_buffer(uint8_t* buffer, size_t length)
      {
        for (size_t i = 0; i < length; ++i)
        {
          _keys.inplace_decrypt_byte(buffer[i]);
        }
      }

      void set_final_byte(uint8_t c)
      {
        _finalByte = int(c);
      }

      int get_final_byte() const
      {
        return _finalByte;
      }

      bool has_correct_password() const
      {
        return (uint8_t(_finalByte & 0xff) == _encryptionHeader.get_final_byte());
      }

      encryption_header_base& get_encryption_header()
      {
        return _encryptionHeader;
      }

      size_t get_encryption_header_size() const
      {
        return sizeof(_encryptionHeader);
      }

    private:
      zipcrypto_keys              _keys;
      zipcrypto_encryption_header _encryptionHeader;
      int                         _finalByte = -1;
  };

}
