#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef USE_DEBUG
#define DEBUG(level, format, ...)                                                                 \
  {                                                                                               \
    if (level < 3)                                                                                \
      __ASM("BKPT 0");                                                                            \
    printf("[%02d]: " format "\tFile:%s\tLine:%d\r\n", level, ##__VA_ARGS__, __FILE__, __LINE__); \
  }
#else
#define DEBUG(level, format, ...) ;
#endif

#endif
