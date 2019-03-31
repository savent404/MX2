#ifndef USR_CONFIG_H_
#define USR_CONFIG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "MX_def.h"
#include "param.h"
#include "colorMatrix.h"
#include "triggerSets.h"

extern PARA_DYNAMIC_t USR;
extern const PARA_STATIC_t STATIC_USR;
MX_C_API uint8_t usr_config_init(void);
MX_C_API uint8_t usr_switch_bank(int dest, int freshFlag);
MX_C_API uint8_t usr_init_bank(int bankPos, int storagePos);
MX_C_API uint8_t usr_update_triggerPah(int bankPos, bool isColorSwitch);
#define TRIGGER_MAX_NUM(x) (STATIC_USR.filelimits.trigger_##x##_max)

#endif
