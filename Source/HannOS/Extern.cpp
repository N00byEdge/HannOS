#include <cstdint>
#include <array>

namespace {
  template<typename T>
  using charLut = std::array<T, 256>;

  template<typename F>
  constexpr auto makeCharLut(F const &f) {
    charLut<decltype(f(std::declval<char>()))> result{};
    for(int i = 0; i < 256; ++ i) {
      result[i] = f(i);
    }
    return result;
  }
}

extern "C"
void *memset(void *ptr, int value, std::size_t num) {
  auto aptr = reinterpret_cast<char *>(ptr);
  while(num --> 0) {
    *aptr++ = value;
  }
  return ptr;
}

extern "C"
void *memmove(void *dest, void const *src, std::size_t n) {
  if((src == dest) || !n) return dest;
  auto adest = reinterpret_cast<char *>(dest);
  auto asrc = reinterpret_cast<char const *>(src);

  if(adest < asrc) {
    while(n --> 0) {
      *adest++ = *asrc++;
    }
  }
  else {
    adest += n - 1;
    asrc += n - 1;
    while(n --> 0) {
      *adest-- = *asrc--;
    }
  }

  return dest;
}

extern "C"
void *memcpy(void *dest, void const *src, std::size_t n) {
  return memmove(dest, src, n);
}

extern "C"
std::size_t strlen(char const *str) {
  std::size_t len = 0;

  while(*str) {
    ++str;
    ++len;
  }

  return len;
}

void *operator new(std::size_t) {
  return reinterpret_cast<void *>(0x40);
}

void operator delete(void *ptr) noexcept {
  if(ptr) {

  }
}

extern "C"
void memswp(void *a, void *b, std::size_t n) {
  if((a == b) | !n) return;

  auto const swpptr = [](auto &aptr, auto &bptr) {
    *aptr ^= *bptr;
    *bptr ^= *aptr;
    *aptr ^= *bptr;
    ++aptr, ++bptr;
  };

  auto ap = reinterpret_cast<std::uint32_t *>(a);
  auto bp = reinterpret_cast<std::uint32_t *>(b);

  while(n > 3) {
    n-= 4;
    swpptr(ap, bp);
  }

  auto apb = reinterpret_cast<std::uint8_t *>(ap);
  auto bpb = reinterpret_cast<std::uint8_t *>(bp);

  while(n) {
    -- n;
    swpptr(apb, bpb);
  }
}

#include "CPU.hpp"
#include "Serial.hpp"

namespace std {
  void __throw_system_error(int val) {
    HannOS::Serial::varSerialLn("Fatal system error: ", val, ". Halting.");
    HannOS::CPU::halt();
  }

  void __throw_out_of_range_fmt(char const *fmt, ...) {
    HannOS::Serial::varSerialLn("Out of range error: ", fmt, ". That's a format string but I'm not touching that shit.");
    HannOS::CPU::halt();
  }
}

extern "C"
int memcmp(char const *lhs, char const *rhs, std::size_t num) {
  while(num--) {
    int diff = *lhs - *rhs;
    if(diff)
      return diff;
  }
  return 0;
}

extern "C"
char const *memchr(char const *ptr, int value, std::size_t num) {
  for(; num--; ++ptr) if(*ptr == value)
      return ptr;
  return nullptr;
}

namespace {
  constexpr auto isdigitLut = makeCharLut([](unsigned char val) constexpr -> bool {
    if('0' <= val && val <= '9')
      return true;
    else return false;
  });
}

extern "C"
int isdigit(int val) {
  return isdigitLut[val];
}

#include "Paging.hpp"

extern "C"
void *aquireLorgeStack() {
  constexpr auto numPages = 50;
  auto stackBase = reinterpret_cast<char *>(HannOS::Paging::consumeVirtPages(numPages));
  auto stackEnd = HannOS::Paging::PageSize * numPages + stackBase - 8;
  return stackEnd;
}
