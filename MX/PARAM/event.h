#pragma once

#include "MX_def.h"
#include "cmsis_os.h"
#include "enum.hpp"

// clang-format off
BETTER_ENUM(EventId_t,
        int8_t,
        null,
        On,
        Off,
        Charging,
        Charged,
        Out,
        In,
        BankSwitch,
        ColorSwitch,
        Swing,
        Stab,
        Spin_Start,
        Spin_Update,
        Spin_End,
        Clash,
        Blaster,
        Lockup_Start,
        Lockup_End);
// clang-format on

typedef int16_t EventMsg_t;

MX_C_API bool MX_Event_Init();
MX_C_API bool MX_Event_Peek();
MX_C_API bool MX_Event_Get(EventId_t& id, EventMsg_t& msg, uint32_t timeout = 0xFFFFFFFF);
MX_C_API bool MX_Event_Put(const EventId_t id, const EventMsg_t msg, uint32_t timeout = 0xFFFFFFFF);
