#pragma once
#include "FreeRTOS.h"
#include "ff.h"

static inline FILINFO* fatfs_allocFileInfo(void)
{
    FILINFO* info = (FILINFO*)pvPortMalloc(sizeof(*info));
#if _USE_LFN
    info->lfname = pvPortMalloc(64);
#endif
    return info;
}

static inline void fatfs_freeFileInfo(FILINFO* ptr)
{

#if _USE_LFN
    vPortFree(ptr->lfname);
#endif
    vPortFree(ptr);
}
