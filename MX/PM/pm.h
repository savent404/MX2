#pragma once

#include "MX_def.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Init Power manger system
 * @note  enable battery supply when inited
 */
MX_PORT_API bool MX_PM_Init(void);
MX_PORT_API bool MX_PM_DeInit(void);

MX_PORT_API void MX_PM_Boot(void);
MX_PORT_API void MX_PM_Shutdown(void);

MX_PORT_API bool MX_PM_needWarning(void);
MX_PORT_API bool MX_PM_needPowerOff(void);

/**
 * @note if pluged, then it's charging
 */
MX_PORT_API bool MX_PM_isCharging(void);
/**
 * @note pluged & power level upper than threshold
 */
MX_PORT_API bool MX_PM_isCharged(void);
/**
 * @note Get current battery capacity
 */
MX_PORT_API bool MX_PM_CurrentCapacity(uint16_t *permillage);
