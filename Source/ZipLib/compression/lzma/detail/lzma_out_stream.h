#pragma once
#include "../../../utils/stream/storage.h"
#include "../../../extlibs/lzma/Types.h"

namespace detail
{
  class lzma_out_stream
    : public ISeqOutStream
  {
    public:
      lzma_out_stream()
      {
        this->Write = [](void* p, const void* buf, size_t size)
        {
          lzma_out_stream* pthis = static_cast<lzma_out_stream*>(p);
          return pthis->write(buf, size);
        };
      }

      size_t write(const void* buf, size_t size)
      {
        utils::stream::store(*_stream, buf, size);
        return size;
      }

      const std::ostream& get_stream() const { return *_stream; }
      std::ostream& get_stream() { return *_stream; }
      void set_stream(std::ostream& stream) { _stream = &stream; }

    private:
      std::ostream* _stream = nullptr;
  };
}
