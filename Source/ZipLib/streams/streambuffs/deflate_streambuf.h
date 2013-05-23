#pragma once
#include <streambuf>
#include <ostream>
#include <cstdint>

#include "streambuf_conf.h"
#include "../crc32stream.h"
#include "../../extlibs/zlib/zlib.h"

template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class deflate_streambuf :
  public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::int_type int_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;

    deflate_streambuf()
      : _outputStream(nullptr)
      , _internalInputBuffer(nullptr)
      , _internalOutputBuffer(nullptr)
    {

    }

    deflate_streambuf(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& output, int compressionLevel = Z_DEFAULT_COMPRESSION)
      : _outputStream(nullptr)
      , _internalInputBuffer(nullptr)
      , _internalOutputBuffer(nullptr)
    {
      init(output, compressionLevel);
    }

    virtual ~deflate_streambuf()
    {
      sync();
      destroy();
    }

    bool init(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& output, int compressionLevel)
    {
      _internalInputBuffer  = new ELEM_TYPE[INTERNAL_BUFFER_SIZE];
      _internalOutputBuffer = new ELEM_TYPE[INTERNAL_BUFFER_SIZE];

      _inputBufferSize = 0;

      _outputStream = &output;

      _bytesRead = 0;
      _bytesWritten = 0;

      // default memory functions
      _zstream.zalloc = nullptr;
      _zstream.zfree  = nullptr;
      _zstream.opaque = nullptr;

      _compressionLevel = compressionLevel;

      // set stream buffer
      ELEM_TYPE* endOfOutputBuffer = _internalInputBuffer + INTERNAL_BUFFER_SIZE;
      this->setp(_internalInputBuffer, endOfOutputBuffer - 1);

      return zlib_suceeded(deflateInit2(&_zstream, _compressionLevel, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY));
    }

    bool is_init() const
    {
      return (_outputStream != nullptr && _internalInputBuffer != nullptr && _internalOutputBuffer != nullptr);
    }

    size_t get_bytes_read() const
    {
      return _bytesRead;
    }

    size_t get_bytes_written() const
    {
      return _bytesWritten;
    }

  protected:
    int_type overflow(int_type c = traits_type::eof()) override
    {
      bool is_eof = traits_type::eq_int_type(c, traits_type::eof());

      // if the input is not EOF, just put it into the buffer
      // and increment current pointer
      if (!is_eof)
      {
        *this->pptr() = c;
        this->pbump(1);
      }

      // if input buffer is full or input is EOF,
      // compress the buffer
      if (this->pptr() >= this->epptr() || is_eof)
      {
        return process_buffer(is_eof);
      }

      // not eof
      return ~traits_type::eof();
    }

    int sync() override
    {
      return process_buffer(true) ? 0 : -1;
    }

  private:
    bool zlib_suceeded(int errorCode)
    {
      return ((_lastError = errorCode) >= 0);
    }

    bool destroy()
    {
      if (is_init())
      {
        if (_internalInputBuffer != nullptr)
        {
          delete[] _internalInputBuffer;
        }

        if (_internalOutputBuffer != nullptr)
        {
          delete[] _internalOutputBuffer;
        }

        return zlib_suceeded(deflateEnd(&_zstream));
      }

      return true;
    }

    bool process_buffer(bool flush)
    {
      // set the input buffer
      // TODO: deal properly with zero inputLenght
      // (and force zipcrypto)
      ptrdiff_t inputLength = this->pptr() - this->pbase();

      _zstream.next_in  = reinterpret_cast<Bytef*>(this->pbase());
      _zstream.avail_in = static_cast<uInt>(inputLength);

      // shift current position back to start
      this->pbump(static_cast<int>(-inputLength));

      _bytesRead += inputLength;

      // compress data
      return compress_buffer(flush);
    }

    bool compress_buffer(bool flush)
    {
      bool result = true;

      do {
        // zstream output
        _zstream.next_out  = reinterpret_cast<Bytef*>(_internalOutputBuffer);
        _zstream.avail_out = static_cast<uInt>(INTERNAL_BUFFER_REAL_SIZE);

        // compress stream
        result = zlib_suceeded(deflate(&_zstream, flush ? Z_FINISH : Z_NO_FLUSH));

        size_t have = INTERNAL_BUFFER_REAL_SIZE - static_cast<size_t>(_zstream.avail_out);

        if (have > 0)
        {
          _outputStream->write(_internalOutputBuffer, have);
          _bytesWritten += have;
        }
      } while (_zstream.avail_out == 0);

      return result;
    }

    //////////////////////////////////////////////////////////////////////////

    enum : size_t
    {
      INTERNAL_BUFFER_SIZE      = STREAM_DEFAULT_BUFFER_SIZE,
      INTERNAL_ELEMENT_SIZE     = sizeof(ELEM_TYPE),
      INTERNAL_BUFFER_REAL_SIZE = INTERNAL_BUFFER_SIZE * INTERNAL_ELEMENT_SIZE
    };

    //////////////////////////////////////////////////////////////////////////

    std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>* _outputStream;

    z_stream    _zstream;                 // internal zlib structure
    int         _lastError;               // last error of zlib operation
    int         _compressionLevel;        // compression level

    ELEM_TYPE*  _internalInputBuffer;     // pointer to the start of the input buffer
    ELEM_TYPE*  _internalOutputBuffer;    // pointer to the start of the output buffer

    size_t      _inputBufferSize;         // how many bytes are read in the input buffer
    size_t      _bytesRead;
    size_t      _bytesWritten;
};
