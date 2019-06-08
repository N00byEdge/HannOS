#include "Descriptors.hpp"
#include "Serial.hpp"
#include "CPU.hpp"
#include "Memory.hpp"
#include "Random.hpp"
#include "Paging.hpp"

#include <random>

void primes() {
  constexpr unsigned mprime = 64*1024;
  char nonprime[mprime/2]{0, 1, 0};
  for(unsigned i = 3; i < mprime; i += 2) if(!nonprime[i / 2]) {
    HannOS::Serial::varSerialLn(i);
    for(unsigned j = i * i; j < mprime; j += i)
      nonprime[j / 2] = 1;
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

using namespace std::string_view_literals;
#include <algorithm>

/*void CMatrixPP(HannOS::VGAHandle disp) {
  //HannOS::CPU::outb(0x3D4, 0x0A);
  //HannOS::CPU::outb(0x3D5, (HannOS::CPU::inb(0x3D5) & 0xC0) | 0);
 
  // HannOS::CPU::outb(0x3D4, 0x0B);
  // HannOS::CPU::outb(0x3D5, (HannOS::CPU::inb(0x3D5) & 0xE0) | 0);
  // HannOS::CPU::outb(0x3d4, 0x0a);
  // HannOS::CPU::outb(0x3d5, 0x20);
  // outb(0x3D4, 0x0A);
  // outb(0x3D5, 0x20);
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
} */

#include <algorithm>
#include <sstream>
#include <cstdio>

#include "Display.hpp"

extern "C" void kernel() {
  for(int y = 0; y < HannOS::Display::height(); ++y) {
    for(int x = 0; x < HannOS::Display::width(); ++x) {
      Pixel p = (x % 20 == 0 || y % 20 == 0) ? Pixel{0xff, 0xff, 0xff} : Pixel{0x8c, 0x00, 0xf8};
      HannOS::Display::putPixel(x, y, p);
    }
  }
  
  HannOS::Serial::varSerialLn("All processes dieded");
  HannOS::Serial::varSerialLn("Pointer length: ", static_cast<std::int8_t>(sizeof(std::intptr_t)));

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
}
