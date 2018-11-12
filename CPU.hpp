#pragma once

#include <string>

#include "Bitfields.hpp"

namespace HannOS::CPU {
  using CPUIdentifier = std::array<char, 12>;
  struct CPUInfo {
    CPUInfo():

    identifier {
      [&] {
        CPUIdentifier id;
        getVendorIdentifier(id);
        return id;
      }()
    }

    , maxFunc{getMaxFunc()}

    , features{
      [&]{
        Features feat;
        getFeatures(feat);
        return feat;
      }()
    }

    { }

    CPUIdentifier identifier;
    int maxFunc;

    struct Features {
      union {
        std::uint32_t eax;
        Bitfield<0, 4> stepping;
        Bitfield<4, 4> model;
        Bitfield<8, 4> family;
        Bitfield<12, 2> proctype;
        Bitfield<16, 4> extendedModel;
        Bitfield<20, 8> extendedFamily;
      };

      union {
        std::uint32_t ebx;
        Bitfield<0, 8> BrandIDX;
        Bitfield<8, 8> CLFLUSH_LineSize;
        Bitfield<16, 8> addressableLogicalProcessors;
        Bitfield<24, 8> LocalAPICID;
      };

      union {
        std::uint32_t ecx;
        Bitfield<0, 1> sse3;
        Bitfield<1, 1> pclmulqdq;
        Bitfield<2, 1> dtes64;
        Bitfield<3, 1> monitor;
        Bitfield<4, 1> dscpl;
        Bitfield<5, 1> vmx;
        Bitfield<6, 1> smx;
        Bitfield<7, 1> est;
        Bitfield<8, 1> tm2;
        Bitfield<9, 1> ssse3;
        Bitfield<10, 1> cnxtid;
        Bitfield<11, 1> sdbg;
        Bitfield<12, 1> fma;
        Bitfield<13, 1> cx16;
        Bitfield<14, 1> xtpr;
        Bitfield<15, 1> pdcm;
        Bitfield<17, 1> pcid;
        Bitfield<18, 1> dca;
        Bitfield<19, 1> sse4_1;
        Bitfield<20, 1> sse4_2;
        Bitfield<21, 1> x2apic;
        Bitfield<22, 1> movbe;
        Bitfield<23, 1> popcnt;
        Bitfield<24, 1> tscDeadline;
        Bitfield<25, 1> aes;
        Bitfield<26, 1> xsave;
        Bitfield<27, 1> osxsave;
        Bitfield<28, 1> avx;
        Bitfield<29, 1> f16c;
        Bitfield<30, 1> rdrnd;
        Bitfield<31, 1> hypervisor;
      };

      union {
        std::uint32_t edx;
        Bitfield<0, 1> fpu;
        Bitfield<1, 1> vme;
        Bitfield<2, 1> de;
        Bitfield<3, 1> pse;
        Bitfield<4, 1> tsc;
        Bitfield<5, 1> msr;
        Bitfield<6, 1> pae;
        Bitfield<7, 1> mce;
        Bitfield<8, 1> cx8;
        Bitfield<9, 1> apic;
        Bitfield<11, 1> sep;
        Bitfield<12, 1> mttr;
        Bitfield<13, 1> pge;
        Bitfield<14, 1> mca;
        Bitfield<15, 1> cmov;
        Bitfield<16, 1> pat;
        Bitfield<17, 1> pse36;
        Bitfield<18, 1> psn;
        Bitfield<19, 1> clfsh;
        Bitfield<21, 1> ds;
        Bitfield<22, 1> acpi;
        Bitfield<23, 1> mmx;
        Bitfield<24, 1> fxsr;
        Bitfield<25, 1> sse;
        Bitfield<26, 1> sse2;
        Bitfield<27, 1> ss;
        Bitfield<28, 1> htt;
        Bitfield<29, 1> tm;
        Bitfield<30, 1> ia64;
        Bitfield<31, 1> pbe;
      };
    };

    Features features;

  private:
    __attribute__ ((naked))
    void getVendorIdentifier(CPUIdentifier &) {
      asm(R"(
        xor %eax, %eax
        cpuid
        movl 4(%esp), %eax
        movl %ebx, (%eax)
        movl %edx, 4(%eax)
        movl %ecx, 8(%eax)
        ret
      )");
    }

    __attribute__ ((naked))
    void getFeatures(Features &) {
      asm(R"(
        push %rbx
        movl $1, %eax
        cpuid
        push %rax
        movl 16(%esp), %eax
        movl %ebx, 4(%eax)
        movl %ecx, 8(%eax)
        movl %edx, 12(%eax)
        pop %rbx
        movl %ebx, (%eax)
        pop %rbx
        ret
      )");
    }


    __attribute__ ((naked))
    inline int getMaxFunc() {
      asm(R"(
        movl $0x80000000, %eax
        cpuid
        ret
      )");
    }
  };

  inline CPUInfo getCPUID() {
    return {};
  }
}
