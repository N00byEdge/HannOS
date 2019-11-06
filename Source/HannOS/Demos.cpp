#include "Demos.hpp"
#include "String.hpp"
#include "Serial.hpp"
#include "Display.hpp"
#include "Random.hpp"
#include "Keyboard.hpp"

#include <algorithm>
#include <cstdio>
#include <random>
#include <algorithm>

namespace {
  namespace Math {
    inline double sqrt(double val) {
      double out;
      __asm__ ("sqrtsd %1, %0" : "=x" (out) : "x" (val));
      return out;
    }
  }
}

namespace HannOS::Demos {
  void primesSerial(std::string_view argv) {
    unsigned mprime = 0;
    auto args = HannOS::String::parse(argv, ' ', HannOS::String::ParseIgnore{}, mprime);
    if(args.num < 2) {
      HannOS::Serial::varSerialLn("Usage: ", args.parts[0], " maxVal");
      return;
    }

    if(mprime > 2) { HannOS::Serial::varSerialLn(2u); }
    if(mprime > 3) { HannOS::Serial::varSerialLn(3u); }
    char nonprime[mprime/2]{0, 1, 0};
    for(unsigned i = 3; i < mprime; i += 2) if(!nonprime[i / 2]) {
      HannOS::Serial::varSerialLn(i);
      for(unsigned j = i * i; j < mprime; j += i)
        nonprime[j / 2] = 1;
    }
  }

  void snekVGAPS2PIT() {
    enum struct Direction: std::uint8_t {
      Up, Down, Left, Right,
    };

    enum struct TileGraphics: std::uint8_t {
      Snek, Head, Candy, None,
    };

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

    struct {
      bool updateGame = false;
      int interruptCount = 0;
    } update;

    Direction nextGoing = going;

    // @TODO: Adjust PIT interrupt rate somewhere in the game loop
    HannOS::Interrupts::ScopedHandler pit{interruptNumber(HannOS::Interrupts::ISAIRQ::PIT), [&update]() {
      if(++update.interruptCount == 500) {
        update.interruptCount = 0;
        update.updateGame = true;
      }
      return HannOS::Interrupts::Decision::Consume;
    }};

    HannOS::Interrupts::ScopedHandler keyboard{interruptNumber(HannOS::Interrupts::ISAIRQ::Keyboard), [&]() {
      auto status = HannOS::CPU::inb(HannOS::Keyboard::statusPort);

      if(status & 0x1) { // Omg character
        unsigned char keycode = HannOS::CPU::inb(HannOS::Keyboard::dataPort);
        switch(keycode) {
        case 0x11: // W
          if(going != Direction::Down)
            nextGoing = Direction::Up;
          return HannOS::Interrupts::Decision::Consume;
        case 0x1F: // S
          if(going != Direction::Up)
            nextGoing = Direction::Down;
          return HannOS::Interrupts::Decision::Consume;
        case 0x20: // D
          if(going != Direction::Left)
            nextGoing = Direction::Right;
          return HannOS::Interrupts::Decision::Consume;
        case 0x1E: // A
          if(going != Direction::Right)
            nextGoing = Direction::Left;
          return HannOS::Interrupts::Decision::Consume;
        default:
          break;
        }
      }
      return HannOS::Interrupts::Decision::Passthrough;
    }};

    while(1) {
      HannOS::CPU::halt();

      if(std::exchange(update.updateGame, false)) {
        int x = head->x;
        int y = head->y;

        going = nextGoing;

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
  }

  /*
  void printVGAText(HannOS::VGAHandle display) {
    for(int i = 0; i < 0x10000; ++ i) {
      display().setCursor(0, 0);
      display().drawi(i);
    }
  }
  */

  /*
  void memeditVGAText(HannOS::VGAHandle disp) {
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
  }
  */

  /*
  void matrixVGAText(HannOS::VGAHandle disp) {
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
  }
  */

  void rectVGA(std::string_view argv) {
    int x, y, height, width;
    std::uint8_t r = 0, g = 0, b = 0;
    auto args = HannOS::String::parse(argv, ' ', HannOS::String::ParseIgnore{}, x, y, height, width, r, g, b);

    if(args.num < 8) {
      HannOS::Serial::varSerialLn("Usage: ", args.parts[0], " x y height width r g b");
      return;
    }

    auto color = HannOS::Display::Pixel{};
    color.red = r;
    color.green = g;
    color.blue = b;

    for(int xx = x; xx < x + width; ++ xx) {
      for(int yy = y; yy < y + height; ++ yy) {
        HannOS::Display::putPixel(xx, yy, color);
      }
    }
    HannOS::Display::swapBuffers();
  }

  void getcharPS2() {
    unsigned char chr = 0x00;
    HannOS::Interrupts::ScopedHandler keyboardHandler{interruptNumber(HannOS::Interrupts::ISAIRQ::Keyboard), [&]() {
      auto status = HannOS::CPU::inb(HannOS::Keyboard::statusPort);

      if(status & 0x1) { // Omg character
        chr = HannOS::CPU::inb(HannOS::Keyboard::dataPort);
      }
      return HannOS::Interrupts::Decision::Consume;
    }};

    HannOS::Serial::varSerial("I will print all PS/2 keycodes until ESC is pressed.\n");
    HannOS::Serial::varSerial("READY> ");
    do {
      if(chr) {
        HannOS::Serial::varSerialLn((std::uint16_t)chr);
        if(std::exchange(chr, 0) == 1) // Escape
          break;
        else
          HannOS::Serial::varSerial("READY> ");
      }
      else
        HannOS::CPU::halt();
    } while(true);
  }

  void getcharSerial() {
    char chr;
    HannOS::Serial::varSerial("I will print all serial keycodes until Enter is pressed.\n");
    do {
      HannOS::Serial::varSerial("READY> ");
      chr = HannOS::Serial::read();
      HannOS::Serial::varSerial((std::uint16_t)chr, '\n');
    } while(chr != '\r' && chr != '\n');
  }

  void measureTimePITPS2() {
    struct {
      bool printTime = false;
      int interruptCount = 0;
    } update;

    HannOS::Serial::varSerialLn("Measuring time. Press any key to exit.");

    HannOS::Interrupts::ScopedHandler pit{interruptNumber(HannOS::Interrupts::ISAIRQ::PIT), [&update]() {
      if(++update.interruptCount == HannOS::Interrupts::pitInterruptRate) {
        update.interruptCount = 0;
        update.printTime = true;
      }
      return HannOS::Interrupts::Decision::Consume;
    }};

    bool keepGoing = true;

    HannOS::Interrupts::ScopedHandler keyboard{interruptNumber(HannOS::Interrupts::ISAIRQ::Keyboard), [&]() {
      auto status = HannOS::CPU::inb(HannOS::Keyboard::statusPort);

      if(status & 0x1) { // Omg character
        [[maybe_unused]]
        unsigned char keycode = HannOS::CPU::inb(HannOS::Keyboard::dataPort);
        if(std::exchange(keepGoing, false))
          return HannOS::Interrupts::Decision::Consume;
      }
      return HannOS::Interrupts::Decision::Passthrough;
    }};

    while(keepGoing) {
      HannOS::CPU::halt();

      if(std::exchange(update.printTime, false)) {
        HannOS::Serial::varSerialLn("Time update!");
      }
    }
  }

  void printInterruptCount() {
    HannOS::Serial::varSerialLn("Interrupt count: ", HannOS::Interrupts::interruptCounter);
  }

  void mandelbrotVGA() {
    using ScalarT = float;

    struct Complex {
      ScalarT real = 0, imag = 0;

      Complex operator*(Complex const &other) const {
        return Complex{
            real * other.real - imag * other.imag
          , real * other.imag + imag * other.real
        };
      }

      Complex operator+(Complex const &other) const {
        return Complex{real + other.real, imag + other.imag};
      }

      ScalarT operator()() const {
        return Math::sqrt(real * real + imag * imag);
      }
    };

    auto const width = HannOS::Display::width();
    auto const height = HannOS::Display::height();

    constexpr auto maxIter = 512;

    constexpr auto colors = [](){
      std::array<HannOS::Display::Pixel, maxIter> ret{};

      std::array<HannOS::Display::Pixel, 7> colorPoints = {
        HannOS::Display::Pixel{0, 0, 0},
        HannOS::Display::Pixel{0, 0, 255},
        HannOS::Display::Pixel{0, 255, 255},
        HannOS::Display::Pixel{0, 255, 0},
        HannOS::Display::Pixel{255, 255, 0},
        HannOS::Display::Pixel{255, 0, 0},
        HannOS::Display::Pixel{0, 0, 0},
      };

      for(int i = 0; i < maxIter; ++ i) {
        auto baseCol  = (i * colorPoints.size()) / maxIter;
        auto interpol = (i * colorPoints.size()) % maxIter;
        auto nextCol = baseCol + 1;

        if(baseCol == colorPoints.size() - 1)
          nextCol = baseCol;

        auto &c1 = colorPoints[baseCol];
        auto &c2 = colorPoints[nextCol];

        auto w2 = (float)interpol/maxIter;
        auto w1 = 1.f - w2;

        // Interpolate between colors
        ret[i].red   = c1.red   * w1 + c2.red   * w2;
        ret[i].green = c1.green * w1 + c2.green * w2;
        ret[i].blue  = c1.blue  * w1 + c2.blue  * w2;
      }

      return ret;
    }();

    auto valueAt = [&](ScalarT x, ScalarT y) -> HannOS::Display::Pixel const & {
      Complex currPoint{(ScalarT)x/width-1.5f, (float)y/height-.5f};
      Complex z{0, 0};
      for(int i = 0; i < maxIter; ++ i) {
        z = z * z + currPoint;
        if(z() >= 2)
          return colors[i];
      }
      return colors[0];
    };

    for(int y = 0; y < height; ++ y) {
      for(int x = 0; x < width; ++ x) {
        HannOS::Display::putPixel(x, y, valueAt(x, y));
      }
      HannOS::Display::swapBuffers();
    }
  }
}
