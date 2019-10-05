#include "MX_def.h"
#include "PARAM_def.h"
#include <stdbool.h>

MX_INTERNAL_API void triggerSets_freeBG(triggerSets_BG_t);
MX_INTERNAL_API void triggerSets_freeTG(triggerSets_TG_t);
MX_INTERNAL_API void triggerSets_freeFT(triggerSets_FT_t);
MX_INTERNAL_API void triggerSets_freeHW(triggerSets_HW_t);

MX_C_API triggerSets_BG_t triggerSets_readBG(const char* filePath);
MX_C_API triggerSets_TG_t triggerSets_readTG(const char* filePath);
MX_C_API triggerSets_FT_t triggerSets_readFT(const char* filePath);
MX_C_API triggerSets_HW_t triggerSets_readHW(const char* filePath);

MX_C_API int16_t triggerSets_getBG(triggerSets_BG_t, const char*);
MX_C_API int16_t triggerSets_getTG(triggerSets_TG_t, const char*);
MX_C_API int16_t triggerSets_getFT(triggerSets_FT_t, const char*);
MX_C_API int16_t triggerSets_getHW(triggerSets_HW_t, const char*);
