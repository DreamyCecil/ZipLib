#pragma once
#include "bzip2_properties.h"
#include "../compression_interface.h"
#include "../../utils/stream/serialization.h"
#include "../../extlibs/bzip2/bzlib.h"

#include <cstdint>

class bzip2_encoder
  : public compression_encoder_interface
{
  public:
    bzip2_encoder()
    {

    }

    ~bzip2_encoder()
    {
      if (is_init())
      {
        BZ2_bzCompressEnd(&_bzstream);
        uninit_buffers();
      }
    }

    void init(std::ostream& stream) override
    {
      bzip2_properties props;
      init(stream, props);
    }

    void init(std::ostream& stream, compression_properties_interface& props) override
    {
      // init stream
      _stream = &stream;

      // init buffers
      bzip2_properties& bz2Props = static_cast<bzip2_properties&>(props);
      _bufferCapacity = bz2Props.BufferCapacity;

      uninit_buffers();
      _inputBuffer        = new uint8_t[_bufferCapacity];
      _outputBuffer       = new uint8_t[_bufferCapacity];

      // init bzip2
      _bzstream.bzalloc   = nullptr;
      _bzstream.bzfree    = nullptr;
      _bzstream.opaque    = nullptr;

      _bzstream.next_in   = nullptr;
      _bzstream.next_out  = nullptr;
      _bzstream.avail_in  = 0;
      _bzstream.avail_out = 0;

      _lastError = BZ2_bzCompressInit(&_bzstream, bz2Props.BlockSize, 0, bz2Props.WorkFactor);
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
      _bzstream.next_in = reinterpret_cast<char*>(_inputBuffer);
      _bzstream.avail_in = static_cast<unsigned int>(length);

      bool flush = length < _bufferCapacity;

      // compress data
      do {
        // zstream output
        _bzstream.next_out = reinterpret_cast<char*>(_outputBuffer);
        _bzstream.avail_out = static_cast<unsigned int>(_bufferCapacity);

        // compress stream
        BZ2_bzCompress(&_bzstream, flush ? BZ_FINISH : BZ_RUN);

        size_t have = _bufferCapacity - static_cast<size_t>(_bzstream.avail_out);

        if (have > 0)
        {
          utils::stream::serialize(*_stream, _outputBuffer, have);
        }
      } while (_bzstream.avail_out == 0);
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

    bool bzip2_suceeded(int errorCode)
    {
      return ((_lastError = errorCode) >= 0);
    }

    bz_stream _bzstream;                    // internal bzip2 structure
    int       _lastError        = BZ_OK;    // last error of bzip2 operation

    std::ostream* _stream       = nullptr;

    size_t     _bufferCapacity  = 0;
    uint8_t*   _inputBuffer     = nullptr;  // pointer to the start of the input buffer
    uint8_t*   _outputBuffer    = nullptr;  // pointer to the start of the output buffer
};
