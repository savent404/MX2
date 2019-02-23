#include "console.h"

__MX_WEAK void MX_Console_Print(uint8_t *string, uint16_t size) {}

int MX_Console_Printf(const char *format, ...) {

    static char buffer[512] = { 0 };
    uint16_t res;
    va_list args;
    va_start(args, format);
    res = (uint16_t)vsprintf(buffer, format, args);
    MX_Console_Print((uint8_t*)buffer, res);
    va_end(args);
    return res;
}