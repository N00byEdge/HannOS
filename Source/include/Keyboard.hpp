#pragma once

#include <bitset>
#include <functional>

namespace HannOS::Keyboard {
  using KeyHandle = unsigned char;

  constexpr std::uint8_t dataPort = 0x60;
  constexpr std::uint8_t statusPort = 0x64;
  constexpr std::uint8_t commandPort = statusPort;

  // Distinct type relative to Interrupts::Decision to avoid conversions
  enum struct Decision {
    Passthrough,
    Consume,
  };

  struct KeyboardEvent {
    KeyHandle key;
    bool keyUp           : 1;
    bool fnPressed       : 1;
    bool lControlPressed : 1;
    bool rControlPressed : 1;
    bool lAltPressed     : 1;
    bool rAltPressed     : 1;
    bool lSuperPressed   : 1;
    bool rSuperPressed   : 1;
    bool lShiftPressed   : 1;
    bool rShiftPressed   : 1;
  };

  using Handler = std::function<Decision(KeyboardEvent &)>;

  Handler installHandler(Handler &&);

  struct ScopedHandler {
     ScopedHandler(Handler &&handler):
        current{std::move(handler)}
      , prev{installHandler([this](KeyboardEvent &ev) {
          auto res = this->prev(ev);
          if(res == Decision::Passthrough) {
            return current(ev);
          }
        })}
      { }
    ~ScopedHandler() {
      installHandler(std::move(prev));
    }
  private:
    Handler current;
    Handler prev;
  };
}
