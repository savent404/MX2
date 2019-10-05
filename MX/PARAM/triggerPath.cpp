#include "triggerPath.h"
#include "FreeRTOS.h"
#include "ff.h"
#include "re.h"
#include <stdbool.h>
#include <string.h>

#include "fatfs_ultis.h"

static const int strFixedLen = 32;

static char* returnInArray(char* array, int pos)
{
    return array + pos * strFixedLen;
}
static const char* returnInArray(const char* array, int pos)
{
    return array + pos * strFixedLen;
}

static const char* getRegex(TRIGGERPATH_Type_t type)
{
    switch (type) {
    case TRIGGERPATH_WAV:
        return "[^.]+.[Ww][Aa][Vv]";
    case TRIGGERPATH_BG:
        return "[^.]+.[Bb][Gg]";
    case TRIGGERPATH_TG:
        return "[^.]+.[Tt][Gg]";
    case TRIGGERPATH_FT:
        return "[^.]+.[Ff][Tt]";
    case TRIGGERPATH_PAIRWAV:
        return "^[^_]+[_][LlHh].[Ww][Aa][Vv]";
    }
    return "";
}
static TRIGGER_PATH_t* allocPointer()
{
    TRIGGER_PATH_t* dest = (TRIGGER_PATH_t*)pvPortMalloc(sizeof(*dest));

    dest->number    = 0;
    dest->prefix    = nullptr;
    dest->path_arry = nullptr;

    return dest;
}

static void clearPointer(TRIGGER_PATH_t* dest)
{
    if (dest->prefix != nullptr) {
        delete[] dest->prefix;
        dest->prefix = nullptr;
    }
    if (dest->path_arry != nullptr) {
        delete[] dest->path_arry;
        dest->path_arry = nullptr;
    }
    vPortFree((void*)dest);
}

static TRIGGER_PATH_t* pairMatch(TRIGGER_PATH_t* ptr);

TRIGGER_PATH_t* MX_TriggerPath_Init(const char* dirPath, int maxNum, TRIGGERPATH_Type_t type)
{
    DIR      dir;
    FILINFO* info = fatfs_allocFileInfo();
    int      cnt  = 0;
    enum { stage_1,
           stage_2 } stage
        = stage_1;
    re_t            match_s = re_compile(getRegex(type));
    bool            matched = false;
    TRIGGER_PATH_t* dest    = allocPointer();
again:
    if (f_opendir(&dir, dirPath) != FR_OK) {
        goto failed;
    }
    while (f_readdir(&dir, info) == FR_OK && info->fname[ 0 ] != '\0') {
        matched = re_matchp(match_s, info->fname) != -1;

        if (matched && stage == stage_1)
            cnt++;
        else if (matched && stage == stage_2 && cnt--)
            strncpy(returnInArray(dest->path_arry, cnt), info->fname, strFixedLen - 1);
    }
    f_closedir(&dir);

    if (stage == stage_1) {
        dest->path_arry = cnt ? (char*)pvPortMalloc(cnt * strFixedLen) : nullptr;
        dest->prefix    = (char*)pvPortMalloc(strlen(dirPath) + 1);
        strcpy(dest->prefix, dirPath);
        cnt          = cnt > maxNum ? maxNum : cnt;
        dest->number = cnt;
        stage        = stage_2;
        goto again;
    }
    fatfs_freeFileInfo(info);
    vPortFree(match_s);
    // need to find 'L' & 'H' pair
    if (TRIGGERPATH_PAIRWAV == type) {
        dest = pairMatch(dest);
    }
    return dest;
failed:
    fatfs_freeFileInfo(info);
    clearPointer(dest);
    vPortFree(match_s);
    return nullptr;
}

void MX_TriggerPath_DeInit(TRIGGER_PATH_t* ptr)
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
        return nullptr;
    return returnInArray(ptr->path_arry, pos);
}

const char* MX_TriggerPath_GetPrefix(const TRIGGER_PATH_t* ptr)
{
    return ptr->prefix;
}

static inline int c_find(const char* in, char c)
{
    int cnt = 0;
    while (*in != ' ' && *in != c) {
        cnt++;
        in++;
    }
    return cnt;
}

int MX_TriggerPath_HasSame(const TRIGGER_PATH_t* ptr, const char* name)
{
    int         num    = MX_TriggerPath_getNum(ptr);
    int         strlen = c_find(name, '.');
    const char* cmp;
    for (int i = 0; i < num; i++) {
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
    static char path[ 128 ];
    int         res;
    if (MX_TriggerPath_getNum(ptr) == 0)
        return "";
    if ((res = MX_TriggerPath_HasSame(ptr, name)) < 0)
        res = rand() % MX_TriggerPath_getNum(ptr);
    sprintf(path, "%s/%s", MX_TriggerPath_GetPrefix(ptr),
            MX_TriggerPath_GetName(ptr, res));
    return path;
}

static char pairName[ 64 ];

const char* MX_TriggerPath_GetPairHName(const TRIGGER_PATH_t* ptr, int pos)
{
    sprintf(pairName, "%s_H.WAV", returnInArray(ptr->path_arry, pos));
    return pairName;
}
const char* MX_TriggerPath_GetPairLName(const TRIGGER_PATH_t* ptr, int pos)
{
    sprintf(pairName, "%s_L.WAV", returnInArray(ptr->path_arry, pos));
    return pairName;
}

static TRIGGER_PATH_t* pairMatch(TRIGGER_PATH_t* ptr)
{
    TRIGGER_PATH_t* ptrNew    = allocPointer();
    re_t            match_s   = re_compile("^[^_]+_[HhLl]\\.[Ww][Aa][Vv]");
    int             matchedId = -1;
    char            nameBuffer[ strFixedLen ];
    int*            id = new int[ ptr->number ];

    for (int i = 0; i < ptr->number; i++) {
        id[ i ] = -1;
    }

    for (int i = 0; i < ptr->number; i++) {
        char* name = returnInArray(ptr->path_arry, i);

        // if matched , skip
        if (id[ i ] != -1) {
            continue;
        }

        // match H first
        auto res = re_matchp(match_s, name);
        if (res == -1)
            continue;

        sscanf(name, "%[^_]s", nameBuffer);
        auto c = strlen(nameBuffer) + 1;
        if (name[ c ] == 'H' || name[ c ] == 'h') {

        } else {
            continue;
        }

        // search L pair in the result
        for (int j = 0; j < ptr->number; j++) {
            if (i == j)
                continue;
            char* otherName = returnInArray(ptr->path_arry, j);
            // if matched
            if (re_matchp(match_s, otherName) != -1) {
                if (!strncmp(nameBuffer, otherName, strlen(nameBuffer))) {
                    matchedId++;
                    id[ i ] = matchedId;
                    id[ j ] = matchedId;
                    break;
                }
            }
        }
    }
    if (matchedId >= 0) {
        // alloc a new pathTrigger
        matchedId += 1;
        ptrNew->path_arry = new char[ matchedId * strFixedLen ];
        ptrNew->prefix    = new char[ strlen(ptr->prefix) + 1 ];
        strcpy(ptrNew->prefix, ptr->prefix);
        ptrNew->number = matchedId;

        // fill path
        int cnt = 0;
        for (int i = 0; i < ptr->number; i++) {
            for (int j = 0; j < ptr->number; j++) {
                if (id[ i ] != id[ j ] || i == j || id[ i ] == -1)
                    continue;
                sscanf(returnInArray(ptr->path_arry, i), "%[^_]s", nameBuffer);
                auto len = strlen(nameBuffer);
                len      = len >= strFixedLen - 1 ? strFixedLen : len;
                strncpy(returnInArray(ptrNew->path_arry, cnt++), nameBuffer, len);
                id[ i ] = -1;
                id[ j ] = -1;
            }
        }
    }
    delete[] id;
    clearPointer(ptr);
    vPortFree(match_s);
    return ptrNew;
}
