cmake_minimum_required(VERSION 3.0)

project (BSPH1 C ASM)

enable_language(ASM)

set (BSP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/BspH1)

# set target compile flags
set(MCU_FPU fpv4-sp-d16)
set(MCU_ARCH cortex-m4)
set(MCU_FLOAT_ABI softfp)
set(MUC_FAMILY STM32F1xx)
set(MCU_LINE   STM32F103xE)
set(MCU_LINKER_SCRIPT ${BSP_DIR}/STM32F103RDTx_FLASH.ld)

set(COMMON_FLAGS "-mcpu=${MCU_ARCH}")
set(COMMON_FLAGS "${COMMON_FLAGS} -mthumb")
set(COMMON_FLAGS "${COMMON_FLAGS} -mfloat-abi=${MCU_FLOAT_ABI}")
set(COMMON_FLAGS "${COMMON_FLAGS} -g")
set(COMMON_FLAGS "${COMMON_FLAGS} -lm")
set(COMMON_FLAGS "${COMMON_FLAGS} -lc")
set(COMMON_FLAGS "${COMMON_FLAGS} --specs=nosys.specs")
set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -fdata-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -Wl,--gc-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -Os")

set(COMMON_FLAGS "${COMMON_FLAGS} -D${MCU_LINE}")
set(COMMON_FLAGS "${COMMON_FLAGS} -DUSE_HAL_DRIVER")
set(COMMON_FLAGS "${COMMON_FLAGS} -DARM_MATH_CM4")
set(COMMON_FLAGS "${COMMON_FLAGS} -DGD32F30X_XD")
set(COMMON_FLAGS "${COMMON_FLAGS} -DGD32F303xG")



if (MCU_FLOAT_ABI STREQUAL hard)
    set(COMMON_FLAGS "${COMMON_FLAGS} -mfpu=${MCU_FPU}")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=gnu++14 -fno-exceptions -T${MCU_LINKER_SCRIPT}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -std=gnu99 -T${MCU_LINKER_SCRIPT}")
set(CMAKE_ASM_FLAGS "-mthumb -mcpu=${MCU_ARCH}")

include_directories(${BSP_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include)
include_directories(${BSP_DIR}/Drivers/CMSIS/Include)
include_directories(${BSP_DIR}/Drivers/GD32F30x_standard_peripheral/Include)
include_directories(${BSP_DIR}/Drivers/SPI_SD/Inc)
include_directories(${BSP_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc)
include_directories(${BSP_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy)
include_directories(${BSP_DIR}/Inc)
include_directories(${BSP_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include/)
include_directories(${BSP_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/)
include_directories(${BSP_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS)
include_directories(${BSP_DIR}/Middlewares/Third_Party/FatFs/src/drivers/)
include_directories(${BSP_DIR}/Middlewares/Third_Party/FatFs/src/)

# file(GLOB_RECURSE OS_SOURCE     ${BSP_DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c)
set (OS_DIR ${BSP_DIR}/Middlewares/Third_Party/FreeRTOS/Source)
file(GLOB OS_SOURCE ${OS_DIR}/CMSIS_RTOS/*.c ${OS_DIR}/*.c ${OS_DIR}/portable/GCC/ARM_CM4F/*.c ${OS_DIR}/portable/MemMang/*.c)
file(GLOB_RECURSE FAT_SOURCE    ${BSP_DIR}/Middlewares/Third_Party/FatFs/src/*.c)
file(GLOB_RECURSE DR_SOURCE     ${BSP_DIR}/Drivers/*.c)
file(GLOB_RECURSE USR_SOURCE    ${BSP_DIR}/Src/*.c ${BSP_DIR}/Port/*.c ${BSP_DIR}/Port/*.cpp ${BSP_DIR}/Src/*.cpp)
file(GLOB_RECURSE ASM_SOURCE    ${BSP_DIR}/startup/startup_stm32f103xe_gcc.s)

add_library(BSP STATIC ${DR_SOURCE} ${OS_SOURCE})