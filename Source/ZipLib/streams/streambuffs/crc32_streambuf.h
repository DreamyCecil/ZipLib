#pragma once
#include <streambuf>
#include <cstdint>

#include "../substream.h"
#include "../../extlibs/zlib/zlib.h"

class crc32_streambuf
  : public std::streambuf
{
  public:
    crc32_streambuf()
    {

    }

    crc32_streambuf(std::istream& input)
    {
      init(input);
    }

    void init(std::istream& input)
    {
      _inputStream = &input;

      char_type* endOfBuffer = _internalBuffer + INTERNAL_BUFFER_SIZE;
      this->setg(endOfBuffer, endOfBuffer, endOfBuffer);
    }

    bool is_init() const
    {
      return (_inputStream != nullptr);
    }

    uint32_t get_crc32() const
    {
      return _crc32;
    }

  protected:
    int_type underflow() override
    {
      // buffer exhausted
      if (this->gptr() >= _internalBufferEnd)
      {
        _inputStream->read(_internalBuffer, static_cast<std::streamsize>(INTERNAL_BUFFER_SIZE));
        size_t n = static_cast<size_t>(_inputStream->gcount());

        _internalBufferPosition = _internalBuffer;
        _internalBufferEnd = _internalBuffer + n;

        if (n == 0)
        {
          return traits_type::eof();
        }
      }

      // set buffer pointers, increment current position
      char_type* base = _internalBufferPosition++;

      // setting all pointers to the same position forces calling of this method each time,
      // so crc32 really represents the checksum of what really has been read
      this->setg(base, base, base + 1);

      _crc32 = crc32(_crc32, reinterpret_cast<Bytef*>(this->gptr()), static_cast<uInt>(sizeof(char_type)));

      return traits_type::to_int_type(*this->gptr());
    }
    
  private:
    enum : size_t
    {
      INTERNAL_BUFFER_SIZE = 1 << 15
    };

    char_type     _internalBuffer[INTERNAL_BUFFER_SIZE];
    char_type*    _internalBufferPosition = _internalBuffer + INTERNAL_BUFFER_SIZE;
    char_type*    _internalBufferEnd      = _internalBuffer + INTERNAL_BUFFER_SIZE;

    std::istream* _inputStream = nullptr;
    uint32_t _crc32 = 0;
};
