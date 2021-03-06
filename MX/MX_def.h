#pragma once

#if __GNUC__ >= 4
#    ifdef __cplusplus
#        define MX_C_API extern "C"
#    else
#        define MX_C_API
#    endif
#elif __ICCARM__
#    ifdef __cplusplus
#        define MX_C_API extern "C"
#    else
#        define MX_C_API
#    endif
#endif

#define MX_INTERNAL_API MX_C_API
#define MX_PORT_API MX_C_API

#define __MX_WEAK __attribute__((weak))

#ifndef MX_MUX_BUFFSIZE
#    define MX_MUX_BUFFSIZE (512)
#endif

// MUX thread stack
#ifndef MX_MUX_THREAD_STACK_SIZE
#    define MX_MUX_THREAD_STACK_SIZE (1024+512)
#endif

#ifndef MX_MUX_DUAL_TRACK
#    define MX_MUX_DUAL_TRACK (1)
#endif

#if MX_MUX_DUAL_TRACK == 0
#    define MX_MUX_MAXIUM_TRACKID (1)
#else
#    define MX_MUX_MAXIUM_TRACKID (2)
#endif

#ifndef MX_LOOP_INTERVAL
#    define MX_LOOP_INTERVAL (20)
#endif

#ifndef USE_NP
#    define USE_NP 0
#endif

#ifndef MX_LED_INTERVAL
#    define MX_LED_INTERVAL (20)
#endif

#define MX_LED_MS2CNT(x) ((x) / MX_LED_INTERVAL)

/**
 * @brief LED持续时长支持根据音频时长
 */
#ifndef LED_SUPPORT_FOLLOW_AUDIO
#    if USE_NP == 0
#        define LED_SUPPORT_FOLLOW_AUDIO (0)
#    else
#        define LED_SUPPORT_FOLLOW_AUDIO (1)
#    endif
#endif

#ifndef MX_WTDG_FEED
#    ifdef USE_DEBUG
#        define MX_WTDG_FEED() NULL
#    else
MX_C_API void MX_WTDG_HW_Feed(void);
#        define MX_WTDG_FEED() MX_WTDG_HW_Feed()
#    endif
#endif

#ifndef MX_MUX_WAV_FIX_OFFSET
#    define MX_MUX_WAV_FIX_OFFSET (44)
#endif

#ifndef MX_MUX_WAV_VOL_LEVEL
#    define MX_MUX_WAV_VOL_LEVEL (7)
#endif
#ifndef MX_getMsTime
#    define MX_getMsTime() (osKernelSysTick() * 1000 / osKernelSysTickFrequency)
#endif

#define returnLastStack(id) (uint16_t(uxTaskGetStackHighWaterMark(static_cast<TaskHandle_t>(id))))
