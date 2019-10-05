#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "debug.h"
#include <stdlib.h>

void* operator new(size_t size)
{
    auto ptr = pvPortMalloc(size_t(size));

    if (osKernelRunning() != 1)
        DEBUG(0, "NEW operator should run after freeRTOS started");
    if (ptr == nullptr)
        DEBUG(0, "NEW operator error");
    return ptr;
}

void* operator new(size_t size, const char* file, int line)
{
    DEBUG(5, "NEW:\tsize:%ud\tfile:%s\tline:%d", size, file, line);
    auto ptr = pvPortMalloc(size_t(size));

    if (osKernelRunning() != 0)
        DEBUG(0, "NEW operator should run after freeRTOS started");
    if (ptr == nullptr)
        DEBUG(0, "NEW operator error");
    return ptr;
}

void operator delete(void* ptr)
{
    vPortFree(ptr);
}
void operator delete[](void* ptr)
{
    vPortFree(ptr);
}