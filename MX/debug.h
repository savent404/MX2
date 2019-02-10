#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef USE_DEBUG
#define DEBUG(level, format, ...) {if (level < 3)__ASM("BKPT 0");printf("[%02d]: "format"\tFile:%s\tLine:%d\r\n", level, ##__VA_ARGS__,__FILE__,__LINE__);}
#define DEBUG_BKPT() __ASM("BKPT 0")
#else
#define DEBUG(level, format, ...) ;
#define DEBUG_BKPT()
#endif

#ifndef _Error_Handler
#define _Error_Handler(fileName, Line) DEBUG(0, "Error occur File:%s\tLine:%d", fileName, Line)
#endif

