cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(HannOS C CXX ASM)

set(C_AND_CXX_FLAGS "-ffreestanding -m32 -g -mno-sse -mno-mmx -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${C_AND_CXX_FLAGS} -Wall -fno-exceptions -fno-rtti")
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -WX -std:c++17 -Os")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wno-sign-compare -std=c++17 -Os")
endif(MSVC)
set(CMAKE_ASM_FLAGS "-m32")

include_directories(
  ${PROJECT_SOURCE_DIR}
)

file(GLOB asmfiles ${PROJECT_SOURCE_DIR}/*.s)
file(GLOB cfiles ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB cppfiles ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME}.bin
	${asmfiles}
	${cfiles}
	${cppfiles}
)

set(LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/linker.lds)

set(CMAKE_CXX_LINK_EXECUTABLE "ld <OBJECTS> -T ${LINKER_SCRIPT} -o <TARGET>")
set_target_properties(${PROJECT_NAME}.bin PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

