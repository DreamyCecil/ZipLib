#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdint>

struct encryption_properties_interface
{
  template <typename T>
  static inline T clamp(T minimum, T maximum, T value)
  {
    return std::min(std::max(minimum, value), maximum);
  }

  virtual void normalize() { }
  
  size_t BufferCapacity = 1 << 15;
  std::string Password;
};

class encryption_interface
{
  public:
    typedef char char_type;

    virtual ~encryption_interface() { }

    virtual bool is_init() const = 0;

    virtual char_type* get_buffer_begin() = 0;
    virtual char_type* get_buffer_end() = 0;
};

class encryption_encoder_interface
  : public encryption_interface
{
  public:
    virtual void init(std::ostream& stream) = 0;
    virtual void init(std::ostream& stream, encryption_properties_interface& props) = 0;
    virtual void encrypt_next(size_t length) = 0;
    virtual void sync() = 0;
};

class encryption_decoder_interface
  : public encryption_interface
{
  public:
    virtual void init(std::istream& stream) = 0;
    virtual void init(std::istream& stream, encryption_properties_interface& props) = 0;
    virtual size_t decrypt_next() = 0;
    virtual bool has_correct_password() const = 0;
};
