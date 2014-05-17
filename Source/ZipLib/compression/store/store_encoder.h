#pragma once
#include "../compression_interface.h"

#include "store_properties.h"

#include "../../streams/crc32stream.h"
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
      _inputBuffer = new char_type[_bufferCapacity];
      _outputBuffer = new char_type[_bufferCapacity];
    }

    bool is_init() const override
    {
      return _stream != nullptr;
    }

    char_type* get_buffer_begin() override
    {
      return _inputBuffer;
    }

    char_type* get_buffer_end() override
    {
      return _inputBuffer + _bufferCapacity;
    }

    void encode_next(size_t length) override
    {
      _stream->write(_inputBuffer, length);
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
    char_type* _inputBuffer     = nullptr;  // pointer to the start of the input buffer
    char_type* _outputBuffer    = nullptr;  // pointer to the start of the output buffer
};
