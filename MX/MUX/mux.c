#include "mux.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "param.h"
#include <stdlib.h>

static MUX_Track_t tracks[ MX_MUX_MAXIUM_TRACKID ];

static osThreadId    selfThreadId[ MX_MUX_MAXIUM_TRACKID ];
static osSemaphoreId selfSemId[ MX_MUX_MAXIUM_TRACKID ];
// static osMutexId selfMutexId[MX_MUX_MAXIUM_TRACKID];
static osSemaphoreId selfMutexId[ MX_MUX_MAXIUM_TRACKID ];

static inline void initMutex(void)
{
    osSemaphoreDef(mux);
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++)
        selfMutexId[ i ] = osSemaphoreCreate(osSemaphore(mux), 1);
}
static inline void waitMutex(MUX_Track_Id_t tid)
{
    osSemaphoreWait(selfMutexId[ tid ], osWaitForever);
}
static inline void releaseMutex(MUX_Track_Id_t tid)
{
    osSemaphoreRelease(selfMutexId[ tid ]);
}

static inline void callCallbackFunc(MUX_Slot_Callback_t func)
{
    if (func)
        func();
}
static inline void clearAllCallback(MUX_Slot_t* pSlot)
{
    for (int i = 0; i < SlotCallback_Max; i++)
        pSlot->callback[ i ] = NULL;
}

void MX_MUX_Init(void)
{
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++) {
        tracks[ i ].id   = i;
        tracks[ i ].mode = TrackState_Idle;
#if MX_MUX_DUAL_TRACK == 1
        if (i == TrackId_MainLoop)
            tracks[ i ].maxium_slot = 3;
        else
            tracks[ i ].maxium_slot = 1;
#else
        tracks[ 0 ].maxium_slot = 3 + 1;
#endif
        tracks[ i ].slots      = (MUX_Slot_t*)pvPortMalloc(sizeof(MUX_Slot_t) * tracks[ i ].maxium_slot);
        tracks[ i ].buffer     = (muxBuffer_t*)pvPortMalloc(sizeof(muxBuffer_t) * MX_MUX_BUFFSIZE * 2);
        tracks[ i ].bufferSize = MX_MUX_BUFFSIZE * 2;
        tracks[ i ].pos        = pos1;
        for (int j = 0; j < tracks[ i ].maxium_slot; j++) {
            tracks[ i ].slots[ j ].pObj = (MUX_FileObj_t*)pvPortMalloc(sizeof(MUX_FileObj_t));
            tracks[ i ].slots[ j ].mode = SlotMode_Idle;
            tracks[ i ].slots[ j ].id   = j;
            clearAllCallback(&tracks[ i ].slots[ j ]);
        }
        MX_MUX_HW_Init(i);
        mux_resetDmaBuffer(tracks[ i ].buffer, MX_MUX_BUFFSIZE * 2);
    }
    initMutex();
    /** Init semaphore */
    osSemaphoreDef(mux);
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++) {
        selfSemId[ i ] = osSemaphoreCreate(osSemaphore(mux), 1);
        osSemaphoreWait(selfSemId[ i ], 1);
    }

    /** Start thread */
    osThreadDef(mux1, MX_MUX_Handle, osPriorityNormal, 0, MX_MUX_THREAD_STACK_SIZE);
    selfThreadId[ 0 ] = osThreadCreate(osThread(mux1), &tracks[ 0 ]);

#if MX_MUX_DUAL_TRACK == 1
    osThreadDef(mux2, MX_MUX_Handle, osPriorityNormal, 0, MX_MUX_THREAD_STACK_SIZE);
    selfThreadId[ 1 ] = osThreadCreate(osThread(mux2), &tracks[ 1 ]);
#endif
}

void MX_MUX_DeInit(void)
{
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++) {
        osMutexWait(selfMutexId[ i ], 0);
        osThreadSuspend(selfThreadId[ i ]);
        osThreadTerminate(selfThreadId[ i ]);
        osSemaphoreDelete(selfSemId[ i ]);
        MX_MUX_HW_Stop(i);
        tracks[ i ].mode = TrackState_Idle;
        MX_MUX_HW_DeInit(i);

        for (int j = 0; j < tracks[ i ].maxium_slot; j++) {
            vPortFree(tracks[ i ].slots[ j ].pObj);
        }
        vPortFree(tracks[ i ].slots);
        vPortFree(tracks[ i ].buffer);

        osMutexRelease(selfMutexId[ i ]);
        osMutexDelete(selfMutexId[ i ]);
    }
}

MUX_Slot_Id_t
MX_MUX_Start(MUX_Track_Id_t tid, MUX_Slot_Mode_t mode, const char* path, int vol)
{
    MUX_Track_t* pTrack = tracks + tid;
    for (int i = 0; i < pTrack->maxium_slot; i++) {
        if (pTrack->slots[ i ].mode != SlotMode_Idle)
            continue;

        MUX_FileObj_t* pObj = pTrack->slots[ i ].pObj;

        waitMutex(tid);
        if (mux_wavOps_open(pObj, path) == false)
            goto failed;

        if (mux_wavOps_seek(pObj, 0) == false) {
            mux_wavOps_close(pObj);
            goto failed;
        }
        pTrack->slots[ i ].mode = mode;
        pTrack->slots[ i ].vol  = vol;
#ifdef USE_DEBUG
        strncpy(pTrack->slots[ i ].filePath, path, sizeof(pTrack->slots[ i ].filePath));
        DEBUG(5, "[MUX] Start audio:%s\tT:%d|S:%d", pTrack->slots[ i ].filePath,
              pTrack->id,
              pTrack->slots[ i ].id);
#endif
        clearAllCallback(&pTrack->slots[ i ]);
        releaseMutex(tid);
        return i;
    }
    return -1;
failed:
    releaseMutex(tid);
    return -1;
}

void MX_MUX_Stop(MUX_Track_Id_t tid, MUX_Slot_Id_t sid)
{
    if (tracks[ tid ].slots[ sid ].mode != SlotMode_Idle) {
        waitMutex(tid);
        tracks[ tid ].slots[ sid ].mode = SlotMode_Idle;
        mux_wavOps_close(tracks[ tid ].slots[ sid ].pObj);
        tracks[ tid ].slots[ sid ].callback[ SlotCallback_Destory ] = NULL;
        tracks[ tid ].slots[ sid ].callback[ SlotCallback_Done ]    = NULL;
#ifdef USE_DEBUG
        tracks[ tid ].slots[ sid ].filePath[ 0 ] = '\0';
#endif
        clearAllCallback(&tracks[ tid ].slots[ sid ]);
        releaseMutex(tid);
    }
}

unsigned int MX_MUX_getLastTime(MUX_Track_Id_t tid, MUX_Slot_Id_t sid)
{
    if (tracks[ tid ].maxium_slot <= sid)
        return 0;
    if (tracks[ tid ].slots[ sid ].mode == SlotMode_Idle)
        return 0;
    return mux_wavOps_lastTime(tracks[ tid ].slots[ sid ].pObj);
}
void MX_MUX_RegisterCallback(MUX_Track_Id_t tid, MUX_Slot_Id_t sid, MUX_Slot_CallbackWay_t way, MUX_Slot_Callback_t func)
{
    waitMutex(tid);
    tracks[ tid ].slots[ sid ].callback[ way ] = func;
    releaseMutex(tid);
}

void MX_MUX_UnregisterCallback(MUX_Track_Id_t tid, MUX_Slot_Id_t sid, MUX_Slot_CallbackWay_t way)
{
    waitMutex(tid);
    tracks[ tid ].slots[ sid ].callback[ way ] = NULL;
    releaseMutex(tid);
}

void MX_MUX_Handle(void const* arg)
{
    MUX_Track_t* pTrack = (MUX_Track_t*)arg;
    MUX_Slot_t*  pSlot  = NULL;

    int bufferSize     = pTrack->bufferSize / 2;
    int bufferByteSize = bufferSize * sizeof(muxBuffer_t);

    int*         storageBuffer = (int*)pvPortMalloc(sizeof(int) * bufferSize);
    muxBuffer_t* readBuffer    = (muxBuffer_t*)pvPortMalloc(bufferByteSize);
    muxBuffer_t* destBuffer;
    float        f = 1.0f;
    MX_MUX_HW_Start(pTrack->id, pTrack->buffer, pTrack->bufferSize);
    pTrack->mode = TrackState_Running;

    for (;;) {
        // wait for semaphore
        osSemaphoreWait(selfSemId[ pTrack->id ], osWaitForever);

        memset(storageBuffer, 0, sizeof(int) * bufferSize);

        waitMutex(pTrack->id);

        // calculate public vol
        int vol_ans = 0;
        for (int i = 0; i < pTrack->maxium_slot; i++) {
            pSlot = &pTrack->slots[ i ];
            if (pSlot->mode == SlotMode_Idle)
                continue;
            vol_ans += pSlot->vol;
        }

        float volMulti = vol_ans ? (float)(USR.config->Vol) / (float)(vol_ans) : 0;

        for (int i = 0; i < pTrack->maxium_slot; i++) {
            pSlot = &pTrack->slots[ i ];

            if (pSlot->mode == SlotMode_Idle)
                continue;
            /** pSlot->mode != SlotMode_Idel ***/

            int  readedSize = 0;
            int  tmpReadSize;
            int* pStoraged = storageBuffer;
            int  leftSize;
            int  bytesNeeded;
        readAgain:
            tmpReadSize = 0;
            leftSize    = mux_wavOps_getSize(pSlot->pObj) - mux_wavOps_tell(pSlot->pObj);
            bytesNeeded = bufferByteSize - readedSize;
            tmpReadSize = mux_wavOps_read(pSlot->pObj, readBuffer, leftSize >= bytesNeeded ? bytesNeeded : leftSize);
            readedSize += tmpReadSize;
            mux_convert_addToInt(readBuffer, pStoraged, tmpReadSize / sizeof(muxBuffer_t), volMulti, &f, pSlot->vol);

            // if no read error then skip next ops
            // if no read ops or read error, goto clear stage
            if (readedSize >= bufferByteSize)
                continue;

            // clear stage
            switch (pSlot->mode) {
            case SlotMode_Once:
                mux_wavOps_close(pSlot->pObj);
                pSlot->mode = SlotMode_Idle;
                callCallbackFunc(pSlot->callback[ SlotCallback_Done ]);
                callCallbackFunc(pSlot->callback[ SlotCallback_Destory ]);
                clearAllCallback(pSlot);
                DEBUG(5, "[MUX]: Oneshot slot done: %s\t T:%d|S:%d",
                      pSlot->filePath,
                      pTrack->id,
                      pSlot->id);
                break;
            case SlotMode_Loop:
                mux_wavOps_seek(pSlot->pObj, 0);
                callCallbackFunc(pSlot->callback[ SlotCallback_Done ]);
                DEBUG(5, "[MUX]: Loop slot EOF: %s\t T:%d|S:%d",
                      pSlot->filePath,
                      pTrack->id,
                      pSlot->id);
                
                pStoraged = storageBuffer + readedSize / sizeof(muxBuffer_t);
                goto readAgain;
                break;
            }
        }
        destBuffer = pTrack->buffer + pTrack->pos * bufferSize;
        mux_convert_mergeToBuffer(storageBuffer, destBuffer, bufferSize, MX_MUX_WAV_VOL_LEVEL);
        releaseMutex(pTrack->id);
    }
}

bool MX_MUX_hasSlotsIdle(MUX_Track_Id_t tid)
{
    MUX_Track_t* pTrack = &tracks[ tid ];
    MUX_Slot_t*  pSlot;
    for (int i = 0; i < pTrack->maxium_slot; i++) {
        pSlot = &pTrack->slots[ i ];
        if (pSlot->mode == SlotMode_Idle)
            return true;
    }
    return false;
}

void MX_MUX_HW_DMA_Callback(MUX_Track_Id_t tid)
{
    tracks[ tid ].pos = pos2;
    osSemaphoreRelease(selfSemId[ tid ]);
}

void MX_MUX_HW_DMA_HalfCallback(MUX_Track_Id_t tid)
{
    tracks[ tid ].pos = pos1;
    osSemaphoreRelease(selfSemId[ tid ]);
}

MX_C_API MUX_Slot_t* MX_MUX_getInstance(MUX_Track_Id_t tid, MUX_Slot_Id_t sid)
{
    if (tid < 0 || tid >= MX_MUX_MAXIUM_TRACKID) {
        return NULL;
    }
    if (sid < 0 || sid >= tracks[ tid ].maxium_slot) {
        return NULL;
    }
    if (tracks[ tid ].slots == NULL) {
        return NULL;
    }
    return tracks[ tid ].slots + sid;
}
