include(CMakeForceCompiler)

set(MCU_FLOAT_ABI soft)
set(MCU_FPU fpv4-sp-d16)
set(MCU_ARCH cortex-m3)
set(MCU_FLOAT_ABI soft)
set(MUC_FAMILY STM32F1xx)
set(MCU_LINE   STM32F103xE)
set(DIR        ${CMAKE_CURRENT_SOURCE_DIR}/MX2_AudioPlayer)
set(MCU_LINKER_SCRIPT ${DIR}/STM32F103RDTx_FLASH.ld)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION 1)


set(COMMON_FLAGS "-mcpu=${MCU_ARCH} -mthumb -mfloat-abi=${MCU_FLOAT_ABI} -D${MCU_LINE} -g -DUSE_HAL_DRIVER -D$ARM_MATH_CM3 -lm -lc --specs=nosys.specs")
#set(COMMON_FLAGS "-mcpu=${MCU_ARCH} -mthumb -mthumb-interwork -mfloat-abi=${MCU_FLOAT_ABI} -ffunction-sections -fdata-sections -g -fno-common -fmessage-length=0 -D${MCU_LINE} -DUSE_HAL_DRIVER -DARM_MATH_CM3 -DUSE_DEBUG --specs=nosys.specs")
if (MCU_FLOAT_ABI STREQUAL hard)
    set(COMMON_FLAGS "${COMMON_FLAGS} -mfpu=${MCU_FPU}")
endif ()

set(CMAKE_C_COMPILER       arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER     arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER     arm-none-eabi-as)
set(CMAKE_OBJCOPY     	   arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP     	   arm-none-eabi-objdump)

set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=c++11 -T${MCU_LINKER_SCRIPT}")
set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99 -T${MCU_LINKER_SCRIPT}")
set(CMAKE_ASM_FLAGS "-mthumb -mcpu=${MCU_ARCH}")
#set(CMAKE_EXE_LINKER_FLAGS "-T ${MCU_LINKER_SCRIPT} ${COMMON_FLAGS}" CACHE STRING "Linker Flags")
#set(CMAKE_EXE_LINKER_FLAGS "-eb_start_files -Wl,--gc-sections -T ${MCU_LINKER_SCRIPT} ${COMMON_FLAGS}" CACHE STRING "Linker Flags")
