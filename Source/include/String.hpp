#pragma once

#include <array>
#include <type_traits>

namespace Impl {
  template<typename T>
  struct NumChars: std::integral_constant<int, sizeof(T) * 2> { };
  template <>
  struct NumChars<bool>: std::integral_constant<int, 1> { };
  template<bool nullTerminate = true, typename T>
  constexpr auto to_chars(T val) {
    std::array<char, NumChars<T>::value + nullTerminate> ret{};

    auto it = ret.rbegin();
    
    if constexpr(nullTerminate)
      *(it++) = '\x00';

    for(; it != ret.rend(); ++it, val >>= 4) {
      *it = "0123456789abcdef"[val & 0x0f];
    }

    return ret;
  }

  template<bool nullTerminate, typename T>
  struct to_chars_impl {
    constexpr auto operator()(T const &val) {
      if constexpr(std::is_pointer_v<T>) {
        return to_chars<nullTerminate>(reinterpret_cast<std::intptr_t>(val));
      }
      if constexpr(std::numeric_limits<T>::is_integer) {
        return to_chars<nullTerminate>(val);
      }
    }
  };
}

template<bool nullTerminate = true, typename T>
auto to_chars(T const &val) {
  return Impl::to_chars_impl<nullTerminate, T>{}(val);
}
