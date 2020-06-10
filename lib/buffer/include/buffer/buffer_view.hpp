#ifndef BUFFER_BUFFER_VIEW_HPP
#define BUFFER_BUFFER_VIEW_HPP

#include "buffer_span.hpp"

namespace buffer {
  template<typename CharT>
  using buffer_view = detail::buffer_span<CharT, false>;

  using byte_buffer_view = buffer_view<char>;
}

#endif //BUFFER_BUFFER_VIEW_HPP
