#include "Descriptors.hpp"
#include "Display.hpp"

#include "CPU.hpp"
#include "Memory.hpp"
#include "Random.hpp"

#include <random>

/*void loadGDT() {
  std::memset(globalDescriptorTable, '\x00', (1<<20));
  HannOS::DescriptorReg desc;
  auto descp = &desc;
  asm volatile("lgdt (%0)"
      : "=a"(descp));
}*/

void primes(DisplayHandle display) {
  constexpr unsigned mprime = 64*1024;
  char nonprime[mprime/2]{0, 1, 0};
  for(unsigned i = 3; i < mprime; i += 2) {
    if(!nonprime[i / 2]) {
      display().drawi(i);
      display().feedLine();
      for(unsigned j = i * i; j < mprime; j += i) {
        nonprime[j / 2] = 1;
      }
    }
  }
}

void printbenchmark(DisplayHandle display) {
  for(int i = 0; i < 0x10000; ++ i) {
    display().setCursor(0, 0);
    display().drawi(i);
  }
}

void memeditor(DisplayHandle disp) {
  //int *dummy = nullptr;
  //auto base = reinterpret_cast<int *>(&dummy);
  //auto base = reinterpret_cast<int *>(globalDescriptorTable);
  auto base = reinterpret_cast<int *>(0xb8000);
  int direction = 1;

  while (true) {
    auto addr = base;
    disp().setCursor(0, 0);
    disp().draws("Editing ");
    disp().drawi(reinterpret_cast<std::size_t>(base));
    for (int y = 1; y < Display::ScreenHeight; ++y) {
      disp().setCursor(0, y);
      disp().drawi(reinterpret_cast<std::size_t>(addr));
      disp().draws(":");
      for (int x = 2; x <= 6; ++x) {
        disp().setCursor(x * 9, y);
        auto val = to_chars(*addr);
        addr += direction;
        disp().draws(&std::get<0>(val));
      }
    }
  }
}

using namespace std::string_view_literals;
#include <algorithm>

void CMatrixPP(DisplayHandle disp) {

  //HannOS::CPU::outb(0x3D4, 0x0A);
	//HannOS::CPU::outb(0x3D5, (HannOS::CPU::inb(0x3D5) & 0xC0) | 0);
 
	//HannOS::CPU::outb(0x3D4, 0x0B);
	//HannOS::CPU::outb(0x3D5, (HannOS::CPU::inb(0x3D5) & 0xE0) | 0);
  //HannOS::CPU::outb(0x3d4, 0x0a);
  //HannOS::CPU::outb(0x3d5, 0x20);
  //outb(0x3D4, 0x0A);
  //outb(0x3D5, 0x20);
  disp().clear();
  struct Bunch {
    int start = std::uniform_int_distribution<int>
      {0, Display::ScreenHeight}
      (HannOS::CPU::RandomDevice);
    int end = Display::ScreenHeight;
  };
  std::array<Bunch, Display::ScreenWidth> bunches;
  auto updateBunch = [&](Bunch &bunch, int x) {
    if(bunch.start == Display::ScreenHeight) {
        // chance to start over
        if(std::uniform_int_distribution<int>{0, 25}(HannOS::CPU::RandomDevice)) {
          return;
        } else {
          bunch.end = 0;
          bunch.start = bunch.end - std::uniform_int_distribution<int>
                                    {11, 24}
                                    (HannOS::CPU::RandomDevice);
        }
      }
      // chance to update
      if (!std::uniform_int_distribution<int>{0, 16}(HannOS::CPU::RandomDevice))
        return;
      // Update the bunch
      if(bunch.end > 0 && bunch.end <= Display::ScreenHeight) {
        disp().decorate(x, bunch.end - 1, 0x0a); // Color character drawn last update green
      }
      if(++bunch.end <= Display::ScreenHeight) {
        // Make new character
        char sample;
        constexpr auto chars = ""sv
            "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
            "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f";
        std::sample(chars.begin(), chars.end(), &sample, 1, HannOS::CPU::RandomDevice);
        disp().decorate(x, bunch.end - 1, 0x0f); // Color character white
        disp().draw(x, bunch.end - 1, sample);   // Set to the sampled character
      }
      if(bunch.start++ < Display::ScreenHeight && bunch.start > 0) {
        disp().draw(x, bunch.start - 1, 0x20); // Clear trail
      }
  };

  while(1) {
    for(int i = 0; i < Display::ScreenWidth; ++ i)
      updateBunch(bunches[i], i);
    for(volatile int i = 0; i < 10000000; ++ i);
  }
  HannOS::CPU::halt();
}

#include <algorithm>
#include <sstream>
#include <cstdio>

extern "C" void kernel() {
  //loadGDT();
  Display display{};
  display.clear();

  display.draws("All processes dieded\n", 0xe0);
  display.draws("Pointer length: ");
  display.drawi(static_cast<std::int8_t>(sizeof(std::intptr_t)));
  display.feedLine();

  display.drawi(HannOS::CPU::RandomDevice());

  auto cpuid = HannOS::CPU::getCPUID();

  //printbenchmark(display);
  //primes(display);
  memeditor(display);

  //HannOS::CPU::outb(0x70, 0x8A);
  //HannOS::CPU::outb(0x71, 0x20);
  //display.drawi(HannOS::CPU::inb(0x07));

  if (cpuid.features.rdrnd) {
    CMatrixPP(display);
  } else {
    display.draws("No RDRAND! PANIC!\n");
  }

  display.drawvar("CPU info: \"", cpuid.identifier.chars, "\", max_func: ", cpuid.maxFunc, Display::NewLine);
  display.draws("CPU info: \"");
  display.draws(cpuid.identifier.chars);
  display.draws("\", max_func: ");
  display.drawi(cpuid.maxFunc);
  display.feedLine();
  display.draws("eax: ");
  display.drawi(cpuid.features.eax);
  display.feedLine();
  display.draws("ebx: ");
  display.drawi(cpuid.features.ebx);
  display.feedLine();
  display.draws("ecx: ");
  display.drawi(cpuid.features.ecx);
  display.feedLine();
  display.draws("edx: ");
  display.drawi(cpuid.features.edx);
  display.feedLine();

  display.draws("\n\n:(\nOopsie Whoopsie! Uwu we made a fucky wucky!! "
                "A wittle fucko boingo! The code monkeys at our "
                "headquarters are working VEWY HAWD to fix this!\n"
    , 0x1f
  );

  /*auto val = HannOS::kMalloc();
  display.draws("Allocated 0x1000 at ");
  display.drawi(val);
  display.feedLine();

  auto val2 = HannOS::kMalloc();
  display.draws("Allocated 0x1000 at ");
  display.drawi(val2);
  display.feedLine();

  HannOS::kFree(val);
  HannOS::kFree(val2);

  val = HannOS::kMalloc();
  display.draws("Allocated 0x1000 at ");
  display.drawi(val);
  display.feedLine();

  val2 = HannOS::kMalloc();
  display.draws("Allocated 0x1000 at ");
  display.drawi(val2);
  display.feedLine();*/
}
