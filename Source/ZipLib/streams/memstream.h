#pragma once
#include <iostream>
#include <cstdint>
#include "streambuffs/mem_streambuf.h"

/**
 * \brief Basic input memory stream.
 *        Creates input stream around the memory buffer.
 *        Supports seeking.
 *        Returns EOF when stream seeks behind the size of buffer.
 */
class imemstream
  : public std::istream
{
  public:
    imemstream(const char_type* buffer, size_t length)
      : std::istream(&_memStreambuf)
      , _memStreambuf(const_cast<char_type*>(buffer), length)
    {

    }

    template <size_t N>
    imemstream(const char_type (&buffer)[N])
      : imemstream(buffer, N)
    {

    }

  private:
    mem_streambuf _memStreambuf;
};

/**
 * \brief Basic output memory stream.
 *        Creates output stream around the memory buffer.
 *        Supports seeking.
 *        Sets badbit if the stream wants to write behind the buffer size.
 */
class omemstream
  : public std::ostream
{
  public:
    omemstream(char_type* buffer, size_t length)
      : _memStreambuf(buffer, length)
      , std::ostream(&_memStreambuf)
    {

    }

    template <size_t N>
    omemstream(char_type (&buffer)[N])
      : _memStreambuf(buffer, N)
      , std::ostream(&_memStreambuf)
    {

    }

  private:
    mem_streambuf _memStreambuf;
};

/**
 * \brief Basic input/output memory stream.
 *        Combines imemstream & omemstream.
 *        Creates input/output stream around the memory buffer.
 *        Supports seeking.
 *        Returns EOF when stream seeks behind the size of buffer.
 *        Sets badbit if the stream wants to write behind the buffer size.
 */
class iomemstream
  : public std::iostream
{
  public:
    iomemstream(char_type* buffer, size_t length)
      : _memStreambuf(buffer, length)
      , std::iostream(&_memStreambuf)
    {

    }

    template <size_t N>
    iomemstream(char_type (&buffer)[N])
      : _memStreambuf(buffer, N)
      , std::iostream(&_memStreambuf)
    {

    }

  private:
    mem_streambuf _memStreambuf;
};
