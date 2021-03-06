#pragma once

/** typedef for slot *************************************/
typedef enum {
    SlotMode_Idle        = 0,
    SlotMode_Once        = 1,
    SlotMode_Loop        = 2
} MUX_Slot_Mode_t;

typedef int MUX_Slot_Id_t;

typedef enum {
    SlotCallback_Done    = 0,
    SlotCallback_Destory = 1,
    SlotCallback_Max,
} MUX_Slot_CallbackWay_t;

typedef void (*MUX_Slot_Callback_t)(void);

typedef struct mux_wavInfo {
    unsigned channels;
    unsigned samplesPreSec;
    unsigned bitsPreSample;
    unsigned blockAlign; // = channels * bitPreSample/8
    unsigned dataOffset;
    unsigned dataSize;
} mux_wavInfo_t;

#if 1
#    include "ff.h"
typedef struct MUX_FileObj_t {
    FIL           fileObj;
    mux_wavInfo_t info;
} MUX_FileObj_t;

#endif

typedef struct {
    MUX_FileObj_t*      pObj;
    MUX_Slot_Mode_t     mode;
    MUX_Slot_Id_t       id;
    MUX_Slot_Callback_t callback[ SlotCallback_Max ];
    int                 vol;
#ifdef USE_DEBUG
    char filePath[ 64 ];
#endif
} MUX_Slot_t;

/** typedef for track ************************************/

#if 1
typedef uint16_t muxBuffer_t;
#endif

typedef enum {
    TrackId_MainLoop = 0,
#if MX_MUX_DUAL_TRACK == 1
    TrackId_Trigger = 1,
#else
    TrackId_Trigger = 0,
#endif
} MUX_Track_Id_t;

typedef enum {
    TrackState_Idle     = 0, //未运行状态
    TrackState_Running  = 1, //正在运行
    TrackState_Critical = 2, //正在临界区域
} MUX_Track_Mode_t;

typedef struct {
    MUX_Track_Id_t   id;
    MUX_Track_Mode_t mode;
    int              maxium_slot;
    MUX_Slot_t*      slots;
    enum { pos1,
           pos2 } pos;
    muxBuffer_t* buffer;
    int          bufferSize;
} MUX_Track_t;
