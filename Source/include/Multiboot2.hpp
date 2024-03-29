#pragma once

#include <cstdint>

#include "Serial.hpp"

namespace HannOS::Multiboot2 {
    struct CommandLine {
    constexpr static std::uint32_t typeTag = 1;
    std::uint32_t type;
    std::uint32_t size;

    void handle() {
      Serial::varSerialLn(
        "Command line: ", reinterpret_cast<char const *>(this) + sizeof(*this));
    }
  };
  static_assert(sizeof(CommandLine) == 8);

  struct BootloaderName {
    constexpr static std::uint32_t typeTag = 2;
    std::uint32_t type;
    std::uint32_t size;

    void handle() {
      Serial::varSerialLn(
        "Bootloader: ", reinterpret_cast<char const *>(this) + sizeof(*this));
    }
  };
  static_assert(sizeof(CommandLine) == 8);

  struct BasicMemoryInfo {
    constexpr static std::uint32_t typeTag = 4;
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t mem_lower;
    std::uint32_t mem_upper;

    void handle() {
      Serial::varSerialLn(
        "Basic memory info: low = ", mem_lower, " high = ", mem_upper);
    }
  };
  static_assert(sizeof(BasicMemoryInfo) == 16);

  struct BIOSBootDevice {
    constexpr static std::uint32_t typeTag = 5;
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t biosdev;
    std::uint32_t partition;
    std::uint32_t sub_partition;

    void handle() {
      Serial::varSerialLn("BIOS booted: ", biosdev, ':', partition, '-', sub_partition);
    }
  };
  static_assert(sizeof(BIOSBootDevice) == 20);

  struct MemoryMap {
    constexpr static std::uint32_t typeTag = 6;
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t entry_size;
    std::uint32_t entry_version;

    void handle();
  };
  static_assert(sizeof(MemoryMap) == 16);

  struct FramebufferInfo {
    constexpr static std::uint32_t typeTag = 8;
    std::uint32_t type;
    std::uint32_t size;
    std::uint64_t framebuffer_addr;
    std::uint32_t framebuffer_pitch;
    std::uint32_t framebuffer_width;
    std::uint32_t framebuffer_height;
    std::uint8_t  framebuffer_bpp;
    std::uint8_t  framebuffer_type;
    std::uint8_t  reserved;

    void handle();
  }__attribute__((packed));
  static_assert(sizeof(FramebufferInfo) == 31);

  struct ELFSymbols {
    constexpr static std::uint32_t typeTag = 9;
    std::uint32_t type;
    std::uint32_t size;
    std::uint16_t num;
    std::uint16_t entsize;
    std::uint16_t shndx;
    std::uint16_t reserved;

    void handle() {
      Serial::varSerialLn("FIXME: Handle ELF symbols");
    }
  };
  static_assert(sizeof(ELFSymbols) == 16);

  struct APMTable {
    constexpr static std::uint32_t typeTag = 10;
    std::uint32_t type;
    std::uint32_t size;
    std::uint16_t version;
    std::uint16_t cseg;
    std::uint32_t offset;
    std::uint16_t cseg_16;
    std::uint16_t dseg;
    std::uint16_t flags;
    std::uint16_t cseg_len;
    std::uint16_t cseg_16_len;
    std::uint16_t dseg_len;

    void handle() {
      Serial::varSerialLn("FIXME: Handle APM table");
    }
  };
  static_assert(sizeof(APMTable) == 28);

  struct ACPIRSDPv1 {
    constexpr static std::uint32_t typeTag = 14;
    std::uint32_t type;
    std::uint32_t size;

    void handle() {
      Serial::varSerialLn("ACPI RSDPv1 with size ", size);
    }
  };
  static_assert(sizeof(ACPIRSDPv1) == 8);

  struct ACPIRSDPv2 {
    constexpr static std::uint32_t typeTag = 15;
    std::uint32_t type;
    std::uint32_t size;

    void handle() {
      Serial::varSerialLn("ACPI RSDPv2 with size ", size);
    }
  };
  static_assert(sizeof(ACPIRSDPv2) == 8);
}
