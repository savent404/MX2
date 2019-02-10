#pragma once

#include "MX_def.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#if MX_MUX_MAXIUM_TRACKID == 2
typedef enum _mx_mux_track_id_ {
    MUX_Track_MainLoop = 0,
    MUX_Track_Trigger = 1,
} MUX_TrackId_t;
#endif

typedef enum _mx_mux_track_mode_ {
    MUX_Mode_Idle = 0,
    MUX_Mode_Once = 1,
    MUX_Mode_Loop = 2,
} MUX_Mode_t;

typedef struct MUX_Info_t {
    /** Input parameter */
    MUX_TrackId_t id;
    MUX_Mode_t    mode;
    char*   path;

    /** Output parameter */
    void*  fileObj;
    size_t leftSize;
} MUX_Info_t;

/**
 * @brief init Mux function
 * @note will enable a thread to run 'MX_MUX_Handle'
 * @note will start hardware
 */
MX_C_API void MX_MUX_Init(void);
/**
 * @brief de-init Mux function
 * @note  disable hardware first
 * @note  delete thread
 * @note  teardown resource
 */
MX_C_API void MX_MUX_DeInit(void);

/**
 * @brief Start a trigger.
 * @note  If MUX_Mode_t is 'Idle', end stop to play this track.
 * @note  If this track is busy now,
 *        just cut the previous trigger, and play the next one
 */
MX_C_API void MX_MUX_Start(MUX_TrackId_t, MUX_Mode_t, const char*);

/**
 * @brief check if the track is empty, and ready to play
 */
MX_C_API bool MX_MUX_isIdle(MUX_TrackId_t);

/**
 * @brief get all attributes of a single track
 */
MX_C_API void MX_MUX_GetStatus(MUX_TrackId_t, MUX_Info_t*);

/** DMA callback implement */
MX_C_API void MX_MUX_HalfCallback(void);
MX_C_API void MX_MUX_Callback(void);

/** Portable function, call in Init or DeInit */
MX_PORT_API void MX_MUX_HW_Init(void* source, size_t size);
MX_PORT_API void MX_MUX_HW_DeInit(void);

/** 
 * Internal static functions
 **/
MX_INTERNAL_API void MX_MUX_CleanUp(MUX_TrackId_t id);
MX_INTERNAL_API void MX_MUX_SetUp(MUX_TrackId_t, MUX_Mode_t, const char*);
MX_INTERNAL_API void MX_MUX_Handle(void const*);
