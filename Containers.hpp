#pragma once

#include <algorithm>

#include "Allocators.hpp"

namespace HannOS {
  struct NoOp {
    void operator()() const { }
  };

  template<typename T, typename Deleter>
  struct UniquePtr: Deleter {
    UniquePtr() { }

    UniquePtr(T *value, Deleter &deleter) noexcept: Deleter{deleter} {
      reset();
      val = value;
    }

    UniquePtr(UniquePtr const &) = delete;
    UniquePtr &operator=(UniquePtr const &) = delete;

    UniquePtr(UniquePtr &&other) noexcept: UniquePtr{other.release()}, Deleter{other.deleter} { }
    UniquePtr &operator=(UniquePtr &&other) noexcept { reset(); val = other.release(); return *this; };

    ~UniquePtr() noexcept { reset(); }

    void reset() noexcept {
      if(val) {
        val = nullptr;
        Deleter::deallocate(val, 1);
      }
    }

    [[nodiscard]] T *release() noexcept {
      auto v = val;
      val = nullptr;
      return v;
    }

    T *get() noexcept { return val; }
    operator bool() const { return val; }
    void swap(UniquePtr &other) noexcept { std::swap(val, other.val); }
    T *operator->() const noexcept { return val; }
    T &operator*() const noexcept { return *val; }

  private:
    T *val = nullptr;
  };

  template<typename T, typename Allocator, typename Deleter, typename ...Ts>
  auto MakeUnique(Allocator &alloc, Deleter &deleter, Ts ...args) {
    UniquePtr<T, Deleter> ptr;
      // Exceptions are currently disabled until further notice.
      // This needs to be reenabled as soon as possible.
      //try {
        auto n = alloc.allocate(1);
        new (n) T(args...);
        ptr = n;
      //} catch(...) {
      //  Deleter::deallocate(ptr.get(), 1);
      //}
    return ptr;
  }

  template<typename T, typename Allocator, typename Deleter = DefaultDeleter<T, Allocator>>
  struct ForwardList: Deleter {
    using value_type = T;
    struct Node {
      UniquePtr<Node, Deleter> next;
      T value;
    };

    struct Iterator {
      Node *n;

      Iterator &operator++() {
        n = n->next.get();
        return *this;
      }

      T &operator*() const {
        return n->value;
      }

      T *operator->() const {
        return n->value;
      }
    };

    template<typename ... Ts>
    Iterator emplaceFront(Ts ...vals) {
      auto ptr = MakeUnique<Node>(*this, *this, std::move(head), vals...);
      head = std::move(ptr);
    }

    void popFront() { head = std::move(head->next); }
    Iterator begin() const { return head.get(); }
    Iterator end() const { return nullptr; }
    bool empty() const { return !head; }
    T extractFront() {
      T ret = std::move(head->value);
      head = std::move(head->next);
      return ret;
    }
  private:
    UniquePtr<Node, Deleter> head;
  };

  template<typename T, std::intptr_t capacity>
  struct Buffer {
    void popBack() {
      buf[--size_].~T();
    }

    template<typename ...Ts>
    void emplaceBack(Ts... args) {
      new (buf[size_++]) T(args...);
    }

    std::intptr_t size() {
      return size_;
    }

    void erase(std::intptr_t at) {
      buf[at].~T();
      std::move(&buf[at + 1], &buf[size_--], &buf[at]);
    }

    void erase(std::intptr_t from, std::intptr_t to) {
      for(std::intptr_t at = from; at < to; ++ at)
        buf[at].~T();
      std::move(&buf[to], &buf[size_], &buf[from]);
      size_ -= (to - from);
    }

    T &operator[](std::intptr_t at) {
      return buf[at];
    }

    T *begin() { return buf; }
    T *end() { return &buf[size_]; }

  private:
    T buf[capacity];
    std::intptr_t size_ = 0;
  };
}
