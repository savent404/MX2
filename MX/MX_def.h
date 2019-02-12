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

#ifndef MX_MUX_MAXIUM_TRACKID
#define MX_MUX_MAXIUM_TRACKID (2)
#endif

#ifndef MX_MUX_BUFFSIZE
#define MX_MUX_BUFFSIZE (512)
#endif

#ifndef MX_LOOP_INTERVAL
#define MX_LOOP_INTERVAL (20)
#endif