#pragma once
#include <streambuf>
#include <cstdint>
#include <cassert>

class audit_streambuf
  : public std::streambuf
{
  public:
    audit_streambuf()
    {

    }

    audit_streambuf(std::ios& stream)
      : _streambuf(stream.rdbuf())
    {

    }

    void set_stream(std::ios& stream)
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

    std::streamsize xsputn(const char_type* ptr, std::streamsize count) override
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
    std::streambuf* _streambuf    = nullptr;
    size_t          _bytesRead    = 0;
    size_t          _bytesWritten = 0;
};
