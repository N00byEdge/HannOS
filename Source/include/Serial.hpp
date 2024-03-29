#pragma once

#include "CPU.hpp"
#include "String.hpp"

namespace HannOS::Serial {
  template<int port>
  struct Serial {
  private:
    using T = char;
    static constexpr int hwport() {
      if constexpr(port == 1) {
        return 0x3f8;
      }
      else if constexpr(port == 2) {
        return 0x2f8;
      }
      else if constexpr(port == 3) {
        return 0x3e8;
      }
      else if constexpr(port == 4) {
        return 0x2e8;
      }
      else {
        static_assert(port != port, "Invalid port number");
      }
    }
  public:
    Serial() {
      CPU::out<hwport() + 1>('\x00');
      CPU::out<hwport() + 3>('\x80');
      CPU::out<hwport() + 0>('\x01');
      CPU::out<hwport() + 1>('\x00');
      CPU::out<hwport() + 3>('\x03');
      CPU::out<hwport() + 2>('\xC7');
    }

    static bool canSend() { return CPU::in<T, hwport() + 5>() & 0x20; }
    static void write(char c) {
      if(!c) return;
      while(!canSend()) __asm__("pause");
      CPU::out<hwport()>(c);
    }
    static bool hasData() { return CPU::in<T, hwport() + 5>() & 0x01; }
    static char read() {
      while(!hasData()) __asm__("pause");
      return CPU::in<T, hwport()>();
    }

    static void writes(char const *s) {
      for(; *s; ++s)
        write(*s);
    }

    template<typename T>
    static void writes(T const &arr) {
      for(auto c: arr)
        write(c);
    }

    template<typename T>
    static void writei(T const &val) {
      writes(String::to_chars<true>(val));
    }
  };

  static inline Serial<1> serial1;
  static inline Serial<2> serial2;
  static inline Serial<3> serial3;
  static inline Serial<4> serial4;

  template<typename T>
  struct isStdArray: std::false_type { };
  template<std::size_t n>
  struct isStdArray<std::array<char, n>>: std::true_type {};

  template<typename T>
  struct isStdString: std::false_type { };
  template<>
  struct isStdString<std::string_view>: std::true_type {};

  template<int port = 1, typename ...Ts>
  static void varSerial(Ts &&...vs) {
    using com = Serial<port>;
    auto const f = [&](auto val) {
      if constexpr(std::is_same_v<std::decay_t<decltype(val)>, char>)
        return com::write(val);
      if constexpr(std::is_integral_v<std::decay_t<decltype(val)>>)
        return com::writei(val);
      if constexpr(std::is_same_v<std::decay_t<decltype(val)>, void *>)
        return com::writei(reinterpret_cast<std::intptr_t>(val));
      if constexpr((std::is_pointer_v<std::decay_t<decltype(val)>>
                && !std::is_same_v<std::decay_t<decltype(val)>, void *>)
                || std::is_array_v<std::decay_t<decltype(val)>>
                || isStdArray<std::decay_t<decltype(val)>>::value
                || isStdString<std::decay_t<decltype(val)>>::value)
      {
        if constexpr(std::is_same_v<std::decay_t<decltype(val[0])>, char>
                  || isStdString<std::decay_t<decltype(val)>>::value)
          return com::writes(val);
        else if constexpr(std::is_array_v<std::decay_t<decltype(val)>>
                       || isStdArray<std::decay_t<decltype(val)>>::value)
          com::writeiarr(val, " ");
        else
          return com::writei(reinterpret_cast<std::intptr_t>(val));
      }
    };
    (f(std::forward<Ts>(vs)), ...);
  }

  template<int port = 1, typename ...Ts>
  static void varSerialLn(Ts &&...vs) {
    varSerial(std::forward<Ts>(vs)..., '\n');
  }

  template<int port = 1>
  static char read() {
    return Serial<port>::read();
  }
}
