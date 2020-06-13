#ifndef BUFFER_BUFFER_BASE_HPP
#define BUFFER_BUFFER_BASE_HPP

#include <cassert>

#include <iterator>
#include <type_traits>

#include "buffer_span.hpp"
#include "buffer_view.hpp"

namespace buffer::detail {
  template<typename CharT, typename ImplementationT>
  class buffer_base {
      static_assert(std::is_same_v < std::remove_cv_t < CharT > , CharT > ,
                    "buffer must have non-const, non-volatile type");
      static_assert(std::is_integral_v < CharT > && !std::is_same_v < CharT, bool > ,
                    "buffer must have integral, non-bool type");

      using impl_type = ImplementationT;
    public:
      using value_type = CharT;
      using size_type = size_t;
      using difference_type = ptrdiff_t;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using iterator = pointer;
      using const_iterator = const_pointer;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;


      [[nodiscard]] inline iterator begin() noexcept {
        return data();
      }

      [[nodiscard]] inline const_iterator begin() const noexcept {
        return cbegin();
      }

      [[nodiscard]] inline const_iterator cbegin() const noexcept {
        return data();
      }

      [[nodiscard]] inline const_iterator cend() const noexcept {
        return data() + size();
      }

      [[nodiscard]] inline pointer data() noexcept {
        return static_cast<impl_type*>(this)->storage_pointer();
      }

      [[nodiscard]] inline const_pointer data() const noexcept {
        return static_cast<const impl_type*>(this)->storage_pointer();
      }

      [[nodiscard]] inline bool empty() const noexcept {
        return size() == 0;
      }

      [[nodiscard]] inline iterator end() noexcept {
        return data() + size();
      }

      [[nodiscard]] inline const_iterator end() const noexcept {
        return cend();
      }

      [[nodiscard]] inline buffer::buffer_span<CharT> first(size_type n) noexcept {
        return buffer::buffer_span<CharT>{*this}.first(n);
      }

      [[nodiscard]] inline buffer::buffer_view<CharT> first(size_type n) const noexcept {
        return buffer::buffer_view<CharT>{*this}.first(n);
      }

      [[nodiscard]] inline size_type size() const noexcept {
        return static_cast<const impl_type*>(this)->storage_size();
      }

      [[nodiscard]] buffer::buffer_view<value_type> subview(size_type offset) const noexcept {
        return buffer::buffer_view<value_type>{*this}.subspan(offset);
      }

      [[nodiscard]] buffer::buffer_span<value_type> subspan(size_type offset) noexcept {
        return buffer::buffer_span<value_type>{*this}.subspan(offset);
      }

      [[nodiscard]] buffer::buffer_view<value_type> subview(size_type offset, size_type count) const noexcept {
        return buffer::buffer_view<value_type>{*this}.subspan(offset, count);
      }

      [[nodiscard]] buffer::buffer_span<value_type> subspan(size_type offset, size_type count) noexcept {
        return buffer::buffer_span<value_type>{*this}.subspan(offset, count);
      }


      [[nodiscard]] inline reference operator[](size_type pos) {
        assert(pos < size());
        return data()[pos];
      }

      [[nodiscard]] inline const_reference operator[](size_type pos) const {
        assert(pos < size());
        return data()[pos];
      }
  };
}

#endif //BUFFER_BUFFER_BASE_HPP
