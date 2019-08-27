#include "Descriptors.hpp"
#include "Serial.hpp"
#include "CPU.hpp"
#include "Memory.hpp"
#include "Random.hpp"
#include "Paging.hpp"
#include "Display.hpp"

#include <algorithm>
#include <sstream>
#include <cstdio>
#include <random>
#include <algorithm>

using namespace std::string_view_literals;

void primes() {
  constexpr unsigned mprime = 64*1024;
  char nonprime[mprime/2]{0, 1, 0};
  for(unsigned i = 3; i < mprime; i += 2) if(!nonprime[i / 2]) {
    HannOS::Serial::varSerialLn(i);
    for(unsigned j = i * i; j < mprime; j += i)
      nonprime[j / 2] = 1;
  }
}

enum struct Direction {
  Up, Down, Left, Right,
};

enum struct TileGraphics {
  Snek, Head, Candy, None,
};

void snek() {
  Direction going = Direction::Right;

  struct Tile {
    int x, y;
    bool snek;
    bool candy;
    Tile *towardsHead;
    TileGraphics graphics;
  };

  std::array<Tile, 40 * 40> tiles;

  Tile *tail;
  Tile *head;

  auto tile = [&](int x, int y) -> Tile & {
    return tiles[(x % 40) * 40 + y % 40];
  };

  auto setTile = [&](Tile &tile, TileGraphics graphics) -> void {
    if(std::exchange(tile.graphics, graphics) != graphics) {
      HannOS::Display::Pixel color;

      switch(graphics) {
      case TileGraphics::Snek:
        color = HannOS::Display::Pixel{0, 255, 0};
        break;
      case TileGraphics::Head:
        color = HannOS::Display::Pixel{255, 255, 255};
        break;
      case TileGraphics::Candy:
        color = HannOS::Display::Pixel{0, 0, 255};
        break;
      case TileGraphics::None:
        color = HannOS::Display::Pixel{0, 0, 0};
        break;
      default:
        break;
      }

      for(int x = tile.x * 14 + 1; x < (tile.x + 1) * 14; ++ x) for(int y = tile.y * 14 + 1; y < (tile.y + 1) * 14; ++ y) {
        HannOS::Display::putPixel(x, y, color);
      }
    }
  };

  auto genCandy = [&]() -> void {
    std::uniform_int_distribution<int> dist{0, 39};
    int x, y;
    do {
      x = dist(HannOS::RandomDevice);
      y = dist(HannOS::RandomDevice);
    } while(tile(x, y).snek);

    setTile(tile(x, y), TileGraphics::Candy);
    tile(x, y).candy = true;
  };

  int length = 2;

  // Do setup
  for(int x = 0; x < 40; ++ x) for(int y = 0; y < 40; ++ y) {
    auto &t = tile(x, y);
    t.x = x;
    t.y = y;
    t.snek = false;
    t.candy = false;
    setTile(tile(x, y), TileGraphics::None);

    for(int dx = 0; dx < 15; ++ dx) {
      HannOS::Display::putPixel(x * 14 + dx, y * 14, {40, 40, 40});
      HannOS::Display::putPixel(x * 14 + dx, (y + 1) * 14, {40, 40, 40});
    }
    for(int dy = 0; dy < 15; ++ dy) {
      HannOS::Display::putPixel(x * 14, y * 14 + dy, {40, 40, 40});
      HannOS::Display::putPixel((x + 1) * 14, y * 14 + dy, {40, 40, 40});
    }
  }

  head = &tile(5, 20);
  head->snek = true;
  setTile(*head, TileGraphics::Head);

  tail = &tile(4, 20);
  tail->snek = true;
  tail->towardsHead = head;
  setTile(*tail, TileGraphics::Snek);

  genCandy();

  while(1) {
    std::array<char, 3> buf;
    auto it = buf.begin();
    for(volatile int i = 0; i < 30000000/(length + 10); ++ i) {
      if(it == buf.end())
        continue;
      if(HannOS::Serial::Serial<1>::hasData())
        *it++ = HannOS::Serial::Serial<1>::read();
    }
    if(it == buf.end()) {
      if(buf[0] == 0x1b && buf[1] == 0x5b) switch(buf[2]) {
      case 0x41: // Up
        if(going != Direction::Down)
          going = Direction::Up;
        break;
      case 0x42: // Down
        if(going != Direction::Up)
          going = Direction::Down;
        break;
      case 0x43: // Right
        if(going != Direction::Left)
          going = Direction::Right;
        break;
      case 0x44: // Left
        if(going != Direction::Right)
          going = Direction::Left;
        break;
      default: break;
      }
    }

    int x = head->x;
    int y = head->y;

    switch(going) {
      case Direction::Up:
        --y;
        break;
      case Direction::Down:
        ++y;
        break;
      case Direction::Right:
        ++x;
        break;
      case Direction::Left:
        --x;
        break;
      default:
        break;
    }
    if(x < 0)
      x = 39;
    else if(x > 39)
      x = 0;
    if(y < 0)
      y = 39;
    else if(y > 39)
      y = 0;
    bool ateCandy = false;
    if(tile(x, y).snek) {
      HannOS::Serial::varSerialLn("Game over, length: ", length);
      HannOS::Display::swapBuffers();
      return;
    }
    else if(std::exchange(tile(x, y).candy, false)) {
      HannOS::Serial::varSerialLn("Length: ", ++length);
      ateCandy = true;
    }

    // Advance head
    auto nextHead = &tile(x, y);
    setTile(*head, TileGraphics::Snek);
    head->towardsHead = nextHead;
    head->snek = true;
    head = nextHead;
    setTile(*head, TileGraphics::Head);

    if(!ateCandy) {
      tail->snek = false;
      setTile(*tail, TileGraphics::None);
      tail = tail->towardsHead;
    } else {
      genCandy();
    }

    HannOS::Display::swapBuffers();
  }
}

/*void printbenchmark(HannOS::VGAHandle display) {
  for(int i = 0; i < 0x10000; ++ i) {
    display().setCursor(0, 0);
    display().drawi(i);
  }
}*/

/*void memeditor(HannOS::VGAHandle disp) {
  //int *dummy = nullptr;
  //auto base = reinterpret_cast<int *>(&dummy);
  auto base = reinterpret_cast<std::uint32_t *>(0xb8000);
  int direction = 1;

  while (true) {
    auto addr = base;
    disp().setCursor(0, 0);
    disp().draws("Editing ");
    disp().drawi(reinterpret_cast<std::size_t>(base));
    for (int y = 1; y < HannOS::VGA::ScreenHeight; ++y) {
      disp().setCursor(0, y);
      disp().drawi(reinterpret_cast<std::size_t>(addr));
      disp().draws(":");
      for (int x = 2; x < 6; ++x) {
        disp().setCursor(x * 9, y);
        auto val = to_chars(*addr);
        addr += direction;
        disp().draws(&std::get<0>(val));
      }
    }
  }
}*/

/*void CMatrixPP(HannOS::VGAHandle disp) {
  disp().clear();
  struct Bunch {
    int start = std::uniform_int_distribution<int>
      {0, HannOS::VGA::ScreenHeight}
      (HannOS::RandomDevice);
    int end = HannOS::VGA::ScreenHeight;
  };
  std::array<Bunch, HannOS::VGA::ScreenWidth> bunches;
  auto updateBunch = [&](Bunch &bunch, int x) {
    if(bunch.start == HannOS::VGA::ScreenHeight) {
        // chance to start over
        if(std::uniform_int_distribution<int>{0, 25}(HannOS::RandomDevice)) {
          return;
        } else {
          bunch.end = 0;
          bunch.start = bunch.end - std::uniform_int_distribution<int>
                                    {11, 24}
                                    (HannOS::RandomDevice);
        }
      }
      // chance to update
      if (!std::uniform_int_distribution<int>{0, 16}(HannOS::RandomDevice))
        return;
      // Update the bunch
      if(bunch.end > 0 && bunch.end <= HannOS::VGA::ScreenHeight) {
        disp().decorate(x, bunch.end - 1, 0x0a); // Color character drawn last update green
      }
      if(++bunch.end <= HannOS::VGA::ScreenHeight) {
        // Make new character
        char sample;
        constexpr auto chars = ""sv
            "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
            "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f";
        std::sample(chars.begin(), chars.end(), &sample, 1, HannOS::RandomDevice);
        disp().decorate(x, bunch.end - 1, 0x0f); // Color character white
        disp().draw(x, bunch.end - 1, sample);   // Set to the sampled character
      }
      if(bunch.start++ < HannOS::VGA::ScreenHeight && bunch.start > 0) {
        disp().draw(x, bunch.start - 1, 0x20); // Clear trail
      }
  };

  while(1) {
    for(int i = 0; i < HannOS::VGA::ScreenWidth; ++ i)
      updateBunch(bunches[i], i);
    for(volatile int i = 0; i < 10000000; ++ i);
  }
  HannOS::CPU::halt();
}*/

void rect(std::string_view argv) {
  int argc;
  auto [command, x, y, height, width, r, g, b, rest] = HannOS::String::split<9>(argv, ' ', argc);
  if(argc < 8) {
    HannOS::Serial::varSerialLn("Usage: ", command, " x y height width r g b");
    return;
  }

  int xval, yval, heightval, widthval;
  std::uint8_t rval, gval, bval;
  std::from_chars(x.begin(), x.end(), xval);
  std::from_chars(y.begin(), y.end(), yval);
  std::from_chars(height.begin(), height.end(), heightval);
  std::from_chars(width.begin(), width.end(), widthval);
  std::from_chars(r.begin(), r.end(), rval);
  std::from_chars(g.begin(), g.end(), gval);
  std::from_chars(b.begin(), b.end(), bval);

  auto color = HannOS::Display::Pixel{rval, gval, bval};
  for(int xx = xval; xx < xval + widthval; ++ xx) {
    for(int yy = yval; yy < yval + heightval; ++ yy) {
      HannOS::Display::putPixel(xx, yy, color);
    }
  }
  HannOS::Display::swapBuffers();
}

void getchars() {
  char chr;
  do {
    HannOS::Serial::varSerial("READY> ");
    chr = HannOS::Serial::read();
    HannOS::Serial::varSerial((std::uint16_t)chr, '\n');
  } while(chr != '\r' && chr != '\n');
}

void shell() {
  auto shellPrefix = "\033[33mhanus@smol $ \033[0m";
  HannOS::Serial::varSerial(shellPrefix);
  std::array<char, 0x1000> buffer{};
  auto lineIt = buffer.begin();

  auto parseCommand = [&buffer, &lineIt]() {
    if(lineIt == buffer.begin()) return;
    auto view = std::string_view{buffer.cbegin(), lineIt - buffer.cbegin()};
    auto cmd = HannOS::String::split<2>(view, ' ');
    if(cmd[0] == "getchars") getchars();
    else if(cmd[0] == "primes") primes();
    else if(cmd[0] == "rect") rect(view);
    else if(cmd[0] == "snek") snek();
    else {
      HannOS::Serial::varSerialLn("Unknown command: ", cmd[0]);
    }
  };

  while(true) {
    auto val = HannOS::Serial::read();
    if(val == '\r' || val == '\n') {
      *lineIt = 0;
      HannOS::Serial::varSerialLn();
      parseCommand();
      HannOS::Serial::varSerial(shellPrefix);
      lineIt = buffer.begin();
    }
    else if(val == '\x7f') { // Backspace
      if(lineIt != buffer.begin()) {
        --lineIt;
        HannOS::Serial::varSerial("\x08 \x08");
      }
    }
    else if(val == '\x04') { // EOT
      HannOS::Serial::varSerialLn(" BAIIII!");
      return;
    }
    else if(lineIt != buffer.end()) {
      HannOS::Serial::varSerial(val);
      *lineIt++ = val;
    }
  }
}

extern "C" void kernel() {
  /*for(int i = 0; ; ++ i) {
    for(int y = 0; y < HannOS::Display::height(); ++y) {
      for(int x = 0; x < HannOS::Display::width(); ++x) {
        auto f = [](auto val) { return val < 1 || val > 18; };
        auto c = i % 2 ? 0 : 255;
        Pixel p = (f(x % 20) || f(y % 20)) ? Pixel{c, c, c} : Pixel{0x8c, 0x00, 0xf8};
        HannOS::Display::putPixel(x, y, p);
      }
    }
    HannOS::Display::swapBuffers();
    HannOS::Serial::varSerialLn("Frawe!\n");*
  }*/
  
  HannOS::Serial::varSerialLn("All processes dieded");
  //HannOS::Serial::varSerialLn("Pointer length: ", static_cast<std::int8_t>(sizeof(std::intptr_t)));

  auto cpuid = HannOS::CPU::getCPUID();

  //printbenchmark(display);
  //primes(display);
  //memeditor(display);

  if (cpuid.features.rdrnd) {
    //CMatrixPP(display);
  } else {
    HannOS::Serial::varSerialLn("No RDRAND! PANIC!");
  }

  HannOS::Serial::varSerialLn(
    "CPU info: \"", cpuid.identifier.chars, "\", max_func: ", cpuid.maxFunc);

  HannOS::Serial::varSerial(
    "eax: ", cpuid.features.eax, '\n'
  , "ebx: ", cpuid.features.ebx, '\n'
  , "ecx: ", cpuid.features.ecx, '\n'
  , "edx: ", cpuid.features.edx, '\n'
  );

  HannOS::Serial::varSerial(
    "\n\n:(\nOopsie Whoopsie! Uwu we made a fucky wucky!! "
    "A wittle fucko boingo! The code monkeys at our "
    "headquarters are working VEWY HAWD to fix this!\n"
  );

  shell();

  HannOS::CPU::halt();
}
