#pragma once
#include "../../../extlibs/lzma/Types.h"

#include <condition_variable>
#include <mutex>

// forward declaration
class lzma_encoder;

namespace detail
{
  class lzma_in_stream
    : public ISeqInStream
  {
    public:
      friend class ::lzma_encoder;

      typedef std::condition_variable event_type;
      typedef std::mutex              mutex_type;

      lzma_in_stream()
      {
        this->Read = [](void* p, void* buf, size_t* size) -> SRes
        {
          lzma_in_stream* pthis = static_cast<lzma_in_stream*>(p);
          return pthis->read(buf, size);
        };
      }

      SRes read(void* buf, size_t* size)
      {
        size_t lastBytesRead = _bytesRead;

        // set buffer pointer and get required size
        _internalInputBuffer = static_cast<uint8_t*>(buf);
        _internalBufferSize = *size;

        // give control back to the main thread
        set_event();

        // wait for buffer fill
        if (!_endOfStream)
        {
          wait_for_event();
        }

        // copy the data
        if ((_bytesRead - lastBytesRead) < *size)
        {
          _endOfStream = true;
        }

        *size = _bytesRead - lastBytesRead;

        return SZ_OK;
      }

    private:
      size_t      _bytesRead            = 0;
      size_t      _internalBufferSize   = 0;
      uint8_t*    _internalInputBuffer  = nullptr;
      event_type  _event;
      mutex_type  _mutex;
      bool        _endOfStream          = false;

      uint8_t* get_buffer_begin() { return _internalInputBuffer; }
      uint8_t* get_buffer_end() { return _internalInputBuffer + _internalBufferSize; }

      void set_event()
      {
        _event.notify_one();
      }

      void wait_for_event()
      {
        std::unique_lock<mutex_type> lk(_mutex);
        _event.wait(lk);
      }

      void compress(size_t length)
      {
        if (_endOfStream)
        {
          return;
        }

        _bytesRead += length;

        // set event in "read" method -> continue compression
        set_event();

        // wait until compression of the buffer is done
        wait_for_event();
      }
  };
}
