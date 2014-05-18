#pragma once
#include "bzip2_properties.h"
#include "../compression_interface.h"
#include "../../utils/stream/storage.h"
#include "../../extlibs/bzip2/bzlib.h"

#include <cstdint>

class bzip2_decoder
  : public compression_decoder_interface
{
  public:
    bzip2_decoder()
    {

    }

    ~bzip2_decoder()
    {
      if (is_init())
      {
        BZ2_bzDecompressEnd(&_bzstream);
        uninit_buffers();
      }
    }

    void init(std::istream& stream) override
    {
      bzip2_properties props;
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
      bzip2_properties& bzip2Props = static_cast<bzip2_properties&>(props);
      _bufferCapacity = bzip2Props.BufferCapacity;

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
      _bzstream.avail_out = (unsigned int)-1; // force first load of data

      // no verbosity & do not use small memory model
      _lastError = BZ2_bzDecompressInit(&_bzstream, 0, 0);
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
      if (_bzstream.avail_out != 0)
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
        _bzstream.next_in = reinterpret_cast<char*>(_inputBuffer);
        _bzstream.avail_in = static_cast<unsigned int>(_inputBufferSize);
      }

      // zstream output
      _bzstream.next_out = reinterpret_cast<char*>(_outputBuffer);
      _bzstream.avail_out = static_cast<unsigned int>(_bufferCapacity);

      // inflate stream
      if (!bzip2_suceeded(BZ2_bzDecompress(&_bzstream)))
      {
        return 0;
      }

      // associate output buffer
      size_t bytesProcessed = _bufferCapacity - static_cast<size_t>(_bzstream.avail_out);

      if (_lastError == BZ_STREAM_END)
      {
        _endOfStream = true;

        // if we read more than we should last time, move pointer to the correct position
        if (_bzstream.avail_in > 0)
        {
          _stream->clear();
          _stream->seekg(-static_cast<std::istream::off_type>(_bzstream.avail_in), std::ios::cur);
        }
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

    bool bzip2_suceeded(int errorCode)
    {
      return ((_lastError = errorCode) >= 0);
    }

    bz_stream   _bzstream;                    // internal bzip2 structure
    int         _lastError        = BZ_OK;    // last error of bzip2 operation

    std::istream* _stream         = nullptr;
    bool       _endOfStream       = false;

    size_t     _bufferCapacity    = 0;
    size_t     _inputBufferSize   = 0;        // how many bytes are read in the input buffer
    size_t     _outputBufferSize  = 0;        // how many bytes are written in the output buffer
    uint8_t*   _inputBuffer       = nullptr;  // pointer to the start of the input buffer
    uint8_t*   _outputBuffer      = nullptr;  // pointer to the start of the output buffer
};
