#pragma once

#include <cstdint>

namespace HannOS {
  struct Random {
    using result_type = std::uint64_t;
    static constexpr result_type min() { return std::numeric_limits<std::uint64_t>::min(); }
    static constexpr result_type max() { return std::numeric_limits<std::uint64_t>::max(); }

    template<typename T = std::uint64_t>
    [[nodiscard]]
    static auto get() {
      T retval;
      asm volatile("rdrand %0":"=r"(retval));
      return retval;
    }

    [[nodiscard]]
    auto operator()() {
      return get();
    }
  };

  inline Random RandomDevice;
}
