#include "MX_def.h"
#include "PARAM_def.h"
#include <stdbool.h>

typedef int16_t* triggerSets_BG_t;
typedef int16_t* triggerSets_FT_t;
typedef int16_t* triggerSets_TG_t;

MX_INTERNAL_API void triggerSets_freeBG(triggerSets_BG_t);
MX_INTERNAL_API void triggerSets_freeTG(triggerSets_TG_t);
MX_INTERNAL_API void triggerSets_freeFT(triggerSets_FT_t);

MX_C_API triggerSets_BG_t triggerSets_readBG(const char* filePath);
MX_C_API triggerSets_TG_t triggerSets_readTG(const char* filePath);
MX_C_API triggerSets_FT_t triggerSets_readFT(const char* filePath);

MX_C_API int16_t triggerSets_getGB(triggerSets_BG_t, const char*);
MX_C_API int16_t triggerSets_getTG(triggerSets_TG_t, const char*);
MX_C_API int16_t triggerSets_getFT(triggerSets_FT_t, const char*);
