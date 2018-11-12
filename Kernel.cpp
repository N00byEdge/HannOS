#include "Descriptors.hpp"
#include "Display.hpp"

#include "CPU.hpp"
#include "Memory.hpp"

extern "C" void okExample() {
  auto mem = reinterpret_cast<int *>(0xb8000);
  *mem = 0x2f4b2f4f;
} 

void loadGDT() {
  std::memset(globalDescriptorTable, '\x00', (1<<20));
  HannOS::DescriptorReg desc;
  auto descp = &desc;
  asm volatile("lgdt (%0)"
      : "=a"(descp));
}

void primes(DisplayHandle display) {
  constexpr unsigned mprime = 64*1024;
  char nonprime[mprime/2]{0, 1, 0};
  for(unsigned i = 3; i < mprime; i += 2) {
    if(!nonprime[i / 2]) {
      display().drawi(i);
      display().draws("\n");
      for(unsigned j = i + i; j < mprime; j += i) {
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
      for (int x = 1; x <= 8; ++x) {
        disp().setCursor(x * 9, y);
        auto val = to_chars(*addr);
        addr += direction;
        disp().draws(&std::get<0>(val));
      }
    }
  }
}

#include <algorithm>
#include <sstream>

extern "C" void kernel() {
  loadGDT();
  
  Display display{};
  display.clear();
	display.draws("All processes dieded\n", 0xe0);

  //printbenchmark(display);
  //primes(display);
  //memeditor(display);

  auto cpuid = HannOS::CPU::getCPUID();
  
  display.draws("CPU info: ");
  display.draws(cpuid.identifier);
  display.feedLine();
  display.draws("eax: ");
  display.drawi(unsigned{cpuid.features.eax});
  display.feedLine();
  display.draws("ebx: ");
  display.drawi(unsigned{cpuid.features.ebx});
  display.feedLine();
  display.draws("ecx: ");
  display.drawi(unsigned{cpuid.features.ecx});
  display.feedLine();
  display.draws("edx: ");
  display.drawi(unsigned{cpuid.features.edx});
  display.feedLine();

  display.draws("\n\n:(\nOopsie Whoopsie! Uwu we made a fucky wucky!! A wittle fucko boingo! The code monkeys at our headquarters are working VEWY HAWD to fix this!", 0x1f);
  display.feedLine();

  auto val = kMalloc();
  display.draws("Allocated 0x1000 at ");
  display.drawi(val);
  display.feedLine();

  auto val2 = kMalloc();
  display.draws("Allocated 0x1000 at ");
  display.drawi(val2);
  display.feedLine();

  kFree(val);
  kFree(val2);

  val = kMalloc();
  display.draws("Allocated 0x1000 at ");
  display.drawi(val);
  display.feedLine();

  val2 = kMalloc();
  display.draws("Allocated 0x1000 at ");
  display.drawi(val2);
  display.feedLine();
}
