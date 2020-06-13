#ifndef BIT_CAST_BIT_CAST_HPP
#define BIT_CAST_BIT_CAST_HPP

#if __cplusplus >= 202002L

#include <bit>

template<class to_type, class from_type>
inline constexpr std::enable_if_t<
        (sizeof(to_type) == sizeof(from_type)) && std::is_trivially_copyable_V<from_type> &&
        std::is_trivial_v<to_type> && (std::is_copy_constructible_v<to_type> || std::is_move_constructible_v<to_type>),
        to_type>
        bit_cast(const from_type& src) noexcept {
    return std::bit_cast<to_type>(src);
}
#elif __cplusplus >= 201103L

#include <cstring>

template<class to_type, class from_type>
inline typename std::enable_if<
        (sizeof(to_type) == sizeof(from_type)) && std::is_trivially_copyable<from_type>::value &&
        std::is_trivial<to_type>::value && (std::is_copy_constructible<to_type>::value || std::is_move_constructible<to_type>::value),
        to_type>::type
bit_cast(const from_type& src) noexcept {
    to_type dst;
    std::memcpy(&dst, &src, sizeof(to_type));
    return dst;
}

#else
#error "Required at least c++11"
#endif

#endif //BIT_CAST_BIT_CAST_HPP
