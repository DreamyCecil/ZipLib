#pragma once
#include <streambuf>
#include <istream>
#include <cstdint>

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class sub_streambuf :
  public std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE> base_type;
    typedef typename std::basic_streambuf<ELEM_TYPE, TRAITS_TYPE>::traits_type traits_type;

    typedef typename base_type::char_type char_type;
    typedef typename base_type::int_type  int_type;
    typedef typename base_type::pos_type  pos_type;
    typedef typename base_type::off_type  off_type;

    typedef std::basic_ios<ELEM_TYPE, TRAITS_TYPE>     stream_type;
    typedef std::basic_istream<ELEM_TYPE, TRAITS_TYPE> istream_type;
    typedef std::basic_ostream<ELEM_TYPE, TRAITS_TYPE> ostream_type;

    sub_streambuf()
    {

    }

    sub_streambuf(istream_type& input, pos_type startOffset, size_t length)
    {
      init(input, startOffset, length);
    }

    void init(istream_type& input, pos_type startOffset, size_t length)
    {
      _inputStream = &input;
      _startPosition = startOffset;
      _currentPosition = startOffset;
      _endPosition = startOffset + static_cast<pos_type>(length);
      _internalBuffer = new ELEM_TYPE[INTERNAL_BUFFER_SIZE];

      // set stream buffer
      ELEM_TYPE* endOfOutputBuffer = _internalBuffer + INTERNAL_BUFFER_SIZE;
      this->setg(endOfOutputBuffer, endOfOutputBuffer, endOfOutputBuffer);
    }

    bool is_init() const
    {
      return (_inputStream != nullptr && _internalBuffer != nullptr);
    }

    virtual ~sub_streambuf()
    {
      delete[] _internalBuffer;
    }

  protected:
    int_type underflow() override
    {
      // buffer exhausted
      if (this->gptr() >= this->egptr())
      {
        ELEM_TYPE* base = _internalBuffer;

        _inputStream->seekg(_currentPosition, std::ios::beg);
        _inputStream->read(_internalBuffer, std::min(static_cast<size_t>(INTERNAL_BUFFER_SIZE), static_cast<size_t>(_endPosition - _currentPosition)));
        size_t n = static_cast<size_t>(_inputStream->gcount());

        _currentPosition += n;

        if (n == 0)
        {
          return traits_type::eof();
        }

        // set buffer pointers
        this->setg(base, base, base + n);
      }

      return traits_type::to_int_type(*this->gptr());
    }

    pos_type seekpos(pos_type pos, std::ios::openmode which = std::ios::in | std::ios::out) override
    {
      static const off_type BAD_OFFSET(-1);

      if ((_startPosition + pos) > _endPosition)
      {
        return pos_type(BAD_OFFSET);
      }

      // set new current position
      _currentPosition = _startPosition + pos;

      // invalidate stream buffer
      ELEM_TYPE* endOfOutputBuffer = _internalBuffer + INTERNAL_BUFFER_SIZE;
      this->setg(endOfOutputBuffer, endOfOutputBuffer, endOfOutputBuffer);

      return pos_type(_inputStream->rdbuf()->pubseekpos(_startPosition + pos, which) - _startPosition);
    }

    pos_type seekoff(off_type off, std::ios::seekdir dir, std::ios::openmode which = std::ios::in | std::ios::out) override
    {
      static const off_type BAD_OFFSET(-1);
      
      if ( dir == std::ios::beg && (_startPosition   + off) > _endPosition 
        || dir == std::ios::cur && (_currentPosition + off) > _endPosition
        || dir == std::ios::end && (_endPosition     + off) > _endPosition)
      {
        return pos_type(BAD_OFFSET);
      }

      auto adjust = dir == std::ios::beg ? _startPosition :
                    dir == std::ios::cur ? _currentPosition :
                    dir == std::ios::end ? _endPosition : 0;

      // invalidate stream buffer
      ELEM_TYPE* endOfOutputBuffer = _internalBuffer + INTERNAL_BUFFER_SIZE;
      this->setg(endOfOutputBuffer, endOfOutputBuffer, endOfOutputBuffer);

      // set new current position
      _currentPosition = adjust + off;

      return pos_type(_inputStream->rdbuf()->pubseekpos(adjust + off, which) - _startPosition);
    }
    
  private:
    enum : size_t
    {
      INTERNAL_BUFFER_SIZE = 1 << 15
    };

    ELEM_TYPE* _internalBuffer = nullptr;

    istream_type* _inputStream = nullptr;
    pos_type _startPosition = 0;
    pos_type _currentPosition = 0;
    pos_type _endPosition = 0;
};
