#include "Paging.hpp"
#include "Memory.hpp"

#include <type_traits>
#include <mutex>

#include "Serial.hpp"

namespace HannOS::Paging {
  namespace {
    constexpr auto PagingLevels = 4;

    inline auto *pagingRoot =
      reinterpret_cast<std::array<PageDirectoryEntry<PagingLevels>, PageDirSize> *>(0x1000);

    template<int level>
    constexpr auto index(std::intptr_t val) {
      return (val >> (level * 9 + 3)) & ((1 << 9) - 1);
    }

    template<int level>
    constexpr auto index(PageTableEntry entry) {
      return index<level>(entry.addrBits << 12);
    }

    template<int level>
    constexpr auto index(void *ptr) {
      return index<level>(reinterpret_cast<std::intptr_t>(ptr));
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

    PageTableEntry setMap(std::array<PageTableEntry, PageDirSize> &pta, void *virt, PageTableEntry entry) {
      return std::exchange(pta[index<1>(virt)], entry);
    }

    template<int level>
    PageTableEntry setMap(std::array<PageDirectoryEntry<level>, PageDirSize> &pda, void *virt, PageTableEntry entry) {
      auto &next = pda[index<level>(virt)];
        if(!next.present) {
          // We have to map in a new page table
          next.addrBits = reinterpret_cast<std::intptr_t>(Memory::fetchClearPage().release()) >> 12;
          next.writeEnable = 1;
          next.present = 1;
        }
        return setMap(next.get(), virt, entry);
    }
  }

  PageTableEntry setMap(void *virtAddr, PageTableEntry entry) {
    std::lock_guard<std::mutex> lock{pageWriteMtx};
    return setMap(*pagingRoot, virtAddr, entry);
  }

  void *consumeVirtPages(std::ptrdiff_t num) {
    auto ret   = Memory::consumeVirtSpace(num);
    auto begin = reinterpret_cast<std::intptr_t>(ret);
    auto end   = begin + num * PageSize;

    for(auto addr = begin; addr != end; addr += PageSize) {
      auto page = Memory::fetchPage().release();
      PageTableEntry pte;
      pte.repr = reinterpret_cast<std::intptr_t>(page);
      pte.present = 1;
      pte.writeEnable = 1;
      setMap(reinterpret_cast<void *>(addr), pte);
    }

    return ret;
  }
}
