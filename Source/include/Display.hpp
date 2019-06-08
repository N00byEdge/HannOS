#pragma once

#include <cstdint>

struct Pixel {
  std::uint8_t red, green, blue;
};

namespace HannOS::Display {
  void putPixel(int x, int y, Pixel p);
  std::ptrdiff_t height() __attribute__((const));
  std::ptrdiff_t width() __attribute__((const));
}
