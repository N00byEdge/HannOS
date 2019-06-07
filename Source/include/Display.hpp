#pragma once
#include "String.hpp"

#include <cstring>

extern "C" void kernel();

namespace HannOS {
  struct Display {
    Display() = default;
    Display(Display const &) = delete;
    Display(Display &&) = delete;
    Display &operator=(Display const &) = delete;
    Display &operator=(Display &&) = delete;

    enum {
      ScreenWidth = 80,
      ScreenHeight = 25,
    };

    static void draws(const char *s, char decoration) {
      for(; *s; ++s) {
        draw(*s);
        if(*s != '\n')
          decorate(decoration);
      }
    }

    static void draws(const char *s) {
      for(; *s; ++s) {
        draw(*s);
      }
    }

    static void draws(std::string_view s) {
      for(auto it = s.begin(); it != s.end(); ++it) {
        draw(*it);
      }
    }

    template<std::size_t sz>
    static void draws(std::array<char, sz> const &arr) {
      draws({arr.data(), sz});
    }

    template<typename T>
    static void drawi(T val) {
      auto str = to_chars(val);
      draws(str.data());
    }

    template<typename T>
    static void drawi(T val, char decoration) {
      auto str = to_chars(val);
      draws(str.data(), decoration);
    }

    template<typename T>
    static void drawiarr(T const &arr, std::string_view delim = ", ") {
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

    /* @TODO: We need to be able to decorate drawvar
    display.drawvar(Display::NewLine, Display::NewLine, Display::Decorate{0x1f}
      ":(", Display::NewLine,
      "Oopsie Whoopsie! Uwu we made a fucky wucky!! "
      "A wittle fucko boingo! The code monkeys at our "
      "headquarters are working VEWY HAWD to fix this!", Display::NewLine
    );
    */
    template<typename ...Ts>
    static void drawvar(Ts &&...vs) {
      auto const f = [&](auto val) {
        if constexpr(std::is_same_v<std::decay_t<decltype(val)>, char>)
          return draw(val);
        if constexpr(std::is_integral_v<std::decay_t<decltype(val)>>)
          return drawi(val);
        if constexpr(std::is_same_v<std::decay_t<decltype(val)>, void *>)
          return drawi(reinterpret_cast<std::intptr_t>(val));
        if constexpr((std::is_pointer_v<std::decay_t<decltype(val)>>
                  && !std::is_same_v<std::decay_t<decltype(val)>, void *>)
                  || std::is_array_v<std::decay_t<decltype(val)>>
                  || isStdArray<std::decay_t<decltype(val)>>::value)
        {
          if constexpr(std::is_same_v<std::decay_t<decltype(val[0])>, char>)
            return draws(val);
          else
            return drawi(reinterpret_cast<std::intptr_t>(val));
        }
        if constexpr(std::is_same_v<decltype(val), NewLine_T>)
          return feedLine();
        //static_assert(!std::is_same_v<decltype(val), decltype(val)>, "LOL FAIL");
      };
      (f(std::forward<Ts>(vs)), ...);
    }

    static void clear() {
      drawn = -1;
      for(int y = 0; y < ScreenHeight; ++ y) {
        for (int x = 0; x < ScreenWidth; ++ x) {
          draw(x, y, ' ');
          decorate(x, y, 0x07);
        }
      }
    }

    static void setCursor(unsigned x, unsigned y) {
      drawn = x - 1 + y * ScreenWidth;
    }

    static void feedLine() {
      drawn += ScreenWidth - drawn % ScreenWidth - 1;
      assertSize();
    }

    static void decorate(char val) {
      decorate(drawn, 0, val);
    }

    static void draw(char val) {
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
    static void assertSize() {
      if(drawn >= ScreenWidth * ScreenHeight) {
        drawn -= ScreenWidth;
        std::memmove(reinterpret_cast<char *>(0xb8000)
                   , reinterpret_cast<char *>(0xb8000) + ScreenWidth * 2
                   , ScreenWidth * (ScreenHeight - 1) * 2);
      }
    }

    inline static int drawn = -1;
    friend struct DisplayHandle;
  };

  static inline Display ActiveDisplay{[]() -> Display {
    Display::clear();
    return {};
  }()};

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
}
