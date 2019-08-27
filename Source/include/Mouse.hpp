#pragma once

#include <cstdint>

#include "Bitfields.hpp"
#include "Serial.hpp"

namespace HannOS::Mouse {
  namespace Impl {
    // Initialize mouse at global startup
    inline auto const mouse = []() {
      HannOS::Serial::varSerial("Initializing mouse.\n");
    }();
  }
}