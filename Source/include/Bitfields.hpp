#pragma once

namespace HannOS::Util {
  #include "Bitfields/Bitfield.hpp"

  template<unsigned startBit, unsigned numBits, typename T = std::uint32_t>
  using Bitfield = Bitfields::Bitfield<startBit, numBits, T>;
}
