#pragma once

#include <functional>

namespace HannOS::Interrupts {
  inline uintmax_t interruptCounter = 0;
  constexpr std::uint8_t masterPICBase = 0x20;
  constexpr std::uintmax_t pitInterruptRate = 100;

  inline std::uint64_t disableInterrupts() {
    std::uint64_t flags;
    asm volatile("pushf\ncli\npop %0" : "=r"(flags) : : "memory");
    return flags;
  }

  inline void enableInterrupts(std::uint64_t flags) {
    asm volatile("push %0\npopf" : : "rm"(flags) : "memory","cc");
  }

  struct ScopedCriticalSection {
     ScopedCriticalSection(): flags{ disableInterrupts() } {  }
    ~ScopedCriticalSection()       { enableInterrupts(flags); }
  private:
    std::uint64_t flags;
  };

  enum struct Decision {
    Passthrough,
    Consume,
  };

  using DecidingHandler = std::function<Decision()>;
  using PlainHandler    = std::function<void()>;

  enum struct ISAIRQ {
    PIT      = 0,
    Keyboard = 1,
    Cascade  = 2,
    COM2     = 3,
    COM1     = 4,
    LPT2     = 5,
    Floppy   = 6,
    LPT1     = 7,
    RTC      = 8,
    Per1     = 9,
    Per2     = 10,
    Per3     = 11,
    Mouse    = 12,
    Coproc   = 13,
    PrimATA  = 14,
    SecATA   = 15,
  };

  enum struct Exception {
    DivideByZero               = 0,
    Debug                      = 1,
    NonMaskableInterrupt       = 2,
    Breakpoint                 = 3,
    Overflow                   = 4,
    BoundRangeExceeded         = 5,
    InvalidOpcode              = 6,
    DeviceNotAvailable         = 7,
    DoubleFault                = 8,
    CoprocSegmentOverrun       = 9,
    InvalidTSS                 = 10,
    SegmentNotPresent          = 11,
    StackSegmentFault          = 12,
    GeneralProtectionFault     = 13,
    PageFault                  = 14,
    // Reserved                = 15,
    x87FloatingPointException  = 16,
    AlignmentCheck             = 17,
    MachineCheck               = 18,
    SIMDFloatingPointException = 19,
    VirtualizationException    = 20,
    // Reserved                = 21...29,
    SecurityException          = 30,
    // Reserved                = 31,
  };

  // Enable or disable standard ISA IRQs by masking the PIC
  // void enableIRQ(ISAIRQ irq);
  // void disableIRQ(ISAIRQ irq);

  inline std::uint8_t interruptNumber(ISAIRQ irq) {
    return static_cast<std::uint8_t>(irq) + masterPICBase;
  }
  inline std::uint8_t interruptNumber(Exception ex) {
    return static_cast<std::uint8_t>(ex);
  }

  // Register a handler, returns the old handler for the slot
  PlainHandler registerHandler(std::uint8_t interruptNumber, PlainHandler &&handler);

  struct ScopedHandler {
    ScopedHandler(std::uint8_t interruptNumber, DecidingHandler &&handler):
        interruptNumber{interruptNumber}
      , current{std::move(handler)}
        // Register new handler, saving the old one
      , oldHandler{registerHandler(interruptNumber,
        [this]() {
          if(this->current() == Decision::Passthrough)
            this->oldHandler();
        }
      )}
    {
    }
    ~ScopedHandler() {
      // Restore old handler
      registerHandler(interruptNumber, std::move(oldHandler));
    }
  private:
    std::uint8_t interruptNumber;
    DecidingHandler current;
    PlainHandler oldHandler;
  };

  struct ExceptionFrame {
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
  };
  
  inline ExceptionFrame const *currentFrame;
}
