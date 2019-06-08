#include "Display.hpp"

namespace {
  Pixel *framebuffer;
  std::ptrdiff_t width_;
  std::ptrdiff_t height_;
  std::ptrdiff_t pitch;
  std::uint8_t depth;

  Pixel &get(int x, int y) {
    return *(framebuffer + x);
  }
}

#include "Paging.hpp"
#include "Multiboot2.hpp"

namespace HannOS::Multiboot2 {
  void FramebufferInfo::handle() {
    framebuffer = reinterpret_cast<Pixel *>(framebuffer_addr);
    width_ = framebuffer_width;
    height_ = framebuffer_height;
    pitch = framebuffer_pitch;
    depth = framebuffer_bpp;
    Serial::varSerialLn("Got display: ", width_, 'x', height_, ", ", depth, " bpc at ", framebuffer, ", pitch=", pitch);

    Serial::varSerialLn("Identity mapping display");
    auto begin = reinterpret_cast<std::intptr_t>(framebuffer);
    auto end = begin + pitch * height_ * 4;

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
    get(x, y) = p;
  }

  std::ptrdiff_t height() {
    return height_;
  }
  std::ptrdiff_t width() {
    return width_;
  }
}
