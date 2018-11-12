#pragma once

#include <cstdint>

namespace HannOS {
  template<typename T, typename Allocator> 
  struct DefaultDeleter: Allocator {
    DefaultDeleter() { }
    DefaultDeleter(Allocator &alloc): Allocator{alloc} { }
    void deallocate(void *ptr, std::intptr_t num) {
      Allocator::deallocate(ptr, num);
    }
  };

  template<typename T, std::intptr_t sz>
  struct BumpAllocator {
    T *allocate(std::intptr_t num) {
      auto ret = buf;
      buf += num;
      return ret;
    }

  private:
    T buf[sz];
  };

  template<typename T, std::intptr_t startAddress>
  struct VoidAllocator {
    static_assert(!(startAddress % alignof(T)), "Invalid alignment of start address");
    T *allocate(std::intptr_t num) {
      auto ret = addr;
      addr += 1;
    }

  private:
    T *addr = launder(reinterpret_cast<T *>(startAddress));
  };

  template<typename T>
  [[nodiscard]] T *launder(T *arg) noexcept {
    static_assert(!(std::is_function<T>::value),
      "Can't launder functions");
    static_assert(!(std::is_same<void, typename std::remove_cv<T>::type>::value),
      "Can't launder cv-void" );
    return arg;
  }
}
