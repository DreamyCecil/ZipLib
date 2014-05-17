#pragma once
#include <ostream>
#include "streambuffs/tee_streambuff.h"

/**
 * \brief Basic teestream. Distributes the input data into every bound output stream.
 */
class teestream
  : public std::ostream
{
  public:
    teestream()
      : std::ostream(&_teeStreambuf)
    {

    }

    teestream(teestream&& other)
      : teestream()
    {
      _teeStreambuf = std::move(other._teeStreambuf);
      this->swap(other);
    }

    teestream& bind(std::streambuf* sb)
    {
      _teeStreambuf.bind(sb);
      return *this;
    }

    teestream& bind(std::ostream& stream)
    {
      _teeStreambuf.bind(stream);
      return *this;
    }

    teestream move()
    {
      return std::move(*this);
    }

  private:
    teestream(const teestream&) = delete;
    teestream& operator = (const teestream&) = delete;

    tee_streambuf _teeStreambuf;
};
