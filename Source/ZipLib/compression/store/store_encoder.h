#pragma once
#include "store_properties.h"
#include "../compression_interface.h"
#include "../../streams/crc32stream.h"
#include "../../utils/stream/serialization.h"
#include "../../extlibs/zlib/zlib.h"

#include <cstdint>

class store_encoder
  : public compression_encoder_interface
{
  public:
    store_encoder()
    {

    }

    ~store_encoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(std::ostream& stream) override
    {
      store_properties props;
      init(stream, props);
    }

    void init(std::ostream& stream, compression_properties_interface& props) override
    {
      // init stream
      _stream = &stream;

      // init buffers
      store_properties& storeProps = static_cast<store_properties&>(props);
      _bufferCapacity = storeProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer  = new uint8_t[_bufferCapacity];
      _outputBuffer = new uint8_t[_bufferCapacity];
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
      utils::stream::serialize(*_stream, _inputBuffer, length);
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

    std::ostream* _stream       = nullptr;

    size_t     _bufferCapacity  = 0;
    uint8_t*   _inputBuffer     = nullptr;  // pointer to the start of the input buffer
    uint8_t*   _outputBuffer    = nullptr;  // pointer to the start of the output buffer
};
