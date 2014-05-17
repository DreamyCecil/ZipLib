#pragma once
#include <streambuf>
#include <algorithm>
#include <vector>

class tee_streambuf
  : public std::streambuf
{
  public:
    tee_streambuf& bind(std::streambuf* sb)
    {
      _sbCollection.push_back(sb);
      return *this;
    }

    tee_streambuf& bind(std::ostream& stream)
    {
      _sbCollection.push_back(stream.rdbuf());
      return *this;
    }

  protected:
    int_type overflow(int_type c = traits_type::eof()) override
    {
      bool failed = false;
      
      for (auto* sb : _sbCollection)
      {
        if (sb->sputc(c) == traits_type::eof())
        {
          failed = true;
        }
      }

      return failed ? traits_type::eof() : c;
    }
  
    int sync() override
    {
      bool failed = false;

      for (auto* sb : _sbCollection)
      {
        if (sb->pubsync() == -1)
        {
          failed = true;
        }
      }

      return failed ? -1 : 0;
    }
  
  private:
    std::vector<std::streambuf*> _sbCollection;
};
