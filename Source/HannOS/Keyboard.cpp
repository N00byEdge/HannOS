#include "Keyboard.hpp"
#include "CPU.hpp"

/*namespace {
  Keyboard::KeyboardEventHandler superHandler;
  Keyboard::KeyboardEventHandler defaultHandler;

  Keyboard::KeyboardEvent lastEvent;
  std::bitset<128> pressed;
}*/

bool keyboardInterruptHandler() {
  auto status = HannOS::CPU::inb(HannOS::Keyboard::statusPort);

  if(status & 0x1) { // Omg character
    [[maybe_unused]]
    unsigned char keycode = HannOS::CPU::inb(HannOS::Keyboard::dataPort);
  }

  return true;
}
