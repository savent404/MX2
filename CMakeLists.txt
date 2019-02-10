cmake_minimum_required(VERSION 3.0)

project(MX2 C CXX ASM)

enable_language(ASM)

# build MXCore static library
include(MX/CMakeLists.txt)

# build BSP static library
include(defaultBSP/CMakeLists.txt)

set(ELF MX2.elf)
add_executable(${ELF} ${USR_SOURCE} ${ASM_SOURCE} ${FAT_SOURCE})
target_link_libraries(${ELF} BSP)
set_target_properties(${ELF} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${ELF} MXCore)
