cmake_minimum_required(VERSION 3.0)

project(MX2 C CXX ASM)

enable_language(ASM)

include_directories(${DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include)
include_directories(${DIR}/Drivers/CMSIS/Include)
include_directories(${DIR}/Drivers/STM32F1xx_HAL_Driver/Inc)
include_directories(${DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy)
include_directories(${DIR}/Inc)
include_directories(${DIR}/Middlewares/Third_Party/FreeRTOS/Source/include/)
include_directories(${DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3/)
include_directories(${DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS)
include_directories(${DIR}/Middlewares/Third_Party/FatFs/src/drivers/)
include_directories(${DIR}/Middlewares/Third_Party/FatFs/src/)

file(GLOB_RECURSE OS_SOURCE     ${DIR}/Middlewares/Third_Party/*.c)
file(GLOB_RECURSE FAT_SOURCE    ${DIR}/Middlewares/Third_Party/FatFs/*.c)
file(GLOB_RECURSE DR_SOURCE     ${DIR}/Drivers/CMSIS/*.c ${DIR}/Drivers/STM32F1xx_HAL_Driver/*.c)
file(GLOB_RECURSE USR_SOURCE    ${DIR}/Src/*.c)
file(GLOB_RECURSE ASM_SOURCE    ${DIR}/*.s)

#message(STATUS ${OS_SOURCE})
#message(STATUS ${FAT_SOURCE})
#message(STATUS ${DR_SOURCE})
#message(STATUS ${USR_SOURCE})

set(ELF MX2.elf)
add_executable(${ELF} ${USR_SOURCE} ${FAT_SOURCE} ${DR_SOURCE} ${USR_SOURCE} ${ASM_SOURCE})
set_target_properties(${ELF} PROPERTIES LINKER_LANGUAGE C)
#target_link_libraries(${ELF} ${FreeRTOS})
#target_link_libraries(${ELF} ${Fatfs})
#target_link_libraries(${ELF} ${Driver})

