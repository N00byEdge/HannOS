#pragma once

#include <string_view>

// Demos using different interfaces of the OS
// Interfaces could be (for example):
// * PS2 (PS/2)
// * VGA ((S)VGA)
// * Serial (COM[1-4])
// * VGAText (VGA text mode)
// * PIT (Programmable interrupt timer)

// If multiple are used, they are listed from most significant to least.

namespace HannOS::Demos {
  void primesSerial(std::string_view argv);
  void snekVGAPS2PIT();
  //void printVGAText(HannOS::VGAHandle display);
  //void memeditVGAText(HannOS::VGAHandle display);
  //void matrixVGAText(HannOS::VGAHandle display);
  void rectVGA(std::string_view argv);
  void getcharPS2();
  void getcharSerial();
  void measureTimePITPS2();
  void printInterruptCount();
  void mandelbrotVGA();
}
