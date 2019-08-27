cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(HannOS C CXX ASM)

set(C_AND_CXX_FLAGS "-ffreestanding -g -Wall -fno-stack-protector -mno-red-zone -march=core2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_AND_CXX_FLAGS} -fno-exceptions -fno-rtti")
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -WX -std:c++17 -Os")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -std=c++17 -Os")
endif(MSVC)
set(CMAKE_ASM_FLAGS "")

file(GLOB loaderfiles ${PROJECT_SOURCE_DIR}/Source/loader/*.s)
file(GLOB cppfiles ${PROJECT_SOURCE_DIR}/Source/HannOS/*.cpp)

add_executable(${PROJECT_NAME}.bin
  ${cppfiles}
  ${loaderfiles}
)

target_include_directories(${PROJECT_NAME}.bin
  PRIVATE ${PROJECT_SOURCE_DIR}/Source/include
  PUBLIC ${PROJECT_SOURCE_DIR}/extern
)

set(LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/Source/linker.lds)

set(CMAKE_CXX_LINK_EXECUTABLE "ld <OBJECTS> -T ${LINKER_SCRIPT} -o <TARGET> -O3")
set_target_properties(${PROJECT_NAME}.bin
  PROPERTIES
    LINK_DEPENDS ${LINKER_SCRIPT}
)

add_custom_target(iso ALL
  DEPENDS ${PROJECT_NAME}.bin
  COMMAND strip ${PROJECT_NAME}.bin -o iso/boot/${PROJECT_NAME}.bin
  COMMAND grub-mkrescue -o ${PROJECT_NAME}.iso iso
)

add_custom_target(boot ALL
  DEPENDS ${PROJECT_NAME}.bin
  COMMAND strip ${PROJECT_NAME}.bin -o /boot/${PROJECT_NAME}.bin
)

add_custom_target(go
  DEPENDS iso
  COMMAND qemu-system-x86_64 -m 512 -cdrom HannOS.iso -boot d -cpu Haswell -serial stdio -no-reboot
)

add_custom_target(dbg
  DEPENDS iso
  COMMAND qemu-system-x86_64 -m 512 -cdrom HannOS.iso -boot d -cpu Haswell -serial stdio -S -s -no-reboot
)

add_custom_target(goreal
  DEPENDS iso
  COMMAND qemu-system-x86_64 -enable-kvm -m 512 -cdrom HannOS.iso -boot d -cpu host -serial stdio -no-reboot
)
