#pragma once
#include "../compression_interface.h"

#include "lzma_properties.h"
#include "detail/lzma_alloc.h"
#include "detail/lzma_handle.h"
#include "detail/lzma_header.h"
#include "detail/lzma_in_stream.h"
#include "detail/lzma_out_stream.h"

#include <ostream>
#include <thread>
#include <cstdint>

class lzma_encoder
  : public compression_encoder_interface
{
  public:
    lzma_encoder()
    {

    }

    ~lzma_encoder()
    {
      if (is_init())
      {
        sync();
      }
    }

    void init(std::ostream& stream) override
    {
      lzma_properties props;
      init(stream, props);
    }

    void init(std::ostream& stream, compression_properties_interface& props) override
    {
      lzma_properties& lzmaProps = static_cast<lzma_properties&>(props);

      _ostream.set_stream(stream);
      lzmaProps.apply(_handle);

      start_compression_thread();
    }

    bool is_init() const override
    {
      return &_ostream.get_stream() != nullptr;
    }

    char_type* get_buffer_begin() override
    {
      return _istream.get_buffer_begin();
    }

    char_type* get_buffer_end() override
    {
      return _istream.get_buffer_end();
    }

    void encode_next(size_t length) override
    {
      _istream.compress(length);
    }

    void sync() override
    {
      if (_compressionThread.joinable())
      {
        _compressionThread.join();
      }
    }

  private:
    void start_compression_thread()
    {
      detail::lzma_header header;
      header.apply(_handle);
      header.write_to_stream(_ostream);

      _compressionThread = std::thread(&lzma_encoder::encode_threadroutine, this);

      _istream.wait_for_event();
    }

    bool encode_threadroutine()
    {
      return LzmaEnc_Encode(_handle.get_native_handle(), &_ostream, &_istream, nullptr, &_alloc, &_alloc) == SZ_OK;
    }

    detail::lzma_handle _handle;
    detail::lzma_alloc  _alloc;
    detail::lzma_in_stream  _istream;
    detail::lzma_out_stream _ostream;

    std::thread _compressionThread;
};
