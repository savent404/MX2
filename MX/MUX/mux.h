#pragma once

#include "MX_def.h"

#include "mux-def.h"

#if 1
#include "mux_fileobj_fatfs.h"
#endif

#if 1
#include "mux_convert_internal_dac.h"
#endif

MX_C_API void MX_MUX_Init(void);
MX_C_API void MX_MUX_DeInit(void);

MX_C_API MUX_Slot_Id_t 
MX_MUX_Start(MUX_Track_Id_t, MUX_Slot_Mode_t, const char* filepath);
MX_C_API void
MX_MUX_Stop(MUX_Track_Id_t, MUX_Slot_Id_t);

MX_C_API void
MX_MUX_RegisterCallback(MUX_Track_Id_t, MUX_Slot_Id_t, MUX_Slot_CallbackWay_t, MUX_Slot_Callback_t);
MX_C_API void
MX_MUX_UnregisterCallback(MUX_Track_Id_t, MUX_Slot_Id_t, MUX_Slot_CallbackWay_t);

MX_C_API bool MX_MUX_hasSlotsIdle(MUX_Track_Id_t);

MX_C_API void MX_MUX_HW_DMA_Callback(MUX_Track_Id_t);
MX_C_API void MX_MUX_HW_DMA_HalfCallback(MUX_Track_Id_t);

MX_INTERNAL_API void
MX_MUX_Handle(void const*);

MX_PORT_API void MX_MUX_HW_Init(MUX_Track_Id_t);
MX_PORT_API void MX_MUX_HW_DeInit(MUX_Track_Id_t);
MX_PORT_API void MX_MUX_HW_Start(MUX_Track_Id_t, void* source, int size);
MX_PORT_API void MX_MUX_HW_Stop(MUX_Track_Id_t);
