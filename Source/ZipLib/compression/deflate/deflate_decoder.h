#pragma once
#include "deflate_properties.h"
#include "../compression_interface.h"
#include "../../utils/stream/storage.h"
#include "../../extlibs/zlib/zlib.h"  

#include <cstdint>

class deflate_decoder
  : public compression_decoder_interface
{
  public:
    deflate_decoder()
    {

    }

    ~deflate_decoder()
    {
      if (is_init())
      {
        inflateEnd(&_zstream);
        uninit_buffers();
      }
    }

    void init(std::istream& stream) override
    {
      deflate_properties props;
      init(stream, props);
    }

    void init(std::istream& stream, compression_properties_interface& props) override
    {
      // init stream
      _stream = &stream;
      _endOfStream = false;

      // init values
      _inputBufferSize = _outputBufferSize = 0;

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
      _zstream.avail_out  = uInt(-1); // force first load of data

      inflateInit2(&_zstream, -MAX_WBITS);
    }

    bool is_init() const override
    {
      return (_inputBuffer != nullptr && _outputBuffer != nullptr);
    }

    uint8_t* get_buffer_begin() override
    {
      return _outputBuffer;
    }

    uint8_t* get_buffer_end() override
    {
      return _outputBuffer + _outputBufferSize;
    }

    size_t decode_next() override
    {
      // do not load any data until there
      // are something left
      if (_zstream.avail_out != 0)
      {
        // if all data has not been fetched and the stream is at the end,
        // it is an error
        if (_endOfStream)
        {
          return 0;
        }

        // read data into buffer
        read_next();

        // set input buffer and its size
        _zstream.next_in = reinterpret_cast<Bytef*>(_inputBuffer);
        _zstream.avail_in = static_cast<uInt>(_inputBufferSize);
      }

      // zstream output
      _zstream.next_out = reinterpret_cast<Bytef*>(_outputBuffer);
      _zstream.avail_out = static_cast<uInt>(_bufferCapacity);

      // inflate stream
      if (!zlib_suceeded(inflate(&_zstream, Z_NO_FLUSH)))
      {
        return 0;
      }

      // associate output buffer
      size_t bytesProcessed = _bufferCapacity - static_cast<size_t>(_zstream.avail_out);

      if (_lastError == Z_STREAM_END)
      {
        _endOfStream = true;

        // we read more than we should
        assert(_zstream.avail_in == 0);
      }

      _outputBufferSize = bytesProcessed;

      // return count of processed bytes from input stream
      return bytesProcessed;
    }

  private:
    void uninit_buffers()
    {
      delete[] _inputBuffer;
      delete[] _outputBuffer;
    }

    void read_next()
    {
      // read next bytes from input stream
      utils::stream::load(*_stream, _inputBuffer, _bufferCapacity);

      // set the size of buffer
      _inputBufferSize = static_cast<size_t>(_stream->gcount());

      // set lzma buffer pointer to the begin
      _endOfStream = _inputBufferSize != _bufferCapacity;
    }

    bool zlib_suceeded(int errorCode)
    {
      return ((_lastError = errorCode) >= 0);
    }

    z_stream    _zstream;                     // internal zlib structure
    int         _lastError        = Z_OK;     // last error of zlib operation

    std::istream* _stream         = nullptr;
    bool       _endOfStream       = false;

    size_t     _bufferCapacity    = 0;
    size_t     _inputBufferSize   = 0;        // how many bytes are read in the input buffer
    size_t     _outputBufferSize  = 0;        // how many bytes are written in the output buffer
    uint8_t*   _inputBuffer       = nullptr;  // pointer to the start of the input buffer
    uint8_t*   _outputBuffer      = nullptr;  // pointer to the start of the output buffer
};
