#pragma once

namespace HannOS::Util {
  #include "extern/Bitfields/Bitfield.hpp"

  template<auto ...T>
  using Bitfield = Bitfields::Bitfield<T...>;
}
