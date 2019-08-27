#pragma once

namespace HannOS::Util {
  #include "Bitfields/Bitfield.hpp"

  template<unsigned startBit, unsigned numBits, typename T = std::uint32_t>
  using Bitfield = Bitfields::Bitfield<startBit, numBits, T>;

  template<int numSourceBits, typename T>
  T signExtend(T val) {
    struct { T x:numSourceBits; } s;
    return s.x = val;
  }
}
