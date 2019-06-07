#pragma once

#include <string>

#include "Bitfields.hpp"
#include "Interrupt.hpp"

namespace HannOS::CPU {
  template<typename T>
  T in(std::uint16_t port) {
    T retval;
    asm volatile("in %1, %0":"=a"(retval):"Nd"(port));
    return retval;
  }

  template<typename T, std::uint16_t port>
  T in() {
    T retval;
    asm volatile("in %1, %0" : "=a"(retval):"Nd"(port));
  }

  inline auto inb = [](auto port) { return in<std::uint8_t> (port); };
  inline auto inw = [](auto port) { return in<std::uint16_t>(port); };
  inline auto inl = [](auto port) { return in<std::uint32_t>(port); };

  template<typename T>
  void out(std::uint16_t port, T value) {
    asm volatile("out %0, %1"::"a"(value),"Nd"(port));
  }

  template<std::uint16_t port, typename T>
  void out(T value) {
    asm volatile("out %0, %1"::"a"(value),"Nd"(port));
  }

  inline auto outb = [](auto port, auto value) { out<std::uint8_t> (port, value); };
  inline auto outw = [](auto port, auto value) { out<std::uint16_t>(port, value); };
  inline auto outl = [](auto port, auto value) { out<std::uint32_t>(port, value); };

  union CPUIdentifier {
    struct {
      std::uint32_t ebx;
      std::uint32_t edx;
      std::uint32_t ecx;
    };
    std::array<char, 12> chars;
  };
  struct CPUInfo {
    CPUInfo():
      identifier {getVendorIdentifier()}
    , maxFunc    {getMaxFunc()}
    , features   {getFeatures()}

    { }

    CPUIdentifier identifier;
    int maxFunc;

    struct Features {
      union {
        std::uint32_t eax;
        Util::Bitfield<0, 4> stepping;
        Util::Bitfield<4, 4> model;
        Util::Bitfield<8, 4> family;
        Util::Bitfield<12, 2> proctype;
        Util::Bitfield<16, 4> extendedModel;
        Util::Bitfield<20, 8> extendedFamily;
      };

      union {
        std::uint32_t ebx;
        Util::Bitfield<0, 8> BrandIDX;
        Util::Bitfield<8, 8> CLFLUSH_LineSize;
        Util::Bitfield<16, 8> addressableLogicalProcessors;
        Util::Bitfield<24, 8> LocalAPICID;
      };

      union {
        std::uint32_t ecx;
        Util::Bitfield<0, 1> sse3;
        Util::Bitfield<1, 1> pclmulqdq;
        Util::Bitfield<2, 1> dtes64;
        Util::Bitfield<3, 1> monitor;
        Util::Bitfield<4, 1> dscpl;
        Util::Bitfield<5, 1> vmx;
        Util::Bitfield<6, 1> smx;
        Util::Bitfield<7, 1> est;
        Util::Bitfield<8, 1> tm2;
        Util::Bitfield<9, 1> ssse3;
        Util::Bitfield<10, 1> cnxtid;
        Util::Bitfield<11, 1> sdbg;
        Util::Bitfield<12, 1> fma;
        Util::Bitfield<13, 1> cx16;
        Util::Bitfield<14, 1> xtpr;
        Util::Bitfield<15, 1> pdcm;
        Util::Bitfield<17, 1> pcid;
        Util::Bitfield<18, 1> dca;
        Util::Bitfield<19, 1> sse4_1;
        Util::Bitfield<20, 1> sse4_2;
        Util::Bitfield<21, 1> x2apic;
        Util::Bitfield<22, 1> movbe;
        Util::Bitfield<23, 1> popcnt;
        Util::Bitfield<24, 1> tscDeadline;
        Util::Bitfield<25, 1> aes;
        Util::Bitfield<26, 1> xsave;
        Util::Bitfield<27, 1> osxsave;
        Util::Bitfield<28, 1> avx;
        Util::Bitfield<29, 1> f16c;
        Util::Bitfield<30, 1> rdrnd;
        Util::Bitfield<31, 1> hypervisor;
      };

      union {
        std::uint32_t edx;
        Util::Bitfield<0, 1> fpu;
        Util::Bitfield<1, 1> vme;
        Util::Bitfield<2, 1> de;
        Util::Bitfield<3, 1> pse;
        Util::Bitfield<4, 1> tsc;
        Util::Bitfield<5, 1> msr;
        Util::Bitfield<6, 1> pae;
        Util::Bitfield<7, 1> mce;
        Util::Bitfield<8, 1> cx8;
        Util::Bitfield<9, 1> apic;
        Util::Bitfield<11, 1> sep;
        Util::Bitfield<12, 1> mttr;
        Util::Bitfield<13, 1> pge;
        Util::Bitfield<14, 1> mca;
        Util::Bitfield<15, 1> cmov;
        Util::Bitfield<16, 1> pat;
        Util::Bitfield<17, 1> pse36;
        Util::Bitfield<18, 1> psn;
        Util::Bitfield<19, 1> clfsh;
        Util::Bitfield<21, 1> ds;
        Util::Bitfield<22, 1> acpi;
        Util::Bitfield<23, 1> mmx;
        Util::Bitfield<24, 1> fxsr;
        Util::Bitfield<25, 1> sse;
        Util::Bitfield<26, 1> sse2;
        Util::Bitfield<27, 1> ss;
        Util::Bitfield<28, 1> htt;
        Util::Bitfield<29, 1> tm;
        Util::Bitfield<30, 1> ia64;
        Util::Bitfield<31, 1> pbe;
      };
    };

    Features features;

  private:
    static auto getVendorIdentifier() -> CPUIdentifier {
      CPUIdentifier ident;
      asm("cpuid" : "=b"(ident.ebx)
                  , "=c"(ident.ecx)
                  , "=d"(ident.edx)
                  : "0"(0x0)
                  : "eax");
      return ident;
    }

    static auto getFeatures() -> Features {
      Features f;
      asm("cpuid" : "=a"(f.eax)
                  , "=b"(f.ebx)
                  , "=c"(f.ecx)
                  , "=d"(f.edx)
                  : "0"(0x1));
      return f;
    }

    static auto getMaxFunc() -> int {
      int val;
      asm("cpuid" : "=a"(val) : "0"(0x80000000) : "ebx", "ecx", "edx");
      return val;
    }
  };

  inline CPUInfo getCPUID() {
    return {};
  }
}
