#pragma once

#include <cstdint>

struct Pixel {
  std::uint8_t red, green, blue;
};

namespace HannOS::Display {
  void putPixel(int x, int y, Pixel p);
  std::int16_t height() __attribute__((pure));
  std::int16_t width() __attribute__((pure));
}
