cmake_minimum_required(VERSION 3.0)

project(MX2 C CXX ASM)

enable_language(ASM)

option (BSPH1 "use H1 bsp instead of default BSP" off)

# build MXCore static library
include(MX/CMakeLists.txt)

if (BSPH1)
include(BspH1/CMakeLists.txt)
else(BSPH1)
# build BSP static library
include(defaultBSP/CMakeLists.txt)
endif(BSPH1)

set(ELF MX2.elf)
add_executable(${ELF} ${USR_SOURCE} ${ASM_SOURCE} ${FAT_SOURCE})
target_link_libraries(${ELF} BSP)
set_target_properties(${ELF} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${ELF} MXCore)
