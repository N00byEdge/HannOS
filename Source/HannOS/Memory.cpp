#include<cstring>
#include<atomic>
#include<mutex>
#include<new>

#include "Memory.hpp"
#include "Containers.hpp"

namespace HannOS::Memory {
  namespace {
    union FreeNode {
      FreeNode *next;
      std::uint8_t size[Paging::PageSize];
    };
    static_assert(sizeof(FreeNode) == Paging::PageSize);
    FreeNode *currNode = nullptr;
    FreeNode *const heapBase = reinterpret_cast<FreeNode *> (0x100000);
    FreeNode *mappedAtStartup = reinterpret_cast<FreeNode *>(0x200000);

    FreeNode *mappedTotal = mappedAtStartup;
    FreeNode *nextNode = heapBase;

    struct Ignore {
      Ignore() { ignoreSize_ = true; }
      ~Ignore() noexcept { ignoreSize_ = false; }
      static bool ignoreSize() { return ignoreSize_; }
    private:
      static inline bool ignoreSize_ = false;
    };

    auto mappedLeft() {
      return mappedTotal - nextNode;
    }

    std::mutex mut;

    // Since paging will fetch pages from us, we have
    // to stay a bit ahead (0x10 pages should be enough)
    // one for each level * 2 - 1, then some headroom
    constexpr auto pagesToKeepAhead = 0x10;
  }

  PageAllocation fetchPage() {
    std::lock_guard<std::mutex> l{mut};
    if(currNode == nullptr) {
      if(!Ignore::ignoreSize() && mappedLeft() < pagesToKeepAhead) {
        Paging::PageTableEntry pte{};
        pte.present = 1;
        pte.writeEnable = 1;
        // Temporarily ignore size of heap as we might call ourselves, but we should
        // be ahead enough (as many pages as it can allocate)
        Ignore size{};
        pte.addrBits = reinterpret_cast<std::intptr_t>(mappedTotal) >> 12;
        Paging::setMap(mappedTotal++, pte);
        // This should have yielded us 512 new pages,
        // while using at most max_levels * 2 - 1, a net gain
      }
      return PageAllocation{nextNode++};
    }
    return std::exchange(currNode, currNode->next);
  }

  void PageAllocation::freePage(void *ptr) {
    if(!ptr) return;
    auto node = reinterpret_cast<FreeNode *>(ptr);
    std::lock_guard<std::mutex> l{mut};
    node->next = currNode;
    currNode = node;
  }

  PageAllocation fetchClearPage() {
    auto ret = fetchPage();
    std::memset(ret.mem(), '\x00', 0x1000);
    return ret;
  }
}
