#ifndef _LED_H_
#define _LED_H_

#include "MX_def.h"
#include "param.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cmsis_os.h"

#include "LED_def.h"

extern LED_IF_t ledIf;

MX_C_API osEvent MX_LED_GetMessage(uint32_t timeout);
MX_C_API void MX_LED_startTrigger(LED_Message_t message);
MX_C_API void MX_LED_bankUpdate(PARA_DYNAMIC_t*);
MX_C_API void MX_LED_Init(void);

#endif