#pragma once
#include "lzma_properties.h"
#include "detail/lzma_alloc.h"
#include "../compression_interface.h"
#include "../../utils/stream/storage.h"
#include "../../extlibs/lzma/LzmaDec.h"

#include <cstdint>

class lzma_decoder
  : public compression_decoder_interface
{
  public:
    lzma_decoder()
    {
      LzmaDec_Construct(&_handle);
    }

    ~lzma_decoder()
    {
      if (is_init())
      {
        LzmaDec_Free(&_handle, &_alloc);
        uninit_buffers();
      }
    }

    void init(std::istream& stream) override
    {
      lzma_properties props;
      init(stream, props);
    }

    void init(std::istream& stream, compression_properties_interface& props) override
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
      _inputBuffer  = new uint8_t[_bufferCapacity];
      _outputBuffer = new uint8_t[_bufferCapacity];

      // read lzma header
      Byte header[detail::lzma_header::HEADER_SIZE];
      utils::stream::load(*_stream, header);

      // init lzma
      LzmaDec_Allocate(&_handle, &header[4], LZMA_PROPS_SIZE, &_alloc);
      LzmaDec_Init(&_handle);
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
      utils::stream::load(*_stream, _inputBuffer, _bufferCapacity);

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

    std::istream* _stream         = nullptr;

    size_t     _bufferCapacity    = 0;
    size_t     _inputBufferSize   = 0;        // how many bytes are read in the input buffer
    size_t     _outputBufferSize  = 0;        // how many bytes are written in the output buffer
    uint8_t*   _inputBuffer       = nullptr;  // pointer to the start of the input buffer
    uint8_t*   _outputBuffer      = nullptr;  // pointer to the start of the output buffer
};
