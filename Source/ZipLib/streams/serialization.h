#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <list>

namespace detail {

  template <typename CONTAINER_TYPE>
  void resize_stl_container(CONTAINER_TYPE& out, size_t size)
  {
    out.resize(size, CONTAINER_TYPE::value_type());
  }

  template <typename CONTAINER_VALUE_TYPE, size_t SIZE_TYPE>
  void resize_stl_container(std::array<CONTAINER_VALUE_TYPE, SIZE_TYPE>& out, size_t size)
  {

  }

  template <typename ELEM_TYPE, typename TRAITS_TYPE, typename CONTAINER_TYPE>
  void serialize_stl_container(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const CONTAINER_TYPE& value)
  {
    static_assert(sizeof(ELEM_TYPE) == sizeof(CONTAINER_TYPE::value_type), "sizes must match");

    stream.write(reinterpret_cast<const ELEM_TYPE*>(&value[0]), value.size());
  }

  template <typename ELEM_TYPE, typename TRAITS_TYPE, typename CONTAINER_TYPE>
  std::ios::pos_type deserialize_stl_container(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, CONTAINER_TYPE& out, size_t size)
  {
    static_assert(sizeof(ELEM_TYPE) == sizeof(CONTAINER_TYPE::value_type), "sizes must match");

    if (size > 0)
    {
      resize_stl_container(out, size);
      stream.read(reinterpret_cast<ELEM_TYPE*>(&out[0]), size);
      return stream.gcount();
    }

    return 0;
  }

}

/**
 * \brief Serializes the basic input type. Serializes floating number into its binary representation.
 *
 * \tparam  TYPE      Type of the value to be serialized.
 * \param   stream    The stream to be serialized into.
 * \param   value     The value to be serialized.
 */
template <typename TYPE, typename ELEM_TYPE, typename TRAITS_TYPE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const TYPE& value)
{
  stream.write(reinterpret_cast<const ELEM_TYPE*>(&value), sizeof(TYPE) / sizeof(ELEM_TYPE));
}

/**
 * \brief Serializes the input type of specified size
 *
 * \tparam  TYPE      Type of the value to be serialized.
 * \param   stream    The stream to be serialized into.
 * \param   value     Pointer to the value to be serialized.
 * \param   size      Amount of TYPE to be written.
 */
template <typename TYPE, typename ELEM_TYPE, typename TRAITS_TYPE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const TYPE* value, size_t size)
{
  stream.write(reinterpret_cast<const ELEM_TYPE*>(value), size);
}

/**
 * \brief Serializes the string.
 * \param   stream    The stream to be serialized into.
 * \param   value     The string to be serialized.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE, typename ALLOCATOR_TYPE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const std::basic_string<ELEM_TYPE, TRAITS_TYPE, ALLOCATOR_TYPE>& value)
{
  detail::serialize_stl_container(stream, value);
}

/**
 * \brief Serializes the vector.
 *
 * \param   stream    The stream to be serialized into.
 * \param   value     The vector to be serialized.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE, typename VECTOR_ELEM_TYPE, typename ALLOCATOR_TYPE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const std::vector<VECTOR_ELEM_TYPE, ALLOCATOR_TYPE>& value)
{
  detail::serialize_stl_container(stream, value);
}

/**
 * \brief Serializes the list.
 *
 * \param   stream    The stream to be serialized into.
 * \param   value     The list to be serialized.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE, typename LIST_ELEM_TYPE, typename ALLOCATOR_TYPE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const std::list<LIST_ELEM_TYPE, ALLOCATOR_TYPE>& value)
{
  detail::serialize_stl_container(stream, value);
}

/**
 * \brief Serializes the array.
 *
 * \param   stream    The stream to be serialized into.
 * \param   value     The array to be serialized.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE, typename ARRAY_ELEM_TYPE, size_t ARRAY_SIZE>
void serialize(std::basic_ostream<ELEM_TYPE, TRAITS_TYPE>& stream, const std::array<ARRAY_ELEM_TYPE, ARRAY_SIZE>& value)
{
  detail::serialize_stl_container(stream, value);
}

/**
 * \brief Deserializes the basic input type. Deserializes floating number from its binary representation.
 *
 * \tparam  RETURN_TYPE  Type of the value to be deserialized.
 * \param   stream       The stream to be deserialized from.
 * \param   [out] out    The deserialized value.
 *
 * \return  Count of read elements.
 */
template <typename RETURN_TYPE, typename ELEM_TYPE, typename TRAITS_TYPE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, RETURN_TYPE& out)
{
  stream.read(reinterpret_cast<ELEM_TYPE*>(&out), sizeof(RETURN_TYPE));
  return stream.gcount();
}

/**
 * \brief Deserializes the input type from the stream
 *
 * \tparam  RETURN_TYPE  Type of the value to be deserialized.
 * \param   stream       The stream to be deserialized from.
 * \param   [out] out    Pointer to the deserialized value.
 * \param   size         The expected size of return type. The value represents the amount of RETURN_TYPE to be read.
 *
 * \return  Count of read elements.
 */
template <typename RETURN_TYPE, typename ELEM_TYPE, typename TRAITS_TYPE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, RETURN_TYPE* out, size_t size)
{
  stream.read(reinterpret_cast<ELEM_TYPE*>(out), size);
  return stream.gcount();
}

/**
 * \brief Deserializes the string from the stream.
 * \param   stream    The stream to be deserialized from.
 * \param   [out] out The deserialized string.
 * \param   size      The expected size of the string. The value represents the amount of ELEM_TYPE to be read.
 *                    
 * \return  Count of read elements.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE, typename ALLOCATOR_TYPE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, std::basic_string<ELEM_TYPE, TRAITS_TYPE, ALLOCATOR_TYPE>& out, size_t size)
{
  return detail::deserialize_stl_container(stream, out, size);
}

/**
 * \brief Deserializes the vector from the stream.
 * \param   stream    The stream to be deserialized from.
 * \param   [out] out The deserialized vector.
 * \param   size      The expected amount of elements in the serialized vector.
 *                    
 * \return  Count of read elements.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE, typename VECTOR_ELEM_TYPE, typename ALLOCATOR_TYPE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, std::vector<VECTOR_ELEM_TYPE, ALLOCATOR_TYPE>& out, size_t size)
{
  return detail::deserialize_stl_container(stream, out, size);
}

/**
 * \brief Deserializes the list from the stream.
 * \param   stream    The stream to be deserialized from.
 * \param   [out] out The deserialized list.
 * \param   size      The expected amount of elements in the serialized list.
 *                    
 * \return  Count of read elements.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE, typename LIST_ELEM_TYPE, typename ALLOCATOR_TYPE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, std::list<LIST_ELEM_TYPE, ALLOCATOR_TYPE>& out, size_t size)
{
  return detail::deserialize_stl_container(stream, out, size);
}

/**
 * \brief Deserializes the array from the stream.
 * \param   stream    The stream to be deserialized from.
 * \param   [out] out The deserialized array.
 *                    
 * \return  Count of read elements.
 */
template <typename ELEM_TYPE, typename TRAITS_TYPE, typename ARRAY_ELEM_TYPE, size_t ARRAY_SIZE>
std::ios::pos_type deserialize(std::basic_istream<ELEM_TYPE, TRAITS_TYPE>& stream, std::array<ARRAY_ELEM_TYPE, ARRAY_SIZE>& out)
{
  return detail::deserialize_stl_container(stream, out, out.size());
}