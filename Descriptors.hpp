#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "Bitfields.hpp"

// Ordered for little endian
using HannOS::Util::Bitfield;

struct SegmentDescriptor {
  union {
    std::uint32_t lowRepr{};
    Bitfield< 0, 16> limitLower;
    Bitfield<16, 16> baseAddressLower;
  };

  union {
    std::uint32_t highRepr = (1 << 12) | (1 << 15) | (1 << 22);

    Bitfield< 0, 8> baseAddressLowerUpper;
    Bitfield< 8, 1> accessed;
    Bitfield< 9, 1> readwrite;      // Readable if code; writable if data
    Bitfield<10, 1> direction;
    Bitfield<11, 1> executable;
  //Bitfield<12, 1> const;          // Always 1
    Bitfield<13, 2> privilegeRing;
    Bitfield<15, 1> const present;  // Always 1
    Bitfield<16, 4> limitUpper;
  //Bitfield<20, 2> const;          // Always 00
    Bitfield<22, 1> const size;     // size ? 32 bit : 16 bit
    Bitfield<23, 1> granularity;    // granularity ? 4 KiB : 1 byte
    Bitfield<24, 8> baseAddressUpperUpper;
  };

  constexpr std::uint32_t limit() const {
    return limitLower | (limitUpper << limitLower.numBits);
  }

  constexpr void setLimit(std::uint32_t limit) {
    limitLower = limit;
    limitUpper = limit >> limitLower.numBits;
  }

  constexpr std::uint32_t baseAddress() const {
    return baseAddressLower
      | (baseAddressLowerUpper <<
           baseAddressLower.numBits)
      | (baseAddressUpperUpper <<
          (baseAddressLower.numBits + baseAddressLowerUpper.numBits));
  }

  constexpr void setBaseAddress(std::uint32_t baseAddr) {
    baseAddressLower = baseAddr;
    baseAddressLowerUpper = baseAddr >>
       baseAddressLower.numBits;
    baseAddressUpperUpper = baseAddr >>
      (baseAddressLower.numBits + baseAddressLowerUpper.numBits);
  }

private:
  SegmentDescriptor() = default;
};

static_assert(offsetof(SegmentDescriptor, lowRepr) == 0);
static_assert(offsetof(SegmentDescriptor, highRepr) == 4);
static_assert(sizeof(SegmentDescriptor) == 8);

extern SegmentDescriptor globalDescriptorTable[0x100000];

#pragma pack(push, 1)
struct DescriptorReg {
  uint16_t sz = 0xffff;
  union {
    struct {
      uint16_t addrlo;
      uint16_t addrHi;
    };

    SegmentDescriptor *addr = globalDescriptorTable;
  };
};
#pragma pack(pop)

static_assert(offsetof(DescriptorReg, sz) == 0);
static_assert(offsetof(DescriptorReg, addrlo) == 2);
static_assert(offsetof(DescriptorReg, addrHi) == 4);
static_assert(offsetof(DescriptorReg, addr) == 2);
static_assert(sizeof(DescriptorReg) == 6);
static_assert(alignof(DescriptorReg) <= sizeof(DescriptorReg));
