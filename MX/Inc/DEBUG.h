#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "elog.h"
#include "mx-config.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef EBmonitorBufLen
#define EBmonitorBufLen 0x800
#endif

#ifdef USE_DEBUG

extern char EBmonitorBuf[EBmonitorBufLen];
void EBmonitor_buffer(FILE *, char *, uint16_t);
void EBmonitor_flush(FILE *);
int EBmonitor_kbhit();

#define EBmonitor_Init()                                        \
  {                                                             \
    EBmonitor_buffer(stdout, EBmonitorBuffer, EBmonitorBufLen); \
    elog_init();                                                \
    elog_start();                                               \
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);                \
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_ALL);                  \
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_ALL);                  \
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL);                 \
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL);               \
  \
}
#define DEBUG(level, format, ...)                                                                   \
  {                                                                                                 \
    if (level < 10)                                                                                 \
                                                                                                    \
      printf("[%02d]: " format "\tFile:%s\tLine:%d\r\n", level, ##__VA_ARGS__, __FILE__, __LINE__); \
  }
#else
#define DEBUG(level, format, ...) ;
#define EBmonitor_Init() ;
#undef EBmonitorBufLen
#define EBmonitorBufLen 0
#endif

#endif
