cmake_minimum_required(VERSION 3.0)

project (BSPH1 C ASM)

enable_language(ASM)

include_directories(${DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include)
include_directories(${DIR}/Drivers/CMSIS/Include)
include_directories(${DIR}/Drivers/GD32F30x_standard_peripheral/Include)
include_directories(${DIR}/Drivers/SPI_SD/Inc)
include_directories(${DIR}/Drivers/STM32F1xx_HAL_Driver/Inc)
include_directories(${DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy)
include_directories(${DIR}/Inc)
include_directories(${DIR}/Middlewares/Third_Party/FreeRTOS/Source/include/)
include_directories(${DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/)
include_directories(${DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS)
include_directories(${DIR}/Middlewares/Third_Party/FatFs/src/drivers/)
include_directories(${DIR}/Middlewares/Third_Party/FatFs/src/)

# file(GLOB_RECURSE OS_SOURCE     ${DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c)
set (OS_DIR ${DIR}/Middlewares/Third_Party/FreeRTOS/Source)
file(GLOB OS_SOURCE ${OS_DIR}/CMSIS_RTOS/*.c ${OS_DIR}/*.c ${OS_DIR}/portable/GCC/ARM_CM4F/*.c ${OS_DIR}/portable/MemMang/*.c)
file(GLOB_RECURSE FAT_SOURCE    ${DIR}/Middlewares/Third_Party/FatFs/src/*.c)
file(GLOB_RECURSE DR_SOURCE     ${DIR}/Drivers/*.c)
file(GLOB_RECURSE USR_SOURCE    ${DIR}/Src/*.c ${DIR}/Port/*.c ${DIR}/Port/*.cpp ${DIR}/Src/*.cpp)
file(GLOB_RECURSE ASM_SOURCE    ${DIR}/startup/startup_stm32f103xe_gcc.s)

add_library(BSP STATIC ${DR_SOURCE} ${OS_SOURCE})