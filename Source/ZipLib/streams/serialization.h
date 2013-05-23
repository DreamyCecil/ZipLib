#pragma once
#include <iostream>
#include <string>
#include <vector>

template <typename RETURN_TYPE, typename ELEM_TYPE, typename TRAITS_TYPE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, RETURN_TYPE& out)
{
  stream.read(reinterpret_cast<ELEM_TYPE*>(&out), sizeof(RETURN_TYPE));
  return stream.gcount();
}

template <typename ELEM_TYPE, typename TRAITS_TYPE, typename ALLOCATOR_TYPE, template <typename, typename, typename> class STRING_TYPE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, STRING_TYPE<ELEM_TYPE, TRAITS_TYPE, ALLOCATOR_TYPE>& out, size_t length)
{
  if (length > 0)
  {
    out.resize(length, ELEM_TYPE(0));
    stream.read(reinterpret_cast<ELEM_TYPE*>(&out[0]), length);
    return stream.gcount();
  }
  
  return 0;
}

template <typename ELEM_TYPE, typename TRAITS_TYPE, typename VECTOR_ELEM_TYPE, typename ALLOCATOR_TYPE, template <typename, typename> class VECTOR_TYPE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, VECTOR_TYPE<VECTOR_ELEM_TYPE, ALLOCATOR_TYPE>& out, size_t length)
{
  static_assert(sizeof(ELEM_TYPE) == sizeof(VECTOR_ELEM_TYPE), "sizes must match");

  if (length > 0)
  {
    out.resize(length, VECTOR_ELEM_TYPE());
    stream.read(reinterpret_cast<ELEM_TYPE*>(&out[0]), length);
    return stream.gcount();
  }
  
  return 0;
}

//////////////////////////////////////////////////////////////////////////

template <typename TYPE, typename ELEM_TYPE, typename TRAITS_TYPE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const TYPE& value)
{
  stream.write(reinterpret_cast<const ELEM_TYPE*>(&value), sizeof(TYPE) / sizeof(ELEM_TYPE));
}

template <typename ELEM_TYPE, typename TRAITS_TYPE, typename ALLOCATOR_TYPE, template <typename, typename, typename> class STRING_TYPE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const STRING_TYPE<ELEM_TYPE, TRAITS_TYPE, ALLOCATOR_TYPE>& value)
{
  stream.write(reinterpret_cast<const ELEM_TYPE*>(&value[0]), value.length());
}

template <typename ELEM_TYPE, typename TRAITS_TYPE, typename VECTOR_ELEM_TYPE, typename ALLOCATOR_TYPE, template <typename, typename> class VECTOR_TYPE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const VECTOR_TYPE<VECTOR_ELEM_TYPE, ALLOCATOR_TYPE>& value)
{
  static_assert(sizeof(ELEM_TYPE) == sizeof(VECTOR_ELEM_TYPE), "sizes must match");

  stream.write(reinterpret_cast<const ELEM_TYPE*>(&value[0]), value.size());
}
