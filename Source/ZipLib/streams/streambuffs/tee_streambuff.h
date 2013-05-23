#pragma once
#include <streambuf>
#include <algorithm>

template <typename ELEM_TYPE, typename TRAITS_TYPE = std::char_traits<ELEM_TYPE>>
class tee_streambuf:
  public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::int_type int_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;

    tee_streambuf& bind(base_type* sb)
    {
      _sbCollection.push_back(sb);
      return *this;
    }

    tee_streambuf& bind(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream)
    {
      _sbCollection.push_back(stream.rdbuf());
      return *this;
    }

  protected:
    int_type overflow(int_type c = traits_type::eof())
    {
      if (std::find_if(_sbCollection.begin(),
                       _sbCollection.end(),
                       [&c](base_type* sb) { return sb->sputc(c) == traits_type::eof(); }) != _sbCollection.end())
      {
        return traits_type::eof();
      }
      
      return c;
    }
  
    int sync()
    {
      if (std::find_if(_sbCollection.begin(),
                       _sbCollection.end(),
                       [](base_type* sb) { return sb->pubsync() == -1; }) != _sbCollection.end())
      {
        return -1;
      }

      return 0;
    }
  
  private:
    std::vector<base_type*> _sbCollection;
};
