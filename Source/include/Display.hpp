#pragma once
#include "String.hpp"

#include <cstring>

extern "C" void kernel();

struct Display {
  Display(Display const &) = delete;
  ~Display() = default;

  enum {
    ScreenWidth = 80,
    ScreenHeight = 25,
  };

  void draws(const char *s, char decoration) {
    for(; *s; ++s) {
      draw(*s);
      if(*s != '\n')
        decorate(decoration);
    }
  }

  void draws(const char *s) {
    for(; *s; ++s) {
      draw(*s);
    }
  }

  void draws(std::string_view s) {
    for(auto it = s.begin(); it != s.end(); ++it) {
      draw(*it);
    }
  }

  template<std::size_t sz>
  void draws(std::array<char, sz> const &arr) {
    draws({arr.data(), sz});
  }

  template<typename T>
  void drawi(T val) {
    auto str = to_chars(val);
    draws(str.data());
  }

  template<typename T>
  void drawi(T val, char decoration) {
    auto str = to_chars(val);
    draws(str.data(), decoration);
  }

  template<typename T>
  void drawiarr(T const &arr, std::string_view delim = ", ") {
    auto it = std::begin(arr);
    if (it == std::end(arr)) return;
    while(true) {
      drawi(*it);
      draws(delim);
      if(++it == std::end(arr)) break;
    }
  }

  struct NewLine_T{};
  inline static NewLine_T NewLine;

  template<typename T>
  struct isStdArray: std::false_type { };
  template<std::size_t n>
  struct isStdArray<std::array<char, n>>: std::true_type {};

  template<typename ...Ts>
  void drawvar(Ts &&...vs) {
    auto const f = [&](auto val) {
      if constexpr(std::is_integral_v<std::decay_t<decltype(val)>>)
        return drawi(val);
      if constexpr(std::is_pointer_v<std::decay_t<decltype(val)>>
                || std::is_array_v<std::decay_t<decltype(val)>>
                || isStdArray<std::decay_t<decltype(val)>>::value)
        if constexpr(std::is_same_v<std::decay_t<decltype(val[0])>, char>)
          return draws(val);
      if constexpr(std::is_same_v<decltype(val), NewLine_T>)
        return feedLine();
      //static_assert(!std::is_same_v<decltype(val), decltype(val)>, "LOL FAIL");
    };
    (f(std::forward<Ts>(vs)), ...);
  }

  void clear() {
    drawn = -1;
    for(int y = 0; y < ScreenHeight; ++ y) {
      for (int x = 0; x < ScreenWidth; ++ x) {
        draw(x, y, ' ');
        decorate(x, y, 0x07);
      }
    }
  }

  void setCursor(unsigned x, unsigned y) {
    drawn = x - 1 + y * ScreenWidth;
  }

  void feedLine() {
    drawn += ScreenWidth - drawn % ScreenWidth - 1;
    assertSize();
  }

  void decorate(char val) {
    decorate(drawn, 0, val);
  }

  void draw(char val) {
    if(val == '\n') {
      feedLine();
    }
    else {
      ++drawn;
      assertSize();
      draw(drawn, 0, val);
    }
  }

  static void draw(unsigned x, unsigned y, char val) {
    *(reinterpret_cast<char *>(0xb8000)
      + (x + y * ScreenWidth) * 2) = val;
  }

  static void decorate(unsigned x, unsigned y, char val) {
    *(reinterpret_cast<char *>(0xb8000)
      + (x + y * ScreenWidth) * 2 + 1) = val;
  }

private:
  void assertSize() {
    if(drawn >= ScreenWidth * ScreenHeight) {
      drawn -= ScreenWidth;
      std::memmove(reinterpret_cast<char *>(0xb8000)
                 , reinterpret_cast<char *>(0xb8000) + ScreenWidth * 2
                 , ScreenWidth * (ScreenHeight - 1) * 2);
    }
  }

  int drawn = -1;
  friend void kernel();
  friend struct DisplayHandle;
  Display() {
    clear();
  }
};

struct DisplayHandle {
  DisplayHandle(Display &d) : disp{ d }, lastDrawn{ std::exchange(d.drawn, 0) } {
    std::memcpy(lastView.data(), reinterpret_cast<char *>(0xb8000), Display::ScreenHeight * Display::ScreenWidth * 2);
    d.clear();
  }

  DisplayHandle(DisplayHandle const &h): DisplayHandle(h.disp) { }

  Display &display() const {
    return disp;
  }

  Display &operator()() const {
    return disp;
  }

  ~DisplayHandle() {
    std::memcpy(reinterpret_cast<char *>(0xb8000), lastView.data(), Display::ScreenHeight * Display::ScreenWidth * 2);
    disp.drawn = lastDrawn;
  }

private:
  Display &disp;
  std::array<char, Display::ScreenHeight * Display::ScreenWidth * 2> lastView;
  int lastDrawn;
};
