#pragma once
#include <streambuf>

class null_streambuf
  : public std::streambuf
{
  public:
    null_streambuf()
    {

    }

  protected:
    std::streamsize xsgetn(char_type* ptr, std::streamsize count) override
    {
      return 0;
    }

    std::streamsize xsputn(const char_type* ptr, std::streamsize count) override
    {
      return count;
    }
};
