#pragma once

#include <bitset>
#include <functional>

struct Keyboard {
  using KeyHandle = char;

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

  using KeyboardEventHandler = std::function<void(KeyboardEvent const &)>;

  struct KeyboardHandle {
    KeyboardHandle(Keyboard &keyboard): keyboard{ keyboard } {

    }

    ~KeyboardHandle() {

    }

  private:
    KeyboardEventHandler lastSuperHandler;
    KeyboardEventHandler lastDefaultHandler;
    Keyboard &keyboard;
  };

private:
  KeyboardEventHandler superHandler;
  KeyboardEventHandler defaultHandler;

  std::bitset<128> pressed;
};
