#ifndef BUFFER_BUFFER_SPAN_HPP
#define BUFFER_BUFFER_SPAN_HPP

#include "conversion/c_array_conversion.hpp"

#include "private/byte.hpp"
#include "private/conversion_helpers.hpp"
#include "private/span_def.hpp"

namespace buffer {
  using byte_buffer_span = buffer_span<byte>;


  template <typename T>
  inline constexpr auto to_byte_span(T&& t) noexcept {
    return to_span<byte, T>(std::forward<T>(t));
  }
}

#endif //BUFFER_BUFFER_SPAN_HPP
