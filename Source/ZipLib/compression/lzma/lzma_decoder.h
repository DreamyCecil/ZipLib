#pragma once
#include "../compression_interface.h"

#include "detail/lzma_alloc.h"
#include "lzma_properties.h"

#include "../../extlibs/lzma/LzmaDec.h"

#include <cstdint>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class basic_lzma_decoder
  : public compression_decoder_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename compression_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename compression_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    basic_lzma_decoder()
    {
      LzmaDec_Construct(&_handle);
    }

    ~basic_lzma_decoder()
    {
      if (is_init())
      {
        LzmaDec_Free(&_handle, &_alloc);
        uninit_buffers();
      }
    }

    void init(istream_type& stream) override
    {
      lzma_properties props;
      init(stream, props);
    }

    void init(istream_type& stream, compression_properties_interface& props) override
    {
      // init stream
      _stream = &stream;

      // init values
      _inPos = _inProcessed = _outProcessed = 0;
      _inputBufferSize = 0;

      // init buffers
      lzma_properties& lzmaProps = static_cast<lzma_properties&>(props);
      _bufferCapacity = lzmaProps.BufferCapacity;

      uninit_buffers();
      _inputBuffer = new ELEM_TYPE[_bufferCapacity];
      _outputBuffer = new ELEM_TYPE[_bufferCapacity];

      // read lzma header
      Byte header[LZMA_PROPS_SIZE + 4];
      _stream->read(reinterpret_cast<ELEM_TYPE*>(header), sizeof(header) / sizeof(ELEM_TYPE));

      // init lzma
      LzmaDec_Allocate(&_handle, &header[4], LZMA_PROPS_SIZE, &_alloc);
      LzmaDec_Init(&_handle);
    }

    bool is_init() const override
    {
      return (_inputBuffer != nullptr && _outputBuffer != nullptr);
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
      if (_inPos == _inputBufferSize)
      {
        read_next();
      }

      _inProcessed = _inputBufferSize - _inPos;
      _outProcessed = _bufferCapacity;

      ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
      ELzmaStatus status;
      SRes res;
      res = LzmaDec_DecodeToBuf(
        &_handle,
        reinterpret_cast<Byte*>(_outputBuffer),
        &_outProcessed,
        reinterpret_cast<Byte*>(_inputBuffer) + _inPos,
        &_inProcessed,
        finishMode,
        &status);

      _inPos += _inProcessed;

      _outputBufferSize = _outProcessed;

      return _outputBufferSize;

      // if (res != SZ_OK)
      //   return _lzmaInProcessed;
      // 
      // if (_lzmaInProcessed == 0 && _lzmaOutProcessed == 0)
      // {
      //   if (status != LZMA_STATUS_FINISHED_WITH_MARK)
      //     return _lzmaOutProcessed;
      //   return _lzmaOutProcessed;
      // }
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
      _stream->read(_inputBuffer, _bufferCapacity);

      // set the size of buffer
      _inputBufferSize = static_cast<size_t>(_stream->gcount());

      // set lzma buffer pointer to the begin
      _inPos = 0;
    }

    CLzmaDec _handle;
    detail::lzma_alloc _alloc;

    SizeT _inPos                  = 0;
    SizeT _inProcessed            = 0;
    SizeT _outProcessed           = 0;

    istream_type* _stream         = nullptr;

    size_t     _bufferCapacity    = 0;
    size_t     _inputBufferSize   = 0;        // how many bytes are read in the input buffer
    size_t     _outputBufferSize  = 0;        // how many bytes are written in the output buffer
    ELEM_TYPE* _inputBuffer       = nullptr;  // pointer to the start of the input buffer
    ELEM_TYPE* _outputBuffer      = nullptr;  // pointer to the start of the output buffer
};

typedef basic_lzma_decoder<uint8_t, std::char_traits<uint8_t>>  byte_lzma_decoder;
typedef basic_lzma_decoder<char, std::char_traits<char>>        lzma_decoder;
typedef basic_lzma_decoder<wchar_t, std::char_traits<wchar_t>>  wlzma_decoder;
