#pragma once
#include "deflate_properties.h"
#include "../compression_interface.h"
#include "../../utils/stream/storage.h"
#include "../../extlibs/zlib/zlib.h"

#include <cstdint>

class deflate_encoder
  : public compression_encoder_interface
{
  public:
    deflate_encoder()
    {

    }

    ~deflate_encoder()
    {
      if (is_init())
      {
        deflateEnd(&_zstream);
        uninit_buffers();
      }
    }

    void init(std::ostream& stream) override
    {
      deflate_properties props;
      init(stream, props);
    }

    void init(std::ostream& stream, compression_properties_interface& props) override
    {
      // init stream
      _stream = &stream;

      // init buffers
      deflate_properties& deflateProps = static_cast<deflate_properties&>(props);
      _bufferCapacity = deflateProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer        = new uint8_t[_bufferCapacity];
      _outputBuffer       = new uint8_t[_bufferCapacity];

      // init deflate
      _zstream.zalloc     = nullptr;
      _zstream.zfree      = nullptr;
      _zstream.opaque     = nullptr;

      _zstream.next_in    = nullptr;
      _zstream.next_out   = nullptr;
      _zstream.avail_in   = 0;
      _zstream.avail_out  = 0;

      deflateInit2(&_zstream, deflateProps.CompressionLevel, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
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

    void encode_next(size_t length) override
    {
      // set the input buffer
      _zstream.next_in = reinterpret_cast<Bytef*>(_inputBuffer);
      _zstream.avail_in = static_cast<uInt>(length);

      bool flush = length < _bufferCapacity;

      // compress data
      do {
        // zstream output
        _zstream.next_out = reinterpret_cast<Bytef*>(_outputBuffer);
        _zstream.avail_out = static_cast<uInt>(_bufferCapacity);

        // compress stream
        deflate(&_zstream, flush ? Z_FINISH : Z_NO_FLUSH);

        size_t have = _bufferCapacity - static_cast<size_t>(_zstream.avail_out);

        if (have > 0)
        {
          utils::stream::store(*_stream, _outputBuffer, have);
        }
      } while (_zstream.avail_out == 0);
    }

    void sync() override
    {

    }

  private:
    void uninit_buffers()
    {
      delete[] _inputBuffer;
      delete[] _outputBuffer;
    }

    bool zlib_suceeded(int errorCode)
    {
      return ((_lastError = errorCode) >= 0);
    }

    z_stream    _zstream;                   // internal zlib structure
    int         _lastError      = Z_OK;     // last error of zlib operation

    std::ostream* _stream       = nullptr;

    size_t     _bufferCapacity  = 0;
    uint8_t*   _inputBuffer     = nullptr;  // pointer to the start of the input buffer
    uint8_t*   _outputBuffer    = nullptr;  // pointer to the start of the output buffer
};
