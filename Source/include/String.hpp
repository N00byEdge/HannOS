#pragma once

#include <array>
#include <type_traits>
#include <charconv>

namespace HannOS::String {
  namespace Impl {
    template<typename T>
    struct NumChars: std::integral_constant<int, sizeof(T) * 2> { };
    template <>
    struct NumChars<bool>: std::integral_constant<int, 1> { };

    template<bool nullTerminate = true, typename T>
    [[nodiscard]]
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
      [[nodiscard]]
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
  [[nodiscard]]
  auto to_chars(T const &val) {
    return Impl::to_chars_impl<nullTerminate, T>{}(val);
  }

  template<int numParts>
  using splitResult = std::array<std::string_view, numParts>;

  template<int targetParts>
  auto split(std::string_view source, char splitOn, int &partsOut) {
    static_assert(targetParts >= 1, "Has to split into at least one part");
    partsOut = 1;
    splitResult<targetParts> ret{source};

    for(auto currPart = ret.begin(); partsOut < targetParts; ++currPart, ++partsOut) {
      auto foundAt = currPart->find(splitOn);
      if(foundAt == currPart->npos)
        break;
      *std::next(currPart) = currPart->substr(foundAt + 1);
      *currPart = currPart->substr(0, foundAt);
    }
    return ret;
  }

  template<int targetParts>
  auto split(std::string_view source, char splitOn) {
    int parts;
    return split<targetParts>(source, splitOn, parts);
  }

  template<typename ...Args, typename Value>
  void parse(std::string_view text, Value &val, Args ...args) {
    std::to_chars(text.begin(), text.end(), val);
    parse(args...);
  }
}
