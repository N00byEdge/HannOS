#include "Display.hpp"

namespace {
  std::intptr_t framebuffer;
  std::int16_t width_;
  std::int16_t height_;
  std::ptrdiff_t pitch;
  std::uint8_t depth;

  // At least multiplication by constants should be faster
  // than multiplication by arbitrary variables
  // Also, the branch predictor should be able to figure this
  // out really quickly and it never changes.
  auto dep(std::intptr_t pos) {
    if(depth == 4)
      return pos * 4;
    else // Assume 3 bytes per pixel
      return pos * 3;
  }

  auto invdep(std::intptr_t num) {
    if(depth == 4)
      return num / 4;
    else
      return num / 3;
  }

  Pixel &get(int x, int y) {
    return *reinterpret_cast<Pixel *>(framebuffer + dep(x + invdep(pitch) * y));
  }
}

#include "Paging.hpp"
#include "Multiboot2.hpp"

namespace HannOS::Multiboot2 {
  void FramebufferInfo::handle() {
    framebuffer = framebuffer_addr;
    width_ = framebuffer_width;
    height_ = framebuffer_height;
    pitch = framebuffer_pitch;
    depth = framebuffer_bpp;
    Serial::varSerialLn("Got display: ", width_, 'x', height_, ", ", depth, " bit depth at ", framebuffer, ", pitch=", pitch);
    if(depth != 24 && depth != 32) {
      Serial::varSerialLn("Invalid bit depth!", depth);
      HannOS::CPU::halt();
    }
    depth /= 8; // I want it in bytes, not bits. Sue me.

    Serial::varSerialLn("Using ", depth, " bytes per pixel");

    Serial::varSerialLn("Identity mapping display");
    auto begin = reinterpret_cast<std::intptr_t>(&get(0,0));
    auto end = reinterpret_cast<std::intptr_t>(&get(width_, height_));

    Paging::alignPageDown(begin);
    Paging::alignPageUp(end);
    for(auto val = begin; val < end; val += Paging::PageSize) {
      auto ptr = reinterpret_cast<void *>(val);
      if(!Paging::virtaddrPresent(ptr)) {
        Paging::PageTableEntry pte;
        pte.repr = val;
        pte.present = 1;
        pte.writeEnable = 1;
        Paging::setMap(ptr, pte);
      } else {
        Serial::varSerialLn("Page already mapped! ", val);
        HannOS::CPU::halt();
      }
    }
  }
}

namespace HannOS::Display {
  void putPixel(int x, int y, Pixel p) {
    auto &px = get(x, y);
    //Serial::varSerialLn("Before putting: ", px.color);
    px = p;
    //Serial::varSerialLn("After putting: ", px.color);
  }

  std::int16_t height() {
    return height_;
  }
  std::int16_t width() {
    return width_;
  }
}
