#ifndef _PATH_H_
#define _PATH_H_

#ifndef PATH_CONFIG
#define PATH_CONFIG "SETTING.txt"
#endif

#ifndef PATH_COLORMATRIX
#define PATH_COLORMATRIX "ColorMatrix.txt"
#endif

#ifndef PATH_HWCONFIG
#define PATH_HWCONFIG "LEDCFG.txt"
#endif

#ifndef WAV_BOOT
#define WAV_BOOT "System/Boot.wav"
#endif

#ifndef WAV_CHARGING
#define WAV_CHARGING "System/charging.wav"
#endif

#ifndef WAV_COLORSWITCH
#define WAV_COLORSWITCH "System/ColorSwitch.wav"
#endif

#ifndef WAV_LOWPOWER
#define WAV_LOWPOWER "System/lowpower.wav"
#endif

#ifndef WAV_POWEROFF
#define WAV_POWEROFF "System/Poweroff.wav"
#endif

#ifndef WAV_RECHARGE
#define WAV_RECHARGE "System/Recharge.wav"
#endif

#ifndef WAV_ERROR
#define WAV_ERROR "System/alarm.wav"
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

#ifndef REPLACE_NAME
#define REPLACE_NAME "effect.txt"
#endif

#define TRIGGER(x) Trigger_##x##_name

#endif
