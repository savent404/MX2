#ifndef _LED_H_
#define _LED_H_

#include "MX_def.h"
#include "param.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cmsis_os.h"

#include "LED_def.h"
#include "PARAM_def.h"

extern LED_IF_t ledIf;

MX_C_API osEvent MX_LED_GetMessage(uint32_t timeout);

/**
 * @brief the API to tell LED handler trigger is started
 * @note  If enabled Follow Audio last time, this API should call
 *        after started trigger's audio
 */
MX_C_API void MX_LED_startTrigger(LED_CMD_t message);
MX_C_API void MX_LED_bankUpdate(PARA_DYNAMIC_t*);
MX_C_API void MX_LED_updateBG(triggerSets_BG_t);
MX_C_API void MX_LED_updateTG(triggerSets_TG_t);
MX_C_API void MX_LED_updateFT(triggerSets_FT_t);
MX_C_API void MX_LED_applySets(void);
MX_C_API void MX_LED_Init(void);

#endif
