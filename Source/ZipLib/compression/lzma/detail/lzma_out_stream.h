#pragma once
#include "../../../extlibs/lzma/Types.h"

namespace detail
{
  template <typename ELEM_TYPE, typename TRAITS_TYPE>
  class lzma_out_stream
    : public ISeqOutStream
  {
    public:
      typedef std::basic_ostream<ELEM_TYPE, TRAITS_TYPE> ostream_type;

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
        auto currentPosition = _stream->tellp();
        _stream->write(reinterpret_cast<const ELEM_TYPE*>(buf), size);

        size_t delta = static_cast<size_t>(_stream->tellp()) - static_cast<size_t>(currentPosition);

        return delta;
      }

      const ostream_type& get_stream() const { return *_stream; }
      ostream_type& get_stream() { return *_stream; }
      void set_stream(ostream_type& stream) { _stream = &stream; }

    private:
      ostream_type* _stream = nullptr;
  };
}
