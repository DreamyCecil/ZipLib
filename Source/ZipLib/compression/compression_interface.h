#pragma once
#include <iostream>
#include <algorithm>
#include <cstdint>

struct compression_properties_interface
{
  template <typename T>
  static inline T clamp(T minimum, T maximum, T value)
  {
    return std::min(std::max(minimum, value), maximum);
  }

  virtual void normalize() { }

  size_t BufferCapacity = 1 << 15;
};

class compression_interface
{
  public:
    virtual ~compression_interface() { }

    virtual bool is_init() const = 0;

    virtual uint8_t* get_buffer_begin() = 0;
    virtual uint8_t* get_buffer_end() = 0;
};

class compression_encoder_interface
  : public compression_interface
{
  public:
    virtual void init(std::ostream& stream) = 0;
    virtual void init(std::ostream& stream, compression_properties_interface& props) = 0;
    virtual void encode_next(size_t length) = 0;
    virtual void sync() = 0;
};

class compression_decoder_interface
  : public compression_interface
{
  public:
    virtual void init(std::istream& stream) = 0;
    virtual void init(std::istream& stream, compression_properties_interface& props) = 0;
    virtual size_t decode_next() = 0;
};
