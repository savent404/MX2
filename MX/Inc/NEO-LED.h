#ifndef _NEO_LED_H_
#define _NEO_LED_H_

#ifndef LOG_TAG
#define LOG_TAG "neo-led"
#endif

#include "DEBUG.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "path.h"
#include "mx-config.h"
#include "mx-file.h"
#include "mx-led.h"
#include "mx-spi.h"
#include "neopixel-drive.h"

extern const LED_Opra_t NEO_LED_Opra;
extern SPI_HandleTypeDef* NEO_LED_LL_Init(void);
#endif
