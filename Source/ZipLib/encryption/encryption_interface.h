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

  virtual void normalize() = 0;
  
  std::string Password;
};

struct encryption_encoder_properties_interface
  : encryption_properties_interface
{

};

struct encryption_decoder_properties_interface
  : encryption_properties_interface
{

};

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class encryption_interface_basic
{
  public:
    typedef std::basic_istream<ELEM_TYPE, TRAITS_TYPE> istream_type;
    typedef std::basic_ostream<ELEM_TYPE, TRAITS_TYPE> ostream_type;

    virtual ~encryption_interface_basic() { }

    virtual bool is_init() const = 0;

    virtual ELEM_TYPE* get_buffer_begin() = 0;
    virtual ELEM_TYPE* get_buffer_end() = 0;
};

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class encryption_encoder_interface_basic
  : public encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    virtual void init(ostream_type& stream) = 0;
    virtual void init(ostream_type& stream, encryption_encoder_properties_interface& props) = 0;
    virtual void encrypt_next(size_t length) = 0;
    virtual void sync() = 0;
};

template <typename ELEM_TYPE, typename TRAITS_TYPE>
class encryption_decoder_interface_basic
  : public encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>
{
  public:
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::istream_type istream_type;
    typedef typename encryption_interface_basic<ELEM_TYPE, TRAITS_TYPE>::ostream_type ostream_type;

    virtual void init(istream_type& stream) = 0;
    virtual void init(istream_type& stream, encryption_decoder_properties_interface& props) = 0;
    virtual size_t decrypt_next() = 0;
    virtual bool has_correct_password() const = 0;
};

typedef encryption_interface_basic<uint8_t, std::char_traits<uint8_t>>           byte_encryption_interface;
typedef encryption_interface_basic<char, std::char_traits<char>>                 encryption_interface;
typedef encryption_interface_basic<wchar_t, std::char_traits<wchar_t>>           wencryption_interface;

typedef encryption_encoder_interface_basic<uint8_t, std::char_traits<uint8_t>>   byte_encryption_encoder_interface;
typedef encryption_encoder_interface_basic<char, std::char_traits<char>>         encryption_encoder_interface;
typedef encryption_encoder_interface_basic<wchar_t, std::char_traits<wchar_t>>   wencryption_encoder_interface;

typedef encryption_decoder_interface_basic<uint8_t, std::char_traits<uint8_t>>   byte_encryption_decoder_interface;
typedef encryption_decoder_interface_basic<char, std::char_traits<char>>         encryption_decoder_interface;
typedef encryption_decoder_interface_basic<wchar_t, std::char_traits<wchar_t>>   wencryption_decoder_interface;
