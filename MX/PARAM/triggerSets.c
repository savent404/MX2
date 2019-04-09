#include "triggerSets.h"
#include "ff.h"
#include "re.h"
#include "FreeRTOS.h"
#include "debug.h"
#include <stdlib.h>

#define MAX_STR_LEN (24)
static const char BG[][MAX_STR_LEN] =
{
    "MODE",
    "T_MC",
    "T_SC",
    "T_MS",
    "T_SM",
    "NP_BREATHCYCLE",
    "NP_TSPARK",
    "NP_SPARKDENSITY",
    "RAINBOWLENGTH",
    "RAINBOWDIRECTION",
    "RAINBOWSPEED",
    "FLAMERATE",
    "FLAMEFREQ",
    "FLAMEMULTI",
    "NP_GLength",
    "NP_Gspeed",
    "NP_Gdirection",
};

static const char TG[][MAX_STR_LEN] = 
{
    "MODE",
    "NP_FLIPCOLOR",
    "NP_TFLIP",
    "NP_MAXFLIPCOUNT",
    "NP_FLIPLENTH",
    "NP_CDRIFT",
    "NP_TDRIFT",
    "NP_AccN",
    "NP_SpeardMode",
    "NP_SpeardLength",
    "NP_SpeardSpeed",
    "NP_SpeardLocation",
    "NP_MaxFlipCount",
    "NP_CometMode",
    "NP_CometType",
    "NP_CometLength",
    "NP_CometRange",
    "NP_CometSpeed",
    "NP_CometLocation",
};

static const char FT[][MAX_STR_LEN] =
{
    "MODE",
    "NP_TBREATH",
    "NP_BRIGHTMAX",
    "NP_BRIGHTMIN",
    "NP_TFLICKER",
    "NP_MAXFLIPCOUNT",
    "NP_FLIPLENTH",
    "NP_WAVELENGTH",
    "NP_WAVESPEED",
    "NP_WAVECOUNT",
    "NP_FADEPOSITION",
    "NP_FADEDIRECTION",
    "NP_TFADE",
};

static const char HW[][MAX_STR_LEN] = 
{
    "NP_ORDER",
    "NP_NUM",
    "NP_PERIOD",
    "NP_V0PULSE",
    "NP_V1PULSE",
    "NP_RSTPULSE",
};

static inline bool getKeyVal(const char* in, char* name, char* val)
{
    static bool inited = false;
    static re_t match;
    if (inited == false)
    {
        inited = true;
        match = re_compile("\\w+\\s*=\\s*\\d+");
    }

    if (re_matchp(match, in) == -1)
        return false;

    const char *_p = in;

    // note: if u type 'a b c=....' you will get 'abc=...'
    while (*_p != '=' && *_p != '\0')
    {
        char t = *_p++;
        if (t == ' ')
            continue;
        if (t <= 'Z' && t >= 'A' ||
            t <= 'z' && t >= 'a' ||
            t == '_')
            *name++ = t;
    }
    *name = '\0';
    while (*_p != '/' && *_p != '\0')
    {
        char t = *_p++;
        if (t == ' ')
            continue;
        if ((t <= '9' && t >= '0') || t == ',' || t == '.')
            *val++ = t;
    }
    *val = '\0';
    return true;
}

static inline int findPosition(const char* name, const char* strArr, int strNum)
{
    for (int i = 0; i < strNum; i++)
    {
        const char* str = strArr + i*MAX_STR_LEN;
        if (!strncasecmp(name, str, MAX_STR_LEN - 1))
            return i;
    }
    return -1;
}
static inline void matchWrite(int16_t* arr, const char* inputLine, const char* strArr, int strNum)
{
    static char name[32], value[32];
    if (!getKeyVal(inputLine, name, value))
        return;
    // find this name in string arr
    int pos = findPosition(name, strArr, strNum);
    if (pos < 0)
        return;
    arr[pos] = (int16_t)atoi(value);
    return;
}

static int16_t *allocStructure(int size)
{
    int16_t *ptr = (int16_t*)pvPortMalloc(size * sizeof(int16_t));
    for (int i = 0; i < size; i++)
    {
        ptr[i] = -1;
    }
    return ptr;
}

static void freeStructure(int16_t* ptr)
{
    vPortFree(ptr);
}

void triggerSets_freeBG(triggerSets_BG_t bg)
{
    vPortFree(bg);
}
void triggerSets_freeTG(triggerSets_TG_t tg)
{
    vPortFree(tg);
}
void triggerSets_freeFT(triggerSets_FT_t ft)
{
    vPortFree(ft);
}
void triggerSets_freeHW(triggerSets_HW_t hw)
{
    vPortFree(hw);
}

triggerSets_BG_t triggerSets_readBG(const char* filePath)
{
    FIL file;
    FRESULT res;

    int16_t* a = allocStructure(sizeof(BG) / sizeof(BG[0]));
    char lineBuffer[128];
    res = f_open(&file, filePath, FA_READ);
    DEBUG_IF(res != FR_OK, 5, "Cant open file:%s", filePath);
    if (res != FR_OK)
        return a;
    while (f_gets(lineBuffer, sizeof(lineBuffer), &file) != 0)
    {
        matchWrite(a, lineBuffer, BG[0], sizeof(BG) / sizeof(BG[0]));
    }
    f_close(&file);
    return a;

}
triggerSets_TG_t triggerSets_readTG(const char* filePath)
{
    FIL file;
    FRESULT res;

    int16_t* a = allocStructure(sizeof(TG) / sizeof(TG[0]));
    char lineBuffer[128];
    res = f_open(&file, filePath, FA_READ);
    DEBUG_IF(res != FR_OK, 5, "Cant open file:%s", filePath);
    if (res != FR_OK)
        return a;
    while (f_gets(lineBuffer, sizeof(lineBuffer), &file) != 0)
    {
        matchWrite(a, lineBuffer, TG[0], sizeof(TG) / sizeof(TG[0]));
    }
    f_close(&file);
    return a;
}
triggerSets_FT_t triggerSets_readFT(const char* filePath)
{
    FIL file;
    FRESULT res;

    int16_t* a = allocStructure(sizeof(FT) / sizeof(FT[0]));
    char lineBuffer[128];
    res = f_open(&file, filePath, FA_READ);
    DEBUG_IF(res != FR_OK, 5, "Cant open file:%s", filePath);
    if (res != FR_OK)
        return a;
    while (f_gets(lineBuffer, sizeof(lineBuffer), &file) != 0)
    {
        matchWrite(a, lineBuffer, FT[0], sizeof(FT) / sizeof(FT[0]));
    }
    f_close(&file);
    return a;
}
triggerSets_HW_t triggerSets_readHW(const char* filePath)
{
    FIL file;
    FRESULT res;

    int16_t* a = allocStructure(sizeof(HW) / sizeof(HW[0]));
    char lineBuffer[128];
    res = f_open(&file, filePath, FA_READ);
    DEBUG_IF(res != FR_OK, 5, "Cant open file:%s", filePath);
    if (res != FR_OK)
        return a;
    while (f_gets(lineBuffer, sizeof(lineBuffer), &file) != 0)
    {
        matchWrite(a, lineBuffer, HW[0], sizeof(HW) / sizeof(HW[0]));
    }
    f_close(&file);
    return a;
}

int16_t triggerSets_getBG(triggerSets_BG_t p, const char* name)
{
    int pos = findPosition(name, BG[0], sizeof(BG)/sizeof(BG[0]));
    if (pos < 0)
        return -1;
    return ((int16_t*)p)[pos];
}

int16_t triggerSets_getTG(triggerSets_TG_t p, const char* name)
{
    int pos = findPosition(name, TG[0], sizeof(TG) / sizeof(TG[0]));
    if (pos < 0)
        return -1;
    return ((int16_t*)p)[pos];
}

int16_t triggerSets_getFT(triggerSets_FT_t p, const char* name)
{
    int pos = findPosition(name, FT[0], sizeof(FT) / sizeof(FT[0]));
    if (pos < 0)
        return -1;
    return ((int16_t*)p)[pos];
}
int16_t triggerSets_getHW(triggerSets_HW_t p, const char* name)
{
    int pos = findPosition(name, HW[0], sizeof(HW) / sizeof(HW[0]));
    if (pos < 0)
        return -1;
    return ((int16_t*)p)[pos];
}
