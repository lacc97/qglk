#ifndef BUFFER_BUFFER_SPAN_HPP
#define BUFFER_BUFFER_SPAN_HPP

#include <cstring>

#include <algorithm>
#include <type_traits>

namespace buffer {
  namespace detail {
    template <typename CharT, typename ImplementationT>
    class buffer_base;

    template<typename CharT, bool Mutable>
    class buffer_span {
        static_assert(std::is_same_v<std::remove_cv_t<CharT>, CharT>, "buffer must have non-const, non-volatile type");
        static_assert(std::is_integral_v<CharT> && !std::is_same_v<CharT, bool>, "buffer must have integral, non-bool type");

        template <typename OtherCharT>
        inline static constexpr bool type_compatible_v = (sizeof(OtherCharT) == sizeof(CharT));

        template <typename OtherCharT>
        using buffer_allowed_t = std::enable_if_t<type_compatible_v<OtherCharT>, bool>;

        template <typename OtherCharT, bool OtherMutable>
        using span_allowed_t = std::enable_if_t<type_compatible_v<OtherCharT> && (OtherMutable || !Mutable), bool>;

        template <typename OtherCharT, bool OtherMutable>
        using span_allowed_in_constructor_t = std::enable_if_t<!std::is_same_v<OtherCharT, CharT> || Mutable != OtherMutable, span_allowed_t<OtherCharT, OtherMutable>>;

        template <typename OtherCharT, bool OtherMutable>
        using span_allowed_in_assignment_t = std::enable_if_t<!std::is_same_v<OtherCharT, CharT> || Mutable != OtherMutable, span_allowed_t<OtherCharT, OtherMutable>>;

      public:
        using value_type = std::conditional_t<Mutable, CharT, const CharT>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = value_type*;
        using const_iterator = const value_type*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;


        inline constexpr buffer_span() noexcept = default;

        inline constexpr buffer_span(const buffer_span&) noexcept = default;

        inline constexpr buffer_span(buffer_span&&) noexcept = default;

        template <typename OtherCharT, typename ImplT, buffer_allowed_t<OtherCharT> = true>
        inline constexpr buffer_span(buffer_base<OtherCharT, ImplT>& buf) noexcept
          : m_Ptr{reinterpret_cast<pointer>(buf.data())},
            m_Size{buf.size()} {}

        template <typename OtherCharT, typename ImplT, buffer_allowed_t<OtherCharT> = true>
        inline constexpr buffer_span(const buffer_base<OtherCharT, ImplT>& buf) noexcept
          : m_Ptr{reinterpret_cast<pointer>(buf.data())},
            m_Size{buf.size()} {}

        template<typename OtherCharT, bool OtherMutable, span_allowed_in_constructor_t<OtherCharT, OtherMutable> = true>
        inline constexpr buffer_span(buffer_span<OtherCharT, OtherMutable> other) noexcept
          : m_Ptr{reinterpret_cast<pointer>(other.data())},
            m_Size{other.size()} {}

        template <typename OtherCharT, span_allowed_t<OtherCharT, !std::is_const_v<OtherCharT>> = true>
        inline constexpr buffer_span(OtherCharT* s, size_type count) noexcept
          : m_Ptr{reinterpret_cast<pointer>(s)},
            m_Size{count} {}

        template <typename OtherCharT, span_allowed_t<OtherCharT, !std::is_const_v<OtherCharT>> = true>
        inline constexpr buffer_span(OtherCharT* b, OtherCharT* e) noexcept
          : m_Ptr{reinterpret_cast<pointer>(b)},
            m_Size{e - b} {
          assert(b < e);
        }


        inline constexpr buffer_span& operator=(const buffer_span&) noexcept = default;

        inline constexpr buffer_span& operator=(buffer_span&&) noexcept = default;

        template<typename OtherCharT, typename ImplT, buffer_allowed_t<OtherCharT> = true>
        inline constexpr buffer_span& operator=(buffer_base<OtherCharT, ImplT>& buf) noexcept {
          m_Ptr = reinterpret_cast<pointer>(buf.data());
          m_Size = buf.size();
          return *this;
        }

        template<typename OtherCharT, typename ImplT, buffer_allowed_t<OtherCharT> = true>
        inline constexpr buffer_span& operator=(const buffer_base<OtherCharT, ImplT>& buf) noexcept {
          m_Ptr = reinterpret_cast<pointer>(buf.data());
          m_Size = buf.size();
          return *this;
        }

        template<typename OtherCharT, bool OtherMutable, span_allowed_in_assignment_t<OtherCharT, OtherMutable> = true>
        inline constexpr buffer_span& operator=(buffer_span<OtherCharT, OtherMutable> other) noexcept {
          m_Ptr = reinterpret_cast<pointer>(other.data());
          m_Size = other.m_Size;
          return *this;
        }


        [[nodiscard]] inline constexpr iterator begin() const noexcept {
          return m_Ptr;
        }

        [[nodiscard]] inline constexpr const_iterator cbegin() const noexcept {
          return m_Ptr;
        }

        [[nodiscard]] inline constexpr const_iterator cend() const noexcept {
          return m_Ptr + m_Size;
        }

        template<typename OutputIterator>
        inline size_type copy(OutputIterator out, size_type count, size_type pos = 0) const noexcept {
          auto src = subspan(pos, count);
          return std::distance(out, std::copy(src.begin(), src.end(), out));
        }

        template<typename OtherCharT, span_allowed_t<OtherCharT, true> = true>
        inline size_type copy_to(buffer_span<OtherCharT, true> dst, size_type count, size_type pos = 0) const noexcept {
          auto src = subspan(pos, std::min(count, dst.size()));
          std::memmove(dst.data(), src.data(), src.size());
          return src.size();
        }

        [[nodiscard]] inline constexpr const_reverse_iterator crbegin() const noexcept {
          return std::make_reverse_iterator<const_iterator>(cend());
        }

        [[nodiscard]] inline constexpr const_reverse_iterator crend() const noexcept {
          return std::make_reverse_iterator<const_iterator>(cbegin());
        }

        [[nodiscard]] inline constexpr pointer data() const noexcept {
          return m_Ptr;
        }

        [[nodiscard]] inline constexpr bool empty() const noexcept {
          return m_Size == 0;
        }

        [[nodiscard]] inline constexpr iterator end() const noexcept {
          return m_Ptr + m_Size;
        }

        [[nodiscard]] inline constexpr buffer_span first(size_type count) const noexcept {
          return subspan(0, count);
        }

        [[nodiscard]] inline constexpr buffer_span last(size_type count) const noexcept {
          return subspan(m_Size - std::min(count, m_Size));
        }

        [[nodiscard]] inline constexpr reverse_iterator rbegin() const noexcept {
          return std::make_reverse_iterator<iterator>(end());
        }

        [[nodiscard]] inline constexpr reverse_iterator rend() const noexcept {
          return std::make_reverse_iterator<iterator>(begin());
        }

        [[nodiscard]] inline constexpr size_type size() const noexcept {
          return m_Size;
        }

        [[nodiscard]] inline constexpr buffer_span subspan(size_type off) const noexcept {
          auto realOff = std::min(off, m_Size);
          return buffer_span{m_Ptr + realOff, m_Size - realOff};
        }

        [[nodiscard]] inline constexpr buffer_span subspan(size_type off, size_type count) const noexcept {
          auto realOff = std::min(off, m_Size);
          return buffer_span{m_Ptr + realOff, std::min(count, m_Size - realOff)};
        }


        [[nodiscard]] constexpr reference operator[](size_type pos) const noexcept {
          return m_Ptr[pos];
        }

      private:
        pointer m_Ptr{nullptr};
        size_type m_Size{0};
    };
  }

  template<typename CharT>
  using buffer_span = detail::buffer_span<CharT, true>;

  using byte_buffer_span = buffer_span<char>;
}

#endif //BUFFER_BUFFER_SPAN_HPP
