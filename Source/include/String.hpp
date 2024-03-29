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
  struct splitResult {
    std::array<std::string_view, numParts> parts;
    int num;
  };

  template<int targetParts>
  auto split(std::string_view source, char splitOn) {
    static_assert(targetParts >= 1, "Has to split into at least one part");
    splitResult<targetParts> ret;
    ret.num = 1;
    ret.parts[0] = source;

    for(auto currPart = ret.parts.begin(); ret.num < targetParts; ++currPart, ++ret.num) {
      auto foundAt = currPart->find(splitOn);
      if(foundAt == currPart->npos)
        break;
      *std::next(currPart) = currPart->substr(foundAt + 1);
      *currPart = currPart->substr(0, foundAt);
    }

    return ret;
  }

  struct ParseIgnore{};

  namespace Impl {
    template<int ind, int spsz, typename T, typename ...Rest>
    void doParse(splitResult<spsz> const &result, T &&v, Rest &&...rest) {
      if(ind >= result.num)
        return;
      if constexpr(!std::is_same_v<std::decay_t<T>, ParseIgnore>)
        std::from_chars(result.parts[ind].begin(), result.parts[ind].end(), std::forward<T>(v));
      if constexpr(sizeof...(rest))
        return doParse<ind + 1>(result, std::forward<Rest>(rest)...);
    }
  }

  template<typename ...Args>
  auto parse(std::string_view const &text, char splitOn, Args &&...args) {
    auto spl = split<sizeof...(args)>(text, splitOn);
    Impl::doParse<0>(spl, std::forward<Args>(args)...);
    return spl;
  }
}
