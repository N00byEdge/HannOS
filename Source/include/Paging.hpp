#pragma once

#include <cstdint>
#include <array>

#include "Bitfields.hpp"

namespace HannOS::Paging {
  using RepT = std::uint64_t;
  template<unsigned startBit, unsigned endBit>
  using PageBits = Util::Bitfield<startBit, endBit, RepT>;
  template<int level>
  struct PageDirectoryEntry;
  struct PageTableEntry;

  constexpr auto PageSize     = 0x1000;
  constexpr auto PageDirSize  = 512;

  static_assert(PageSize == PageDirSize * sizeof(RepT));

  inline auto alignPageDown = [](auto &val) {
    val -= val % Paging::PageSize;
  };

  inline auto alignPageUp = [](auto &val) {
    val += Paging::PageSize - 1;
    alignPageDown(val);
  };

  namespace Impl {
    template<int level> struct PointedTo_    { using type = std::array<PageDirectoryEntry<level - 1>, PageDirSize>; };
    template<>          struct PointedTo_<2> { using type = std::array<PageTableEntry,                PageDirSize>; };

    template<int level>
    using PointedToT = typename PointedTo_<level>::type;
  }

  template<int level>
  struct PageDirectoryEntry {
    union {
      RepT repr;
      PageBits<0, 1> present;
      PageBits<1, 1> writeEnable;
      PageBits<2, 1> userspace;
      PageBits<3, 1> writethrough;
      PageBits<4, 1> cacheDisable;
      PageBits<5, 1> accessed;
    //PageBits<6, 1> zero;
      PageBits<7, 1> pageSize;
    //PageBits<8, 4> unused; // @TODO: Decide how to use
      PageBits<12, 52> addrBits;
    };

    constexpr Impl::PointedToT<level> &get() const {
      return *reinterpret_cast<Impl::PointedToT<level> *>(addrBits << 12);
    }

    constexpr std::int64_t size() const {
      return PageDirSize;
    }
  };

  static_assert(sizeof(PageDirectoryEntry<6>) == sizeof(RepT));
  static_assert(sizeof(PageDirectoryEntry<5>) == sizeof(RepT));
  static_assert(sizeof(PageDirectoryEntry<4>) == sizeof(RepT));
  static_assert(sizeof(PageDirectoryEntry<3>) == sizeof(RepT));
  static_assert(sizeof(PageDirectoryEntry<2>) == sizeof(RepT));

  struct PageTableEntry {
    union {
      RepT repr;
      PageBits<0, 1> present;
      PageBits<1, 1> writeEnable;
      PageBits<2, 1> userspace;
      PageBits<3, 1> writethrough;
      PageBits<4, 1> cacheDisable;
      PageBits<5, 1> accessed;
      PageBits<6, 1> dirty;
    //PageBits<7, 1> zero;
      PageBits<8, 1> global;
    //PageBits<9, 3> unused; // @TODO: Decide how to use
      PageBits<12, 52> addrBits;
    };

    constexpr void *get() const {
      return reinterpret_cast<void *>(addrBits << 12);
    }

    constexpr std::int64_t size() const {
      return PageSize;
    }
  };

  static_assert(sizeof(PageTableEntry) == sizeof(RepT));

  // Is virtual address paged and present
  [[nodiscard]]
  bool virtaddrPresent(void *ptr);

  // Get entry at address
  [[nodiscard]]
  PageTableEntry getMap(void *virtAddr);

  // Returns old page table entry at the same address
  PageTableEntry setMap(void *virtAddr, PageTableEntry entry);

  // Gives you `num` pages of present virtual address space
  void *consumeVirtPages(std::ptrdiff_t num);
}
