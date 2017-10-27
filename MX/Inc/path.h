#ifndef _PATH_H_
#define _PATH_H_

#include "main.h"
#include "mx-config.h"

#ifndef PATH_CONFIG
#define PATH_CONFIG "0://SETTING.txt"
#endif

#ifndef WAV_BOOT
#define WAV_BOOT "0:/System/Boot.wav"
#endif

#ifndef WAV_CHARGING
#define WAV_CHARGING "0:/System/charging.wav"
#endif

#ifndef WAV_COLORSWITCH
#define WAV_COLORSWITCH "0:/System/ColorSwitch.wav"
#endif

#ifndef WAV_LOWPOWER
#define WAV_LOWPOWER "0:/System/lowpower.wav"
#endif

#ifndef WAV_POWEROFF
#define WAV_POWEROFF "0:/System/Poweroff.wav"
#endif

#ifndef WAV_RECHARGE
#define WAV_RECHARGE "0:/System/Recharge.wav"
#endif

#ifndef WAV_ERROR
#define WAV_ERROR "0:/System/alarm.wav"
#endif

#ifndef Trigger_B_name
#define Trigger_B_name "Swing"
#endif

#ifndef Trigger_C_name
#define Trigger_C_name "Clash"
#endif

#ifndef Trigger_D_name
#define Trigger_D_name "Blaster"
#endif

#ifndef Trigger_E_name
#define Trigger_E_name "Lockup"
#endif

#ifndef Trigger_IN_name
#define Trigger_IN_name "In"
#endif

#ifndef Trigger_OUT_name
#define Trigger_OUT_name "Out"
#endif

#ifndef NEO_PROTOCOL
#define NEO_PROTOCOL ".neo"
#endif

#ifndef NEO_CHARGED
#define NEO_CHARGED "0:/System/Charged"NEO_PROTOCOL
#endif

#ifndef NEO_CHARGING 
#define NEO_CHARGING "0:/System/Charging"NEO_PROTOCOL
#endif

#ifndef NEO_Trigger_B_name
#define NEO_Trigger_B_name "_Swing"
#endif

#ifndef NEO_Trigger_C_name
#define NEO_Trigger_C_name "_Clash"
#endif

#ifndef NEO_Trigger_D_name
#define NEO_Trigger_D_name "_Blaster"
#endif

#ifndef NEO_Trigger_E_name
#define NEO_Trigger_E_name "_Lockup"
#endif

#ifndef NEO_Trigger_IN_name
#define NEO_Trigger_IN_name "_In"
#endif

#ifndef NEO_Trigger_OUT_name
#define NEO_Trigger_OUT_name "_Out"
#endif

#ifndef REPLACE_NAME
#define REPLACE_NAME "effect.txt"
#endif

#define TRIGGER(x) Trigger_##x##_name
#define NEO_TRIGGER(x) NEO_Trigger_##x##_name

#endif
