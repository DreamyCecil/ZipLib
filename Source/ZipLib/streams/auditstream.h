#pragma once
#include <iostream>
#include "streambuffs/audit_streambuf.h"

class iauditstream
  : public std::istream
{
  public:
    iauditstream()
      : std::istream(&_auditStreambuf)
    {

      }

    iauditstream(std::ios& stream)
      : std::istream(&_auditStreambuf)
      , _auditStreambuf(stream)
    {

    }

    void set_stream(std::ios& stream)
    {
      _auditStreambuf.set_stream(stream);
    }

    size_t get_bytes_read() const
    {
      return _auditStreambuf.get_bytes_read();
    }

  private:
    audit_streambuf _auditStreambuf;
};

class oauditstream
  : public std::ostream
{
  public:
    oauditstream()
      : std::ostream(&_auditStreambuf)
    {

      }

    oauditstream(std::ios& stream)
      : std::ostream(&_auditStreambuf)
      , _auditStreambuf(stream)
    {

    }

    void set_stream(std::ios& stream)
    {
      _auditStreambuf.set_stream(stream);
    }

    size_t get_bytes_written() const
    {
      return _auditStreambuf.get_bytes_written();
    }

  private:
    audit_streambuf _auditStreambuf;
};

class ioauditstream
  : public std::iostream
{
  public:
    ioauditstream()
      : std::iostream(&_auditStreambuf)
    {

      }

    ioauditstream(std::ios& stream)
      : std::iostream(&_auditStreambuf)
      , _auditStreambuf(stream)
    {

    }

    void set_stream(std::ios& stream)
    {
      _auditStreambuf.set_stream(stream);
    }

    size_t get_bytes_read() const
    {
      return _auditStreambuf.get_bytes_read();
    }

    size_t get_bytes_written() const
    {
      return _auditStreambuf.get_bytes_written();
    }

  private:
    audit_streambuf _auditStreambuf;
};
