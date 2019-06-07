#include "Multiboot2.hpp"

#include "CPU.hpp"

namespace HannOS::Multiboot2 {
  struct MultibootInfo {
    std::uint32_t total_size;
    std::uint32_t reserved;
  };

  extern "C" std::uint32_t *const multibootInfoLoc;
  extern "C" char multibootDataSpace[];
  extern "C" char multibootDataEnd;
  auto const maxSize = &multibootDataEnd - &multibootDataSpace[0];

  extern "C" void loadMultibootInfo() {
    if(!multibootInfoLoc) {
      Display::drawvar("Multiboot info not found!", Display::NewLine);
      HannOS::CPU::halt();
    }
    //Display::drawvar("Multiboot found at ", multibootInfoLoc, Display::NewLine);
    auto const mbSize = *multibootInfoLoc;
    if(mbSize > maxSize) {
      Display::drawvar(
        "Multiboot field too large (", mbSize, "). Maximum size is ",
        maxSize, Display::NewLine  
      );
      HannOS::CPU::halt();
    }
    std::memcpy(&multibootDataSpace[0], multibootInfoLoc, mbSize);

    auto curr = reinterpret_cast<std::uint32_t *>(&multibootDataSpace[0]) + 2;
    while(1) {
      auto type = *curr;
      auto sz = *(curr + 1);
      switch(type) {
        case 0: // End of multiboot info
          return;
        case CommandLine::typeTag:
          reinterpret_cast<CommandLine *>(curr)->handle();
          break;
        case BootloaderName::typeTag:
          reinterpret_cast<BootloaderName *>(curr)->handle();
          break;
        case BasicMemoryInfo::typeTag:
          reinterpret_cast<BasicMemoryInfo *>(curr)->handle();
          break;
        case BIOSBootDevice::typeTag:
          reinterpret_cast<BIOSBootDevice *>(curr)->handle();
          break;
        case MemoryMap::typeTag:
          reinterpret_cast<MemoryMap *>(curr)->handle();
          break;
        case FramebufferInfo::typeTag:
          reinterpret_cast<FramebufferInfo *>(curr)->handle();
          break;
        case ELFSymbols::typeTag:
          reinterpret_cast<ELFSymbols *>(curr)->handle();
          break;
        case APMTable::typeTag:
          reinterpret_cast<APMTable *>(curr)->handle();
          break;
        case ACPIRSDPv1::typeTag:
          reinterpret_cast<ACPIRSDPv1 *>(curr)->handle();
          break;
        case ACPIRSDPv2::typeTag:
          reinterpret_cast<ACPIRSDPv2 *>(curr)->handle();
          break;
        default:
          Display::drawvar("FIXME: Unhandled multiboot field: ", type, ' ', sz, Display::NewLine);
      }
      if(sz % 8 != 0)       // Entries are padded
        sz += 8 - (sz % 8); // to be 8-byte aligned
      curr += sz/4;
    }
  }
}
