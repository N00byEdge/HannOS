#include<cstdint>

#include "Paging.hpp"

namespace HannOS::Memory {
  struct PageAllocation {
    PageAllocation() = default;
    PageAllocation(void *val): addr{val} { }

    PageAllocation(PageAllocation const &other) = delete;
    PageAllocation(PageAllocation &&other) {
      addr = std::exchange(other.addr, nullptr);
    }

    ~PageAllocation() {
      reset();
    }

    PageAllocation &operator=(PageAllocation const &other) = delete;
    PageAllocation &operator=(PageAllocation &&other) {
      addr = std::exchange(other.addr, nullptr);
      return *this;
    }

    [[nodiscard]]
    void *release() { return std::exchange(addr, nullptr); }

    void *mem() const { return addr; }
    void reset() {
      if(addr) {
        freePage(std::exchange(addr, nullptr));
      }
    }
    std::ptrdiff_t size() { return Paging::PageSize; }

  private:
    void *addr = nullptr;
    static void freePage(void *);
  };

  static_assert(sizeof(PageAllocation) == sizeof(void *));

  [[nodiscard]]
  PageAllocation fetchPage();

  [[nodiscard]]
  PageAllocation fetchClearPage();
}
