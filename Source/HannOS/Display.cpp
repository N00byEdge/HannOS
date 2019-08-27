#include "Display.hpp"

#include <cstring>

namespace {
  std::intptr_t frontBuffer;
  std::intptr_t backBuffer;
  std::intptr_t bufSize;
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

  HannOS::Display::Pixel &get(int x, int y) {
    return *reinterpret_cast<HannOS::Display::Pixel *>(backBuffer + dep(x + pitch * y));
  }
}

#include "Paging.hpp"
#include "Multiboot2.hpp"

namespace HannOS::Multiboot2 {
  void FramebufferInfo::handle() {
    frontBuffer = framebuffer_addr;
    width_ = framebuffer_width;
    height_ = framebuffer_height;
    pitch = framebuffer_pitch;
    depth = framebuffer_bpp;
    Serial::varSerialLn("Got display: ", width_, 'x', height_, ", ", depth, " bit depth at ", frontBuffer, ", pitch=", pitch);
    if(depth != 24 && depth != 32) {
      Serial::varSerialLn("Invalid bit depth!", depth);
      HannOS::CPU::halt();
    }
    depth /= 8; // I want it in bytes, not bits. Sue me.
    pitch /= depth; // I want this in pixels.

    Serial::varSerialLn("Using ", depth, " bytes per pixel");

    Serial::varSerialLn("Identity mapping display");

    auto begin = frontBuffer;
    bufSize    = dep(width_ + pitch * height_);
    auto end   = begin + bufSize;

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

    Serial::varSerialLn("Setting up back buffer");
    auto numPages = (end - begin) / Paging::PageSize;
    auto const backBuf = Paging::consumeVirtPages(numPages);
    backBuffer =
      reinterpret_cast<std::intptr_t>(backBuf);
    Display::clear();
    Serial::varSerialLn("Back buffer at virtual address ", backBuf);
  }
}

namespace HannOS::Display {
  void putPixel(int x, int y, Pixel p) {
    get(x, y) = p;
  }

  std::int16_t height() {
    return height_;
  }
  std::int16_t width() {
    return width_;
  }

  void swapBuffers() {
    std::memcpy(
      reinterpret_cast<void *>(frontBuffer),
      reinterpret_cast<void *>(backBuffer),
      bufSize
    );
  }

  void clear(Pixel p) {
    for(auto y = 0; y < height(); ++y) for(auto x = 0; x < width(); ++ x)
      get(x, y) = p;
  }
}
