#pragma once
#include "../compression_interface.h"

#include "store_decoder_properties.h"

#include "../../streams/crc32stream.h"
#include "../../extlibs/zlib/zlib.h"

#include <cstdint>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_store_decoder
  : public compression_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename compression_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename compression_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    basic_store_decoder()
      : _stream(nullptr)
      , _bufferCapacity(0)
      , _outputBufferSize(0)
      , _outputBuffer(nullptr)
    {

    }

    ~basic_store_decoder()
    {
      if (is_init())
      {
        uninit_buffers();
      }
    }

    void init(istream_type& stream) override
    {
      store_decoder_properties props;
      init(stream, props);
    }

    void init(istream_type& stream, compression_decoder_properties_interface& props) override
    {
      // init stream
      _stream = &stream;

      // init values
      _outputBufferSize = 0;

      // init buffers
      store_decoder_properties& storeProps = static_cast<store_decoder_properties&>(props);
      _bufferCapacity = storeProps.BufferCapacity;

      uninit_buffers();
      _outputBuffer = new ELEM_TYPE[_bufferCapacity];
    }

    bool is_init() const override
    {
      return (_outputBuffer != nullptr);
    }

    ELEM_TYPE* get_buffer_begin() override
    {
      return _outputBuffer;
    }

    ELEM_TYPE* get_buffer_end() override
    {
      return _outputBuffer + _outputBufferSize;
    }

    size_t decode_next() override
    {
      // read next bytes from input stream
      _stream->read(_outputBuffer, _bufferCapacity);

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

    istream_type* _stream;

    size_t     _bufferCapacity;
    size_t     _outputBufferSize; // how many bytes are written in the output buffer
    ELEM_TYPE* _outputBuffer;     // pointer to the start of the output buffer
};

typedef basic_store_decoder<uint8_t, std::char_traits<uint8_t>>  byte_store_decoder;
typedef basic_store_decoder<char, std::char_traits<char>>        store_decoder;
typedef basic_store_decoder<wchar_t, std::char_traits<wchar_t>>  wstore_decoder;
