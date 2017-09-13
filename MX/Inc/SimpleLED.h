#ifndef _SIMPLELED_H_
#define _SIMPLELED_H_

#include "DEBUG.h"
#include "USR_CONFIG.h"
#include "cmsis_os.h"
#include "mx-gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  SIMPLELED_STATUS_SLEEP,
  SIMPLELED_STATUS_STANDBY,
  SIMPLELED_STATUS_ON,
  SIMPLELED_STATUS_LOCKUP,
  SIMPLELED_STATUS_CLASH
} SimpleLED_Status_t;

void SimpleLED_Init(void);
void SimpleLED_DeInit(void);
void SimpleLED_Callback(void const *arg);
void SimpleLED_ChangeStatus(SimpleLED_Status_t status);

#endif
