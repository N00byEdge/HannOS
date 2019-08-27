#pragma once

#include <bitset>
#include <functional>

struct Keyboard {
  using KeyHandle = unsigned char;

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

  /*
  struct Handler {
    Handler &prev;

  };

  using KeyboardEventHandler
    = std::function<void(KeyboardEvent &)>;
  using SuperHandler
    = std::function<bool(KeyboardEvent &)>;

  void registerSuperHandler(SuperHandler);
  */
};
