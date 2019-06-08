#include<cstring>
#include<atomic>
#include<mutex>
#include<new>

#include "Memory.hpp"
#include "CPU.hpp"
#include "Serial.hpp"

namespace HannOS::Memory {
  namespace {
    union FreeNode {
      FreeNode *next;
      std::uint8_t size[Paging::PageSize];
    };
    static_assert(sizeof(FreeNode) == Paging::PageSize);

    std::ptrdiff_t numNodes = 0;
    FreeNode *currNode = nullptr;
    std::mutex mut;

    constexpr std::intptr_t mappedAtStartup = Paging::PageSize * Paging::PageDirSize;
    extern "C" std::intptr_t kernelEnd;
    std::intptr_t heapStart = reinterpret_cast<std::intptr_t>(&kernelEnd);

    void consumePage(void *ptr) {
      auto node = reinterpret_cast<FreeNode *>(ptr);
      std::lock_guard<std::mutex> l{mut};
      ++numNodes;
      node->next = currNode;
      currNode = node;
    }
  }

  PageAllocation fetchPage() {
    std::lock_guard<std::mutex> l{mut};
    --numNodes;
    if(currNode == nullptr)
      return {};
    return std::exchange(currNode, currNode->next);
  }

  void PageAllocation::freePage(void *ptr) {
    consumePage(ptr);
  }

  PageAllocation fetchClearPage() {
    auto ret = fetchPage();
    std::memset(ret.mem(), '\x00', 0x1000);
    return ret;
  }
}

#include "Multiboot2.hpp"

void HannOS::Multiboot2::MemoryMap::handle() {
  Serial::varSerialLn("Mapped at startup: ", Memory::mappedAtStartup);
  Serial::varSerialLn("Statically allocated: ", Memory::heapStart);

  auto consumeMappedPages = [](std::intptr_t begin, std::intptr_t end) {
    Serial::varSerialLn("Consuming mapped pages ", begin, ':', end);
    for(; begin < end; begin += Paging::PageSize)
      Memory::consumePage(reinterpret_cast<void *>(begin));
  };

  auto consumeUnmappedPages = [](std::intptr_t begin, std::intptr_t end) {
    Serial::varSerialLn("Consuming unmapped pages ", begin, ':', end);
    for(; begin < end; begin += Paging::PageSize) {
      // Identity map this page
      Paging::PageTableEntry pte{};
      pte.present = 1;
      pte.writeEnable = 1;
      pte.addrBits = begin >> 12;
      Paging::setMap(reinterpret_cast<void *>(begin), pte);
      Memory::consumePage(reinterpret_cast<void *>(begin));
    }
  };

  auto consume = [&](std::intptr_t begin, std::intptr_t end) {
    Serial::varSerialLn("Range ", begin, ':', end, " available");
    if(begin < Memory::heapStart) {
      begin = Memory::heapStart;
    }

    Paging::alignPageUp(begin);
    Paging::alignPageDown(end);

    if(begin < end) {
      if(begin < Memory::mappedAtStartup && Memory::mappedAtStartup < end) {
        // We need to split this into two parts, one mapped and one unmapped
        consumeMappedPages(begin, Memory::mappedAtStartup);
        consumeUnmappedPages(Memory::mappedAtStartup, end);
      }
      else if(end <= Memory::mappedAtStartup) {
        // Already mapped
        consumeMappedPages(begin, end);
      }
      else {
        // Unmapped
        consumeUnmappedPages(begin, end);
      }
    }
  };

  struct MemoryMapEntry {
    std::intptr_t base_addr;
    std::ptrdiff_t length;
    enum struct Type : std::uint32_t {
      Available     = 1,
      ACPIData      = 3,
      HiberPreserve = 4,
      Defective     = 5,
    };
    static char const *name(Type t) {
      switch(t) {
        case Type::Available:     return " available";
        case Type::ACPIData:      return " APIC reserved";
        case Type::HiberPreserve: return " reserved with hibernation requirement";
        case Type::Defective:     return " reserved";
      }
      return " of unknown type";
    }
    Type type;
    std::uint32_t reserved;
  };
  static_assert(sizeof(MemoryMapEntry) == 24);
  static_assert(offsetof(MemoryMapEntry, base_addr) == 0);
  static_assert(offsetof(MemoryMapEntry, length) == sizeof(MemoryMapEntry::base_addr));
  static_assert(offsetof(MemoryMapEntry, type) == offsetof(MemoryMapEntry, length) + sizeof(MemoryMapEntry::length));

  auto const entryPtrInts = entry_size/sizeof(std::intptr_t);
  auto *curr = reinterpret_cast<std::intptr_t *>(
    reinterpret_cast<char *>(this) + sizeof(*this)
  );
  auto *const end = reinterpret_cast<std::intptr_t *> (
    reinterpret_cast<char *>(this) + size
  );
  for(; curr != end; curr += entryPtrInts) {
    auto *ptr = reinterpret_cast<MemoryMapEntry *>(curr);
    Serial::varSerialLn("Range ", ptr->base_addr, '+', ptr->length, ptr->name(ptr->type));
    switch(ptr->type) {
    case MemoryMapEntry::Type::Available:
      consume(ptr->base_addr, ptr->base_addr + ptr->length);
      break;
    default:
      break;
    }
  }
  Serial::varSerialLn("Finished consuming memory. ", Memory::numNodes, " pages available.");
}
