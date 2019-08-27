#pragma once

#include <cstdint>

namespace HannOS::Display {
  struct Pixel {
    std::uint8_t red, green, blue;
  };

  void putPixel(int x, int y, Pixel p);
  std::int16_t height() __attribute__((pure));
  std::int16_t width() __attribute__((pure));
  void swapBuffers();
  void clear(Pixel p = Pixel{0, 0, 0});
}
