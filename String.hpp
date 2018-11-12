#pragma once

#include <array>

namespace Impl {
  template<typename T>
  constexpr auto to_chars(T val) {
    std::array<char, sizeof(T)*2 + 1> ret;

    auto it = ret.rbegin();
    *(it++) = '\x00';

    for(; it != ret.rend(); ++it, val >>= 4) {
      *it = "0123456789abcdef"[val & 0x0f];
    }

    return ret;
  }

  template<typename T>
  struct to_chars_impl {
    constexpr auto operator()(T const &val) {
      if constexpr(std::is_pointer_v<T>) {
        return to_chars(reinterpret_cast<std::intptr_t>(val));
      }
      if constexpr(std::numeric_limits<T>::is_integer) {
        return to_chars(val);
      }
    }
  };
}

template<typename T>
auto to_chars(T const &val) {
  return Impl::to_chars_impl<T>{}(val);
}
