#pragma once
#include <istream>
#include "streambuffs/sub_streambuf.h"

/**
 * \brief Basic input substream. Creates a virtual stream over an existing input stream.
 *        The substream starts at the position 0 and continues until EOF or the specified length.
 */
class isubstream
  : public std::istream
{
  public:
    isubstream()
      : std::istream(&_subStreambuf)
    {

    }

    isubstream(std::istream& input, pos_type startOffset = 0)
      : std::istream(&_subStreambuf)
      , _subStreambuf(input, startOffset, static_cast<size_t>(-1))
    {

    }

    isubstream(std::istream& input, pos_type startOffset, size_t length)
      : std::istream(&_subStreambuf)
      , _subStreambuf(input, startOffset, length)
    {

    }

    void init(std::istream& input, pos_type startOffset = 0)
    {
      _subStreambuf.init(input, startOffset, static_cast<size_t>(-1));
    }

    void init(std::istream& input, pos_type startOffset, size_t length)
    {
      _subStreambuf.init(input, startOffset, length);
    }

    bool is_init() const
    {
      return _subStreambuf.is_init();
    }

  private:
    sub_streambuf _subStreambuf;
};
