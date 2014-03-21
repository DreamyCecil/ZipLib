#pragma once
#include <streambuf>
#include <cstdint>
#include <cassert>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class audit_streambuf
  : public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::char_type char_type;
    typedef typename base_type::int_type  int_type;
    typedef typename base_type::pos_type  pos_type;
    typedef typename base_type::off_type  off_type;

    typedef base_type streambuf_type;
    typedef std::basic_ios<ELEM_TYPE, TRAITS_TYPE> stream_type;

    audit_streambuf()
      : _streambuf(nullptr)
      , _bytesRead(0)
      , _bytesWritten(0)
    {

    }

    audit_streambuf(stream_type& stream)
      : _streambuf(stream.rdbuf())
      , _bytesRead(0)
      , _bytesWritten(0)
    {

    }

    void set_stream(stream_type& stream)
    {
      _streambuf = stream.rdbuf();
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
    pos_type seekpos(pos_type pos, std::ios::openmode which = std::ios::in | std::ios::out) override
    {
      return _streambuf->pubseekpos(pos, which);
    }

    pos_type seekoff(off_type off, std::ios::seekdir dir, std::ios::openmode which = std::ios::in | std::ios::out) override
    {
      return _streambuf->pubseekoff(off, dir, which);
    }

    int_type pbackfail(int_type c = traits_type::eof()) override
    {
      return _streambuf->sungetc();
    }

    std::streamsize xsgetn(char_type* ptr, std::streamsize count) override
    {
      auto n = _streambuf->sgetn(ptr, count);
      _bytesRead += static_cast<size_t>(n);
      return n;
    }

    std::streamsize xsputn(const ELEM_TYPE* ptr, std::streamsize count) override
    {
      auto n = _streambuf->sputn(ptr, count);
      _bytesWritten += static_cast<size_t>(n);
      return n;
    }

    int sync() override
    {
      return _streambuf->pubsync();
    }

  private:
    streambuf_type* _streambuf;
    size_t _bytesRead;
    size_t _bytesWritten;
};
