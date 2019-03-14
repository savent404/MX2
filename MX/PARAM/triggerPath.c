#include "triggerPath.h"
#include "FreeRTOS.h"
#include "ff.h"
#include "re.h"
#include <stdbool.h>
#include <string.h>

static const int strFixedLen = 32;

static const char* getRegex(TRIGGERPATH_Type_t type)
{
    switch (type)
    {
        case TRIGGERPATH_WAV:
            return "[^.]+.[Ww][Aa][Vv]";
        case TRIGGERPATH_BG:
            return "[^.]+.[Bb][Gg]";
        case TRIGGERPATH_TG:
            return "[^.]+.[Tt][Gg]";
        case TRIGGERPATH_FT:
            return "[^.]+.[Ff][Tt]";
    }
    return "";
}
static TRIGGER_PATH_t* allocPointer()
{
    TRIGGER_PATH_t* dest = (TRIGGER_PATH_t*)pvPortMalloc(sizeof(*dest));

    dest->number = 0;
    dest->prefix = NULL;
    dest->path_arry = NULL;

    return dest;
}

static void clearPointer(const TRIGGER_PATH_t* dest)
{
    if (dest->prefix != NULL) {
        vPortFree(dest->prefix);
    }
    if (dest->path_arry != NULL) {
        vPortFree(dest->path_arry);
    }
    vPortFree((void*)dest);
}

TRIGGER_PATH_t* MX_TriggerPath_Init(const char* dirPath, int maxNum, TRIGGERPATH_Type_t type)
{
    DIR dir;
    FILINFO info;
    int cnt = 0;
    enum {stage_1, stage_2} stage = stage_1;
    re_t match_s = re_compile(getRegex(type));
    bool matched = false;
    TRIGGER_PATH_t* dest = allocPointer();
again:
    if (f_opendir(&dir, dirPath) != FR_OK) {
        goto failed;
    }
    while (f_readdir(&dir, &info) == FR_OK && info.fname[0] != '\0')
    {
        matched = re_matchp(match_s, info.fname) != -1;

        if (matched && stage == stage_1)
            cnt++;
        else if (matched && stage == stage_2 && cnt--)
            strncpy(dest->path_arry + cnt*strFixedLen, info.fname, strFixedLen - 1);
    }
    f_closedir(&dir);

    if (stage == stage_1) {
        dest->path_arry = (char*)pvPortMalloc(cnt * strFixedLen);
        dest->prefix = (char*)pvPortMalloc(strlen(dirPath) + 1);
        strcpy(dest->prefix, dirPath);
        cnt = cnt > maxNum ? maxNum : cnt;
        dest->number = cnt;
        stage = stage_2;
        goto again;
    }
    return dest;
failed:
    clearPointer(dest);
    return NULL;
}

void MX_TriggerPath_DeInit(const TRIGGER_PATH_t* ptr)
{
    clearPointer(ptr);
}

int MX_TriggerPath_getNum(const TRIGGER_PATH_t* ptr)
{
    return ptr->number;
}

const char* MX_TriggerPath_GetName(const TRIGGER_PATH_t* ptr, int pos)
{
    if (pos < 0 || pos >= ptr->number)
        return NULL;
    return ptr->path_arry + pos * strFixedLen;
}

const char* MX_TriggerPath_GetPrefix(const TRIGGER_PATH_t* ptr)
{
    return ptr->prefix;
}

static inline int c_find(const char* in, char c)
{
    int cnt = 0;
    while (*in != ' ' && *in != c)
    {
        cnt++;
        in++;
    }
    return cnt;
}

int MX_TriggerPath_HasSame(const TRIGGER_PATH_t* ptr, const char* name)
{
    int num = MX_TriggerPath_getNum(ptr);
    int strlen = c_find(name, '.');
    const char* cmp;
    for (int i = 0; i < num; i++)
    {
        cmp = MX_TriggerPath_GetName(ptr, i);
        if (!strncasecmp(cmp, name, strlen))
            return i;
    }
    return -1;
}

#include <stdio.h>
#include <stdlib.h>
const char* _MX_TriggerPath_getOtherPath(const TRIGGER_PATH_t* ptr, const char* name)
{
    static char path[128];
    int res;
    if ((res = MX_TriggerPath_HasSame(ptr, name)) < 0)
        res = rand() % MX_TriggerPath_getNum(ptr);
    sprintf(path, "%s/%s", MX_TriggerPath_GetPrefix(ptr),
                           MX_TriggerPath_GetName(ptr, res));
    return path;
}
