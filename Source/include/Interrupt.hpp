#pragma once

namespace HannOS::CPU {
  inline void halt() {
    asm volatile("hlt");
  }
  inline void disableInterrupts() {
    asm volatile("cli");
  }
}

class Interrupts {
	
};
