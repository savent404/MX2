#pragma once

#include "MX_def.h"
#include "PARAM_def.h"

#include <stdbool.h>

typedef enum TRIGGERPATH_Type_t {
    TRIGGERPATH_WAV,
    TRIGGERPATH_PAIRWAV,
    TRIGGERPATH_BG,
    TRIGGERPATH_FT,
    TRIGGERPATH_TG,
} TRIGGERPATH_Type_t;

MX_INTERNAL_API TRIGGER_PATH_t*
MX_TriggerPath_Init(const char* dirPath, int maxnum, TRIGGERPATH_Type_t type);

MX_INTERNAL_API void
MX_TriggerPath_DeInit(TRIGGER_PATH_t* ptr);

MX_INTERNAL_API int
MX_TriggerPath_getNum(const TRIGGER_PATH_t* dest);

MX_INTERNAL_API const char*
MX_TriggerPath_GetName(const TRIGGER_PATH_t*, int pos);

/**
 * @brief return <name>_H.wav
 * @param pos
 * @return string
 * @note returned string is not safe, use it quick
 */
MX_INTERNAL_API const char*
MX_TriggerPath_GetPairHName(const TRIGGER_PATH_t*, int pos);

/**
 * @brief return <name>_H.wav
 * @param pos
 * @return string
 * @note returned string is not safe, use it quick
 */
MX_INTERNAL_API const char*
MX_TriggerPath_GetPairLName(const TRIGGER_PATH_t*, int pos);

MX_INTERNAL_API const char*
MX_TriggerPath_GetPrefix(const TRIGGER_PATH_t*);

MX_INTERNAL_API int
MX_TriggerPath_HasSame(const TRIGGER_PATH_t*, const char* name);

MX_INTERNAL_API const char*
_MX_TriggerPath_getOtherPath(const TRIGGER_PATH_t*, const char*);
