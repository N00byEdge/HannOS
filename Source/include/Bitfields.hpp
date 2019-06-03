#pragma once

namespace HannOS::Util {
  #include "Bitfields/Bitfield.hpp"

  template<auto ...T>
  using Bitfield = Bitfields::Bitfield<T...>;
}
