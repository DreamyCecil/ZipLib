#pragma once
#include "store_properties.h"
#include "../compression_interface.h"
#include "../../streams/crc32stream.h"
#include "../../utils/stream/storage.h"
#include "../../extlibs/zlib/zlib.h"

#include <cstdint>

class store_decoder
  : public compression_decoder_interface
{
  public:
    store_decoder()
    {

    }

    ~store_decoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(std::istream& stream) override
    {
      store_properties props;
      init(stream, props);
    }

    void init(std::istream& stream, compression_properties_interface& props) override
    {
      // init stream
      _stream = &stream;

      // init values
      _outputBufferSize = 0;

      // init buffers
      store_properties& storeProps = static_cast<store_properties&>(props);
      _bufferCapacity = storeProps.BufferCapacity;

      uninit_buffers();
      _outputBuffer = new uint8_t[_bufferCapacity];
    }

    bool is_init() const override
    {
      return (_outputBuffer != nullptr);
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
      // read next bytes from input stream
      utils::stream::load(*_stream, _outputBuffer, _bufferCapacity);

      // set the size of buffer
      _outputBufferSize = static_cast<size_t>(_stream->gcount());

      // return count of processed bytes from input stream
      return _outputBufferSize;
    }

  private:
    void uninit_buffers()
    {
      delete[] _outputBuffer;
    }

    std::istream* _stream         = nullptr;

    size_t     _bufferCapacity    = 0;
    size_t     _outputBufferSize  = 0;        // how many bytes are written in the output buffer
    uint8_t*   _outputBuffer      = nullptr;  // pointer to the start of the output buffer
};
