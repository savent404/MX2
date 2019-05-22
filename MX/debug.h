#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_DEBUG
#    include "console.h"
#    define DEBUG_BKPT() __ASM("BKPT 0")
#    define DEBUG(level, format, ...)                                     \
        do {                                                              \
            if (level <= 3) {                                             \
                DEBUG_BKPT();                                             \
            }                                                             \
            MX_Console_Printf_stderr("[%02d]: " format "\tFile:%s\tLine:%d\r\n", \
                              level, ##__VA_ARGS__, __FILE__, __LINE__);  \
        } while (0);

#    define DEBUG_IF(exp, level, format, ...)    \
        if (exp) {                               \
            DEBUG(level, format, ##__VA_ARGS__); \
        }

#else
#    define DEBUG(level, format, ...) ;
#    define DEBUG_BKPT()
#    define DEBUG_IF(exp, level, format, ...) ;
#endif

#ifndef _Error_Handler
#    define _Error_Handler(fileName, Line) DEBUG(0, "Error occur File:%s\tLine:%d", fileName, Line)
#endif

#define DEBUG_ASSERT(exp) \
    if (!exp) {           \
        DEBUG_BKPT();     \
    }
