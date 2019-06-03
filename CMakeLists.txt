cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(HannOS C CXX ASM)

set(C_AND_CXX_FLAGS "-ffreestanding -g -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_AND_CXX_FLAGS} -Wall -fno-exceptions -fno-rtti")
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -WX -std:c++17 -Os")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wno-sign-compare -std=c++17 -Os")
endif(MSVC)
set(CMAKE_ASM_FLAGS "")

file(GLOB loaderfiles ${PROJECT_SOURCE_DIR}/Source/loader/*.s)
file(GLOB cppfiles ${PROJECT_SOURCE_DIR}/Source/HannOS/*.cpp)

# add_library(loader32
#   ${loaderfiles}
# )

# add_custom_command(
#   OUTPUT libloader64.a DEPENDS loader32
#   COMMAND objcopy -O elf64-x86-64 libloader32.a libloader64.a
# )

add_executable(${PROJECT_NAME}.bin
  ${cppfiles}
  ${loaderfiles}
)

# add_custom_target(loader64_target DEPENDS libloader64.a)
# add_dependencies(${PROJECT_NAME}.bin loader64_target)

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
  COMMAND cp ${PROJECT_NAME}.bin iso/boot/${PROJECT_NAME}.bin
  #COMMAND strip ${PROJECT_NAME}.bin -o iso/boot/${PROJECT_NAME}.bin
  COMMAND grub-mkrescue -o ${PROJECT_NAME}.iso iso
)

add_custom_target(boot ALL
  DEPENDS ${PROJECT_NAME}.bin
  COMMAND strip ${PROJECT_NAME}.bin -o /boot/${PROJECT_NAME}.bin
)

add_custom_target(go
  DEPENDS iso
  COMMAND qemu-system-x86_64 -m 512 -cdrom HannOS.iso -boot d -cpu Haswell
)

add_custom_target(dbg
  DEPENDS iso
  COMMAND qemu-system-x86_64 -m 512 -cdrom HannOS.iso -boot d -cpu Haswell -S -s
)

add_custom_target(goreal
  DEPENDS iso
  COMMAND kvm -m 512 -cdrom HannOS.iso -boot d -cpu host
)
