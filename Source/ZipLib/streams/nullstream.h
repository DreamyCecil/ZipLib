#pragma once
#include <iostream>
#include "streambuffs/null_streambuf.h"

/**
 * \brief Basic null stream.
 */
class nullstream
  : public std::iostream
{
  public:
    nullstream()
      : std::iostream(&_nullStreambuf)
    {

    }

  private:
    null_streambuf _nullStreambuf;
};
