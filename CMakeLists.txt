set (PROJECT_NAME MX2)
set (PROJECT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/MX2_AudioPlayer")
set (MCU_FAMILY STM32F1xx)
set (MCU_LINE   STM32F103xE)
set (MCU_LINKER_SCRIPT  ${PROJECT_DIR}/STM32F103RDTx_FLASH.ld)

# compiler things
include(CMakeForceCompiler)

set(MCU_ARCH cortex-m3)
set(MCU_FLOAT_ABI soft)
set(MCU_FPU fpv4-sp-d16)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-gc-sections -T ${MCU_LINKER_SCRIPT} --specs=nosys.specs")
set(COMMON_FLAGS "-mcpu=${MCU_ARCH} -mthumb -mthumb-interwork -mfloat-abi=${MCU_FLOAT_ABI} -ffunction-sections -fdata-sections -g -fno-common -fmessage-length=0")

if (MCU_FLOAT_ABI STREQUAL hard)
    set(COMMON_FLAGS "${COMMON_FLAGS} -mfpu=${MCU_FPU}")
endif ()

set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99")
# endof compiler things


cmake_minimum_required(VERSION 3.6)

project(MX2)
# add option for asm

add_definitions(-D${MCU_LINE})
add_definitions(-D${USE_HAL_LIBRARY})
add_definitions(-DARM_MATH_CM3)

file(GLOB_RECURSE CMSIS_ASM    ${PROJECT_DIR}/startup/*.s)
file(GLOB_RECURSE HAL_SOURCE   ${PROJECT_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/*.c ${PROJECT_DIR}/system_stm32f1xx.c)
file(GLOB_RECURSE USR_SOURCE   ${PROJECT_DIR}/Src/*.c)
file(GLOB_RECURSE FREERTOS_SOURCE ${PROJECT_DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c)

include_directories(${PROJECT_DIR}/Inc)
include_directories(${PROJECT_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc)
include_directories(${PROJECT_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy)
include_directories(${PROJECT_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Inc)
include_directories(${PROJECT_DIR}/Drivers/CMSIS/Include)
include_directories(${PROJECT_DIR}/Drivers/Middlewares/)

add_library(FreeRTOS STATIC ${FREERTOS_SOURCE})



