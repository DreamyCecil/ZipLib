#pragma once
#include <streambuf>
#include <istream>
#include <cstdint>

#include "streambuf_conf.h"
#include "../crc32stream.h"
#include "../../extlibs/zlib/zlib.h"

template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class inflate_streambuf :
  public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::int_type int_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;
    
    inflate_streambuf()
      : _inputStream(nullptr)
      , _internalInputBuffer(nullptr)
      , _internalOutputBuffer(nullptr)
    {

    }

    inflate_streambuf(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& input)
      : _inputStream(nullptr)
      , _internalInputBuffer(nullptr)
      , _internalOutputBuffer(nullptr)
    {
      init(input);
    }

    virtual ~inflate_streambuf()
    {
      destroy();
    }

    bool init(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& input)
    {
      _internalInputBuffer  = new ELEM_TYPE[INTERNAL_BUFFER_SIZE];
      _internalOutputBuffer = new ELEM_TYPE[INTERNAL_BUFFER_SIZE];

      _inputBufferSize = 0;

      _inputStream = &input;

      _bytesRead = 0;
      _bytesWritten = 0;

      _endOfStream = false;

      // default memory functions
      _zstream.zalloc = nullptr;
      _zstream.zfree  = nullptr;
      _zstream.opaque = nullptr;
      _zstream.avail_out = uInt(-1); // force first load of data (see decompress_buffer())

      // set stream buffer
      ELEM_TYPE* endOfOutputBuffer = _internalOutputBuffer + INTERNAL_BUFFER_SIZE;
      this->setg(endOfOutputBuffer, endOfOutputBuffer, endOfOutputBuffer);

      return zlib_suceeded(inflateInit2(&_zstream, -MAX_WBITS));
    }

    bool is_init() const
    {
      return (_inputStream != nullptr && _internalInputBuffer != nullptr && _internalOutputBuffer != nullptr);
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
    int_type underflow() override
    {
      // buffer exhausted
      if (this->gptr() >= this->egptr())
      {
        ELEM_TYPE* base = _internalOutputBuffer;

        // how many bytes has been read
        size_t n = static_cast<size_t>(decompress_buffer());

        if (n == 0)
        {
          return traits_type::eof();
        }

        // set buffer pointers
        this->setg(base, base, base + n);
      }

      return traits_type::to_int_type(*this->gptr());
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

        return zlib_suceeded(inflateEnd(&_zstream));
      }

      return true;
    }

    void read_input_stream()
    {
      // read next bytes from input stream
      _inputStream->read(_internalInputBuffer, INTERNAL_BUFFER_SIZE);

      // set the size of buffer (multiply by INTERNAL_ELEMENT_SIZE?)
      _inputBufferSize = static_cast<size_t>(_inputStream->gcount());

      // increase amount of total read bytes
      _bytesRead += _inputBufferSize;

      // if it was not read requested count of bytes, input stream is ending
      _endOfStream = _inputBufferSize != INTERNAL_BUFFER_REAL_SIZE;
    }

    int decompress_buffer()
    {
      // do not load any data until there
      // are something left
      if (_zstream.avail_out != 0)
      {
        // if all data has not been fetched and the stream is at the end,
        // it is an error
        if (_endOfStream)
        {
          return 0;
        }

        // read data into buffer
        read_input_stream();

        // set input buffer and its size
        _zstream.next_in   = reinterpret_cast<Bytef*>(_internalInputBuffer);
        _zstream.avail_in  = static_cast<uInt>(_inputBufferSize);
      }

      // zstream output
      _zstream.next_out  = reinterpret_cast<Bytef*>(_internalOutputBuffer);
      _zstream.avail_out = static_cast<uInt>(INTERNAL_BUFFER_REAL_SIZE);

      // inflate stream
      if (!zlib_suceeded(inflate(&_zstream, Z_NO_FLUSH)))
      {
        return 0;
      }

      // associate output buffer
      size_t bytesProcessed = INTERNAL_BUFFER_REAL_SIZE - static_cast<size_t>(_zstream.avail_out);

      // increase amount of total written bytes
      _bytesWritten += bytesProcessed;

      if (_lastError == Z_STREAM_END)
      {
        _endOfStream = true;

        // if we read more than we should last time, move pointer to the correct position
        if (_zstream.avail_in > 0)
        {
          _inputStream->clear();
          _inputStream->seekg(-static_cast<off_type>(_zstream.avail_in), std::ios::cur);
        }
      }

      // return count of processed bytes from input stream
      return static_cast<int>(bytesProcessed);
    }

    //////////////////////////////////////////////////////////////////////////

    enum : size_t
    {
      INTERNAL_BUFFER_SIZE      = STREAM_DEFAULT_BUFFER_SIZE,
      INTERNAL_ELEMENT_SIZE     = sizeof(ELEM_TYPE),
      INTERNAL_BUFFER_REAL_SIZE = INTERNAL_BUFFER_SIZE * INTERNAL_ELEMENT_SIZE
    };

    //////////////////////////////////////////////////////////////////////////

    std::basic_istream<ELEM_TYPE, TRAITS_TYPE>* _inputStream;

    z_stream    _zstream;                 // internal zlib structure
    int         _lastError;               // last error of zlib operation

    bool        _endOfStream;             // flag indicating, if the input stream or the deflate stream
                                          // has ended

    ELEM_TYPE*  _internalInputBuffer;     // pointer to the start of the input buffer
    ELEM_TYPE*  _internalOutputBuffer;    // pointer to the start of the output buffer

    size_t      _inputBufferSize;         // how many bytes are read in the input buffer
    size_t      _bytesRead;               // total read bytes
    size_t      _bytesWritten;            // total written bytes
};
