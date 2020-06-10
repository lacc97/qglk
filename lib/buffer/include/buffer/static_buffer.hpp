#ifndef BUFFER_STATIC_BUFFER_HPP
#define BUFFER_STATIC_BUFFER_HPP

#include <array>

#include "buffer_base.hpp"

namespace buffer {
  template <typename CharT, size_t N>
  class static_buffer : public detail::buffer_base<CharT, static_buffer<CharT, N>> {
      using base_type = detail::buffer_base<CharT, static_buffer<CharT, N>>;

      using array_type = std::array<CharT, N>;

    public:
      inline constexpr static_buffer() noexcept = default;

      inline constexpr static_buffer(const static_buffer&) noexcept = default;

      inline constexpr static_buffer(static_buffer&&) noexcept = default;

      inline ~static_buffer() = default;


      inline constexpr static_buffer& operator=(const static_buffer&) noexcept = default;

      inline constexpr static_buffer& operator=(static_buffer&&) noexcept = default;


      inline typename base_type::pointer storage_pointer() {
        return m_Storage.data();
      }

      inline typename base_type::const_pointer storage_pointer() const {
        return m_Storage.data();
      }

      inline typename base_type::size_type storage_size() const {
        return m_Storage.size();
      }

    private:
      array_type m_Storage;
  };

  template <size_t N>
  using static_byte_buffer = static_buffer<char, N>;
}

#endif //BUFFER_STATIC_BUFFER_HPP
