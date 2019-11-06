cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(HannOS CXX ASM)

set(CMAKE_CXX_FLAGS "-ffreestanding -g -Wall -fno-stack-protector -mno-red-zone -march=core2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -Wno-sign-compare -std=c++17 -Os -mno-soft-float")
#if(MSVC)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -WX -std:c++17 -Os")
#else()
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
#endif(MSVC)
#set(CMAKE_ASM_FLAGS "")

file(GLOB loaderfiles ${PROJECT_SOURCE_DIR}/Source/loader/*.s)
file(GLOB cppfiles ${PROJECT_SOURCE_DIR}/Source/HannOS/*.cpp)
file(GLOB sfiles ${PROJECT_SOURCE_DIR}/Source/HannOS/*.s)

add_executable(${PROJECT_NAME}.bin
  ${loaderfiles}
  ${cppfiles}
  ${sfiles}
)

target_include_directories(${PROJECT_NAME}.bin
  PRIVATE ${PROJECT_SOURCE_DIR}/Source/include
  PRIVATE ${PROJECT_SOURCE_DIR}/extern
)

set(LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/Source/linker.lds)

set(CMAKE_CXX_LINK_EXECUTABLE "ld <OBJECTS> -T ${LINKER_SCRIPT} -o <TARGET> -O3")
set_target_properties(${PROJECT_NAME}.bin
  PROPERTIES
    LINK_DEPENDS ${LINKER_SCRIPT}
)

# Make a stripped bootable iso using grub and xorriso
add_custom_command(
  OUTPUT ${PROJECT_NAME}.iso
  DEPENDS ${PROJECT_NAME}.bin
  COMMAND strip ${PROJECT_NAME}.bin -o iso/boot/${PROJECT_NAME}.bin
  COMMAND grub-mkrescue -o ${PROJECT_NAME}.iso iso
)

# Throw it into /boot/ if it exists, otherwise skip this step
add_custom_command(TARGET ${PROJECT_NAME}.bin POST_BUILD
  COMMAND [ -f "/boot/${PROJECT_NAME}.bin" ] && strip ${PROJECT_NAME}.bin -o /boot/${PROJECT_NAME}.bin || true
)

# Run it in QEMU
add_custom_target(go
  DEPENDS ${PROJECT_NAME}.iso
  COMMAND qemu-system-x86_64 -m 128 -cdrom ${PROJECT_NAME}.iso -boot d -cpu Haswell -serial stdio -no-reboot
)

# Run it in QEMU, with gdb server running and halted at startup
add_custom_target(dbg
  DEPENDS ${PROJECT_NAME}.iso
  COMMAND qemu-system-x86_64 -m 128 -cdrom ${PROJECT_NAME}.iso -boot d -cpu Haswell -serial stdio -S -s -no-reboot
)

# Run it in QEMU, with kvm
add_custom_target(goreal
  DEPENDS ${PROJECT_NAME}.iso
  COMMAND qemu-system-x86_64 -enable-kvm -m 128 -cdrom ${PROJECT_NAME}.iso -boot d -cpu host -serial stdio -no-reboot
)

add_custom_target(gobochs
  DEPENDS ${PROJECT_NAME}.iso
  COMMAND bochs -qf ../bochs.bxrc
)

