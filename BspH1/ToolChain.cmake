include(CMakeForceCompiler)

set(MCU_FLOAT_ABI soft)
set(MCU_FPU fpv4-sp-d16)
set(MCU_ARCH cortex-m3)
set(MCU_FLOAT_ABI softfp)
set(MUC_FAMILY STM32F1xx)
set(MCU_LINE   STM32F103xE)
set(DIR        ${CMAKE_CURRENT_SOURCE_DIR}/BspH1)
set(MCU_LINKER_SCRIPT ${DIR}/STM32F103RDTx_FLASH.ld)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION 1)

set(COMMON_FLAGS "-mcpu=${MCU_ARCH} -mthumb -mfloat-abi=${MCU_FLOAT_ABI} -D${MCU_LINE}-g -DUSE_HAL_DRIVER -DARM_MATH_CM4 -lm -lc --specs=nosys.specs")
set(COMMON_FLAGS "${COMMON_FLAGS} -DGD32F30X_XD")
set(COMMON_FLAGS "${COMMON_FLAGS} -DGD32F303xG")

# remove unsed function & data
set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -fdata-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -Wl,--gc-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -Os")

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
