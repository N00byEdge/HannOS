#include "Serial.hpp"
#include "CPU.hpp"
#include "Demos.hpp"

using namespace std::string_view_literals;

void shell() {
  auto shellPrefix = "\033[33mhanus@smol $ \033[0m";
  HannOS::Serial::varSerial(shellPrefix);
  std::array<char, 0x1000> buffer{};
  auto lineIt = buffer.begin();

  auto parseCommand = [&buffer, &lineIt]() {
    if(lineIt == buffer.begin()) return;
    auto view = std::string_view{buffer.cbegin(), static_cast<std::size_t>(lineIt - buffer.cbegin())};
    auto cmd = HannOS::String::split<2>(view, ' ');

    if(cmd.parts[0] == "getserial") HannOS::Demos::getcharSerial();
    else if(cmd.parts[0] == "primes") HannOS::Demos::primesSerial(view);
    else if(cmd.parts[0] == "rect") HannOS::Demos::rectVGA(view);
    else if(cmd.parts[0] == "snek") HannOS::Demos::snekVGAPS2PIT();
    else if(cmd.parts[0] == "nano") HannOS::Serial::varSerialLn("Flipped a bit somewhere in memory...");
    else if(cmd.parts[0] == "getps2") HannOS::Demos::getcharPS2();
    else if(cmd.parts[0] == "intcount") HannOS::Demos::printInterruptCount();
    else if(cmd.parts[0] == "timecounter") HannOS::Demos::measureTimePITPS2();
    else if(cmd.parts[0] == "mbrot") HannOS::Demos::mandelbrotVGA();
    else {
      HannOS::Serial::varSerialLn("Unknown command: ", cmd.parts[0]);
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
  HannOS::Serial::varSerialLn("All processes dieded");

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

  /* Do event loop */
  //while(true) {
  //  HannOS::CPU::halt();
  //}
}
