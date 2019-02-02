#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#ifdef USE_DEBUG
#define DEBUG(level, format, ...) {if (level < 3)__ASM("BKPT 0");printf("[%02d]: "format"\tFile:%s\tLine:%d\r\n", level, ##__VA_ARGS__,__FILE__,__LINE__);}
#else
#define DEBUG(level, format, ...) ;
#endif

#define _Error_Handler(fileName, Line) DEBUG(0, "Error occur File:%s\tLine:%d", fileName, Line)
#endif
