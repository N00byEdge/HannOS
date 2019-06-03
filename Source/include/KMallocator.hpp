#pragma once

namespace HannOS {
  template<typename T, Allocator alloc> 
  struct KMallocator {
    T *allocate(intptr_t num) {
      return kmalloc(num * sizeof(T));
    }

    void deallocate(void *addr, intptr_t num) {
      return kfree(addr, num * sizeof(T));
    }
  }
}
