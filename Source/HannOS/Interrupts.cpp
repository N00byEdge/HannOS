#include <type_traits>
#include <cstdint>
#include <array>
#include <cmath>

#include "Interrupt.hpp"
#include "Bitfields.hpp"
#include "Serial.hpp"

extern "C" std::uintptr_t ignoreIRQ;
extern "C" std::uintptr_t handlers[0x100];
extern "C" std::uint16_t tssDescriptor;
extern "C" std::uint8_t intnum;
constexpr std::uintptr_t exceptionOffset = 0;

namespace {
  template<int ind>
  struct PIC {
    constexpr static auto base() {
      if constexpr(ind == 1) {
        return 0x20;
      }
      else if constexpr(ind == 2) {
        return 0xa0;
      }
    }

    constexpr static auto command() {
      return base();
    }

    constexpr static auto data() {
      return base() + 1;
    }
  };

  void mapPIC() {
    // https://wiki.osdev.org/PIC#Initialisation
    HannOS::CPU::outb(PIC<1>::command(), 0x11);
    HannOS::CPU::waitIO();
    HannOS::CPU::outb(PIC<2>::command(), 0x11);
    HannOS::CPU::waitIO();
    HannOS::CPU::outb(PIC<1>::data(), HannOS::Interrupts::masterPICBase); // PIC 1 offset
    HannOS::CPU::waitIO();
    HannOS::CPU::outb(PIC<2>::data(), HannOS::Interrupts::masterPICBase + 8);  // PIC 2 offset
    HannOS::CPU::waitIO();
    HannOS::CPU::outb(PIC<1>::data(), 0b0000'0100); // Master knows slave PIC at IRQ2
    HannOS::CPU::waitIO();
    HannOS::CPU::outb(PIC<2>::data(), 0b0000'0010); // Slave cascade identity
    HannOS::CPU::waitIO();

    HannOS::CPU::outb(PIC<1>::data(), 0x01);
    HannOS::CPU::waitIO();
    HannOS::CPU::outb(PIC<2>::data(), 0x01);
    HannOS::CPU::waitIO();

    // Mask in all interrupts
    HannOS::CPU::outb(PIC<1>::data(), 0);
    HannOS::CPU::outb(PIC<2>::data(), 0);
  }

  enum struct PICTag: int {
    Master = 1,
    Slave  = 2,
  };

  constexpr auto GetISR = 0x0B;
  constexpr auto EOI    = 0x20;

  template<PICTag tag>
  bool isSpurious(){
    HannOS::CPU::outb(PIC<static_cast<int>(tag)>::command(), GetISR);
    return HannOS::CPU::inb(PIC<static_cast<int>(tag)>::command()) & 0x80;
  }

  template<PICTag tag>
  void sendEOI() {
    if constexpr(tag == PICTag::Slave)
      HannOS::CPU::outb(PIC<2>::command(), EOI);
    HannOS::CPU::outb(PIC<1>::command(), EOI);
  }

  std::array<HannOS::Interrupts::PlainHandler, 0x100> handlerMap;
}

extern "C" std::uintptr_t exceptionStackPtr;

extern "C" void isrHandler() {
  switch(intnum) {
  case HannOS::Interrupts::masterPICBase + 7:
    if(isSpurious<PICTag::Master>())
      // Master PIC is a nice guy and doesn't demand anything from us if he passed us spurious shit
      return;
    break;
  case HannOS::Interrupts::masterPICBase + 15:
    if(isSpurious<PICTag::Slave>()) {
      // Oh shit we gotta thank the master for passing this fucking bullshit interrupt to us from the slave
      HannOS::CPU::outb(PIC<1>::command(), EOI);
      return;
    }
    break;
  default:
    break;
  }

  ++HannOS::Interrupts::interruptCounter;

  HannOS::Interrupts::currentFrame =
    reinterpret_cast<HannOS::Interrupts::ExceptionFrame const *>(
      exceptionStackPtr + 5 * sizeof(std::uintptr_t)
    );

  handlerMap[intnum]();

  switch(intnum) {
  case HannOS::Interrupts::masterPICBase     ... HannOS::Interrupts::masterPICBase + 7:
    sendEOI<PICTag::Master>();
    break;
  case HannOS::Interrupts::masterPICBase + 8 ... HannOS::Interrupts::masterPICBase + 15:
    sendEOI<PICTag::Slave>();
    break;
  default:
    break;
  }
}

namespace {
  template<PICTag tag>
  void setPICMask(std::uint8_t mask) {
    HannOS::CPU::outb(PIC<static_cast<int>(tag)>::data(), mask); 
  }

  template<PICTag tag>
  std::uint8_t getPICMask() {
    return HannOS::CPU::inb(PIC<static_cast<int>(tag)>::data());
  }
}

namespace HannOS::Interrupts {
  PlainHandler registerHandler(std::uint8_t interruptNumber, PlainHandler &&handler) {
    ScopedCriticalSection here{};
    return std::exchange(handlerMap[interruptNumber], std::move(handler));
  }
}

namespace {
  void initPIT() {
    constexpr std::uintmax_t baseFreq = 3579545;
    constexpr std::uintmax_t resetValue =
      std::round((float)baseFreq / HannOS::Interrupts::pitInterruptRate);
    static_assert(resetValue <= 0xffff);
    static_assert(resetValue > 0);

    //   7  |  6  |  5  |  4  |  3  |  2  |  1  |  0 
    // +----------------------------------------------+
    // | Channel  |  RW Mode  |   Channel Mode  | BCD |
    // +----------------------------------------------+
    // |    0     |   lo/hi   | rate generator  | no! |
    // +----------+-----------+-----------------+-----+
    HannOS::CPU::outb(0x43, 0b00'11'010'0);
    HannOS::CPU::outb(0x40, resetValue & 0xff);
    HannOS::CPU::outb(0x40, resetValue >> 16);
  }
}

extern bool keyboardInterruptHandler();

namespace {
  void clearAllHandlers() {
    for(auto &h : handlerMap)
      h = []() { };
  }

  void panic() {
    auto &frame = *HannOS::Interrupts::currentFrame;

    {
      HannOS::Interrupts::ScopedCriticalSection here{};
      clearAllHandlers();
    }

    HannOS::Serial::varSerialLn("Panic!!");
    /*
        std::uintptr_t stackPointer;
    std::uintptr_t rflags;
    std::uintptr_t codeSegment;
    std::uintptr_t instructionPointer;
    std::uintptr_t rdi;
    std::uintptr_t rsi;
    std::uintptr_t rdx;
    std::uintptr_t rcx;
    std::uintptr_t rbx;
    std::uintptr_t rax;
    */
    HannOS::Serial::varSerialLn("rip = ", frame.instructionPointer);
    HannOS::Serial::varSerialLn("rsp = ", frame.stackPointer);
    HannOS::Serial::varSerialLn("rax = ", frame.rax);
    HannOS::Serial::varSerialLn("rbx = ", frame.rbx);
    HannOS::Serial::varSerialLn("rcx = ", frame.rcx);
    HannOS::Serial::varSerialLn("rdx = ", frame.rdx);
    HannOS::Serial::varSerialLn("rsi = ", frame.rsi);
    HannOS::Serial::varSerialLn("rdi = ", frame.rdi);
    while(1)
      HannOS::CPU::halt();
  }
}

namespace {
  struct IDTEntry {
    std::uint16_t addrLow;
    std::uint16_t codeSelector;
    std::uint8_t moreZeroes = 0x00;
    struct Attrib {
      union {
        std::uint8_t repr;
        HannOS::Util::Bitfield<0, 4, std::uint8_t> gateType;

        // 0 for interrupts and traps, 1 for syscalls (?)
        HannOS::Util::Bitfield<4, 1, std::uint8_t> storage;

        // Minimum privilege you can programmatically interrupt from
        HannOS::Util::Bitfield<5, 2, std::uint8_t> descriptorPrivilegeLevel;

        HannOS::Util::Bitfield<7, 1, std::uint8_t> present;
      } __attribute__((packed));
    } __attribute__((packed));
    Attrib        attrib;
    std::uint16_t addrMid;
    std::uint32_t addrHigh;
    std::uint32_t anotherZero = 0x00000000;
  } __attribute__((packed));

  static_assert(sizeof(IDTEntry) == 16);

  IDTEntry encode(std::uintptr_t addr, IDTEntry::Attrib attrib) {
    union Addr {
      std::uintptr_t ptr;
      HannOS::Util::Bitfield<0, 16, std::uintptr_t> addrLow;
      HannOS::Util::Bitfield<16, 16, std::uintptr_t> addrMid;
      HannOS::Util::Bitfield<32, 32, std::uintptr_t> addrHigh;
    };

    Addr a;
    a.ptr = addr;

    IDTEntry result;
    result.addrLow = a.addrLow;
    result.addrMid = a.addrMid;
    result.addrHigh = a.addrHigh;

    result.codeSelector = 0x0008;
    result.attrib = attrib;

    return result;
  }

  std::array<IDTEntry, 0x100> IDT;

  static_assert(sizeof(IDT) == sizeof(IDTEntry) * IDT.size());
}

extern "C"
void enableInterrupts() {
  IDTEntry::Attrib attribException;
  attribException.gateType = 0xf;
  attribException.storage = 0;
  attribException.descriptorPrivilegeLevel = 0;
  attribException.present = 1;

  auto attribIrq = attribException;
  attribIrq.gateType = 0xe;
  attribIrq.descriptorPrivilegeLevel = 3;

  clearAllHandlers();

  for(int i = 0; i < IDT.size(); ++ i)
    // Encoding IRQ
    IDT[i] = encode(handlers[i], i < 20 ? attribException : attribIrq);

  // Setting IDT
  struct {
    std::uint16_t len;
    void *base;
  } __attribute__((packed)) idtr { sizeof(IDT) - 1, IDT.data() };
  __asm__("lidt %0" : : "m"(idtr));
  __asm__("ltr %0" : : "r"((std::uint16_t)(std::uintptr_t)&tssDescriptor));

  // Map the PIC to 0x10 ... 0x30
  mapPIC();

  // Initialize the PIT and set its interrupt rate
  initPIT();

  // Some default interrupt handlers
  HannOS::Interrupts::registerHandler(interruptNumber(HannOS::Interrupts::ISAIRQ::Keyboard), keyboardInterruptHandler);
  HannOS::Interrupts::registerHandler(interruptNumber(HannOS::Interrupts::Exception::InvalidOpcode), []() {
    HannOS::Serial::varSerialLn("Invalid opcode at ", HannOS::Interrupts::currentFrame->instructionPointer);
    panic();
  });
  HannOS::Interrupts::registerHandler(interruptNumber(HannOS::Interrupts::Exception::DoubleFault), []() {
    HannOS::Serial::varSerialLn("Double fault!");
    panic();
  });

  // Enabling interrupts
  __asm__("sti");
  // Interrupts enabled!
}
