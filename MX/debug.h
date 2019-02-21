#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef USE_DEBUG
#define DEBUG(level, format, ...) ;
#define DEBUG_BKPT() __ASM("BKPT 0")
#else
#define DEBUG(level, format, ...) ;
#define DEBUG_BKPT()
#endif

#ifndef _Error_Handler
#define _Error_Handler(fileName, Line) DEBUG(0, "Error occur File:%s\tLine:%d", fileName, Line)
#endif

