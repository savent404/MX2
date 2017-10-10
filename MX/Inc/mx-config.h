#ifndef _MX_CONFIG_H_
#define _MX_CONFIG_H_

#include "main.h"

#ifndef AUDIO_SOFTMIX
#error "Undefined configuration parameter: AUDIO_SOFTMIX"
#define AUDIO_SOFTMIX 1
#endif

#endif
