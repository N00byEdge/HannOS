#pragma once

#include <cstdint>

namespace HannOS::CPU {
  struct Random {
    using result_type = std::uint32_t;
    static constexpr result_type min() { return std::numeric_limits<std::uint32_t>::min(); }
    static constexpr result_type max() { return std::numeric_limits<std::uint32_t>::max(); }

    template<typename T = std::uint32_t>
    static auto get() {
      T retval;
      asm("rdrand %0":"=r"(retval));
      return retval;
    }

    int operator()() {
      return get();
    }
  };

  inline Random RandomDevice;
}
