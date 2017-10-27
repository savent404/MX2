#ifndef _MX_CONFIG_H_
#define _MX_CONFIG_H_

#include "main.h"

#ifndef AUDIO_SOFTMIX
#error "Undefined configuration parameter: AUDIO_SOFTMIX"
#define AUDIO_SOFTMIX 1
#endif

#ifndef USE_NEOPIXEL
#define USE_NEOPIXEL 0
#error "Undefined configuration parameter: USE_NEOPIXEL"
#endif


#endif
