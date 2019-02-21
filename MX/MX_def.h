#pragma once

#if __GNUC__ >= 4
#   ifdef __cplusplus
#       define MX_C_API extern "C"
#   else
#       define MX_C_API
#   endif
#elif __ICCARM__
#   ifdef __cplusplus
#       define MX_C_API extern "C"
#   else
#       define MX_C_API
#   endif
#endif

#define MX_INTERNAL_API MX_C_API
#define MX_PORT_API MX_C_API

#define __MX_WEAK __attribute__((weak))

#ifndef MX_MUX_BUFFSIZE
#define MX_MUX_BUFFSIZE (512)
#endif

// MUX thread stack
#ifndef MX_MUX_THREAD_STACK_SIZE
#define MX_MUX_THREAD_STACK_SIZE (1024)
#endif

#ifndef MX_MUX_DUAL_TRACK
#define MX_MUX_DUAL_TRACK (0)
#endif

#undef MX_MUX_MAXIUM_TRACKID
#if MX_MUX_DUAL_TRACK == 0
#define MX_MUX_MAXIUM_TRACKID (1)
#else
#define MX_MUX_MAXIUM_TRACKID (2)
#endif


#ifndef MX_LOOP_INTERVAL
#define MX_LOOP_INTERVAL (20)
#endif

#ifndef USE_NP
#define USE_NP 0
#endif

#ifndef MX_LED_INTERVAL
#define MX_LED_INTERVAL  (20)
#endif

#ifndef MX_WTDG_FEED
#   ifdef USE_DEBUG
#       define MX_WTDG_FEED() NULL
#   else
void MX_WTDG_HW_Feed(void);
#       define MX_WTDG_FEED() MX_WTDG_HW_Feed()
#   endif
#endif

#ifndef MX_MUX_WAV_FIX_OFFSET
#define MX_MUX_WAV_FIX_OFFSET (44)
#endif