#include "Paging.hpp"
#include "Memory.hpp"

#include <type_traits>
#include <mutex>

namespace HannOS::Paging {
  namespace {
    template<int level>
    constexpr auto index(std::intptr_t val) {
      return (val >> (level * 9 + 3)) & ((1 << 9) - 1);
    }

    template<int level>
    constexpr auto index(PageTableEntry entry) {
      return index<level>(entry.addrBits << 12);
    }

    constexpr bool virtaddrPresent(std::array<PageTableEntry, PageDirSize> const &pta, std::intptr_t val) {
      return pta[index<1>(val)].present;
    }

    template<int level>
    constexpr bool virtaddrPresent(std::array<PageDirectoryEntry<level>, PageDirSize> const &pda, std::intptr_t val) {
      auto &next = pda[index<level>(val)];
      if(!next.present) return false;
      return virtaddrPresent(next.get(), val);
    }
  }

  bool virtaddrPresent(void *ptr) {
    return virtaddrPresent(*pagingRoot, reinterpret_cast<std::intptr_t>(ptr));
  }

  namespace {
    std::mutex pageWriteMtx;

    PageTableEntry setMap(std::array<PageTableEntry, PageDirSize> &pta, PageTableEntry entry) {
      return std::exchange(pta[index<1>(entry)], entry);
    }

    template<int level>
    PageTableEntry setMap(std::array<PageDirectoryEntry<level>, PageDirSize> &pda, PageTableEntry entry) {
      auto &next = pda[index<level>(entry)];
        if(!next.present) {
          // We have to map in a new page table
          next.addrBits = reinterpret_cast<std::intptr_t>(Memory::fetchClearPage().release()) >> 12;
          next.writeEnable = 1;
          next.present = 1;
        }
        return setMap(next.get(), entry);
    }
  }

  PageTableEntry setMap(void *virtAddr, PageTableEntry entry) {
    std::lock_guard<std::mutex> lock{pageWriteMtx};
    return setMap(*pagingRoot, entry);
  }
}
