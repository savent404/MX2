#pragma once

#include "MX_def.h"
#include "cmsis_os.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CLI.h"

#ifdef __cplusplus
#    include "event.h"
#endif

MX_PORT_API void  MX_Console_Print(uint8_t* string, uint16_t size);
MX_PORT_API char* MX_Console_Gets(char* buffer, int maxiumSize);

MX_C_API int  MX_Console_Printf(const char*, ...);
MX_C_API void MX_Console_Scanf(char*, ...);

MX_C_API void MX_Console_Init(void);
MX_C_API void MX_Console_Handle(void const*);

MX_INTERNAL_API BaseType_t CLI_tirgger(char*, size_t, const char*);

MX_C_API void MX_ConsoleTX_WaitMutex(void);
MX_C_API void MX_ConsoleTX_FreeMutex(void);
MX_C_API void MX_ConsoleRX_WaitMutex(void);
MX_C_API void MX_ConsoleRX_FreeMutex(void);
MX_C_API void MX_ConsoleTX_WaitSem(void);
MX_C_API void MX_ConsoleTX_FreeSem(void);
MX_C_API void MX_ConsoleRX_WaitSem(void);
MX_C_API void MX_ConsoleRX_FreeSem(void);