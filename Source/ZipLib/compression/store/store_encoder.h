#pragma once
#include "../compression_interface.h"

#include "store_properties.h"

#include "../../streams/crc32stream.h"
#include "../../extlibs/zlib/zlib.h"

#include <cstdint>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_store_encoder
  : public compression_encoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename compression_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename compression_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    basic_store_encoder()
    {

    }

    ~basic_store_encoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(ostream_type& stream) override
    {
      store_properties props;
      init(stream, props);
    }

    void init(ostream_type& stream, compression_properties_interface& props) override
    {
      // init stream
      _stream = &stream;

      // init buffers
      store_properties& storeProps = static_cast<store_properties&>(props);
      _bufferCapacity = storeProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new ELEM_TYPE[_bufferCapacity];
      _outputBuffer = new ELEM_TYPE[_bufferCapacity];
    }

    bool is_init() const override
    {
      return _stream != nullptr;
    }

    ELEM_TYPE* get_buffer_begin() override
    {
      return _inputBuffer;
    }

    ELEM_TYPE* get_buffer_end() override
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

    ostream_type* _stream       = nullptr;

    size_t     _bufferCapacity  = 0;
    ELEM_TYPE* _inputBuffer     = nullptr;  // pointer to the start of the input buffer
    ELEM_TYPE* _outputBuffer    = nullptr;  // pointer to the start of the output buffer
};

typedef basic_store_encoder<uint8_t, std::char_traits<uint8_t>>  byte_store_encoder;
typedef basic_store_encoder<char, std::char_traits<char>>        store_encoder;
typedef basic_store_encoder<wchar_t, std::char_traits<wchar_t>>  wstore_encoder;
