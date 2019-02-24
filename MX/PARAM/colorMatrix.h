#pragma once

#include "PARAM_def.h"
#include "MX_def.h"
#include "ff.h"
#include "debug.h"
#include "re.h"
#include <stdbool.h>

static inline void
MX_ColorMatrix_Free(colorMatrix_t* ptr)
{
    if (ptr->num != 0 && ptr->arr)
    {
        vPortFree(ptr->arr);
        ptr->arr = NULL;
        ptr->num = 0;
    }
}

static inline void
MX_ColorMatrix_Alloc(colorMatrix_t* ptr, int num)
{
    MX_ColorMatrix_Free(ptr);
    ptr->num = num;
    ptr->arr = (RGBColor_t*)pvPortMalloc(sizeof(RGBColor_t) * num);
}

static inline bool
MX_ColorMatrix_Update(const char* path, colorMatrix_t* ptr)
{
    FIL file;
    FRESULT res;
    char buffer[64];
    re_t match_p = re_compile("\\s*\\d+\\s*,\\s*\\d+\\s*,\\s*\\d+\\s*");
    enum { stage_1, stage_2 } stage = stage_1;
    int cnt = 0;
    if ((res = f_open(&file, path, FA_READ)) != FR_OK)
    {
        DEBUG(3, "Can't open file:%s error:%d", path, (int)res);
        return false;
    }
again:
    while (f_gets(buffer, sizeof(buffer), &file) != 0) {
        bool matched = re_matchp(match_p, buffer) != -1;
        if (stage == stage_1 && matched) {
            cnt++;
        }
        else if (stage == stage_2 && matched) {
            int ans[3];
            sscanf(buffer, "%d,%d,%d", &ans[0], &ans[1], &ans[2]);
            for (int i = 0; i < 3; i++)
            {
                if (ans[i] > 255)
                    ans[i] = 255;
                else if (ans[i] < 0)
                    ans[i] = 0;
                ptr->arr[cnt].arr[i] = ans[i];
            }
            cnt++;
        }
    }

    if (stage == stage_1) {
        f_lseek(&file, 0);
        stage = stage_2;
        MX_ColorMatrix_Alloc(ptr, cnt);
        cnt = 0;
        goto again;
    }
    f_close(&file);
    return true;
}

static inline bool
MX_ColorMatrix_isOutOfRange(const colorMatrix_t* ptr, int pos)
{
    return ptr->num <= pos;
}