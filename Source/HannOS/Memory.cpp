#include<cstring>
#include<atomic>
#include<mutex>
#include<new>

#include "Containers.hpp"

namespace {
  // Allocation only in 0x1000 / 4k chunks
  static constexpr auto chunkSize = 0x1000;
  union FreeNode {
    FreeNode *next;
    char size[chunkSize];
  };
  static_assert(sizeof(FreeNode) == chunkSize);
  FreeNode *currNode = nullptr;
  FreeNode *const heapBase = reinterpret_cast<FreeNode *>(0x500000);
  FreeNode *nextNode = heapBase;
  std::atomic<std::intptr_t> freeListSz = 0; //***Only for measuring!!***
  std::mutex mut;
}

namespace HannOS {
  void *kMalloc() {
    std::lock_guard<std::mutex> l{mut};
    if(currNode == nullptr) return nextNode++;
    --freeListSz;
    return std::exchange(currNode, currNode->next);
  }

  void kFree(void *ptr) {
    if(!ptr) return;
    ++freeListSz;
    auto node = reinterpret_cast<FreeNode *>(ptr);
    std::lock_guard<std::mutex> l{mut};
    node->next = currNode;
    currNode = node;
  }

  void *kCalloc() {
    return std::memset(kMalloc(), '\x00', 0x1000);
  }

  std::intptr_t allocated() {
    return (nextNode - heapBase - freeListSz.load(std::memory_order_relaxed)) * chunkSize;
  }
}
