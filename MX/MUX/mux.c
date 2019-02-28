#include "mux.h"
#include "FreeRTOS.h"
#include "param.h"
#include "cmsis_os.h"
#include <stdlib.h>

static MUX_Track_t tracks[MX_MUX_MAXIUM_TRACKID];

static osThreadId selfThreadId[MX_MUX_MAXIUM_TRACKID];
static osSemaphoreId selfSemId[MX_MUX_MAXIUM_TRACKID];
// static osMutexId selfMutexId[MX_MUX_MAXIUM_TRACKID];
static osSemaphoreId selfMutexId[MX_MUX_MAXIUM_TRACKID];

static inline void initMutex(void) {
    osSemaphoreDef(mux);
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++)
        selfMutexId[i] = osSemaphoreCreate(osSemaphore(mux), 1);
}
static inline void waitMutex(MUX_Track_Id_t tid) {
  osSemaphoreWait(selfMutexId[tid], osWaitForever);
}
static inline void releaseMutex(MUX_Track_Id_t tid) {
  osSemaphoreRelease(selfMutexId[tid]);
}

static inline void callCallbackFunc(MUX_Slot_Callback_t func)
{
    if (func)
        func();
}
static inline void clearAllCallback(MUX_Slot_t* pSlot)
{
    for (int i = 0; i < SlotCallback_Max; i++)
        pSlot->callback[i] = NULL;
}

void MX_MUX_Init(void)
{
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++) {
        tracks[i].id = i;
        tracks[i].mode = TrackState_Idle;
        if (i == TrackId_MainLoop)
            tracks[i].maxium_slot = 1;
        else
            tracks[i].maxium_slot = 2;
        tracks[i].slots = (MUX_Slot_t*)pvPortMalloc(sizeof(MUX_Track_t) * tracks[i].maxium_slot);
        tracks[i].buffer = (muxBuffer_t*)pvPortMalloc(sizeof(muxBuffer_t) * MX_MUX_BUFFSIZE * 2);
        tracks[i].bufferSize = MX_MUX_BUFFSIZE * 2;
        tracks[i].pos = pos1;
        for (int j = 0; j < tracks[i].maxium_slot; j++) {
            tracks[i].slots[j].pObj = (MUX_FileObj_t*)pvPortMalloc(sizeof(MUX_FileObj_t));
            tracks[i].slots[j].mode = SlotMode_Idle;
            tracks[i].slots[j].id = j;
            clearAllCallback(&tracks[i].slots[j]);
        }
        MX_MUX_HW_Init(i);
        mux_resetDmaBuffer(tracks[i].buffer, MX_MUX_BUFFSIZE*2);
    }
    initMutex();
    /** Init semaphore */
    osSemaphoreDef(mux);
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++) {
        selfSemId[i] = osSemaphoreCreate(osSemaphore(mux), 1);
        osSemaphoreWait(selfSemId[i], 1);
    }
    
    /** Start thread */
    osThreadDef(mux1, MX_MUX_Handle, osPriorityNormal, 0, MX_MUX_THREAD_STACK_SIZE);
    selfThreadId[0] = osThreadCreate(osThread(mux1), &tracks[0]);

#if MX_MUX_DUAL_TRACK == 1
    osThreadDef(mux2, MX_MUX_Handle, osPriorityNormal, 0, MX_MUX_THREAD_STACK_SIZE);
    selfThreadId[1] = osThreadCreate(osThread(mux2), &tracks[1]);
#endif
}

void MX_MUX_DeInit(void)
{
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++) {
        osMutexWait(selfMutexId[i], 0);
        osThreadSuspend(selfThreadId[i]);
        osThreadTerminate(selfThreadId[i]);
        osSemaphoreDelete(selfSemId[i]);
        MX_MUX_HW_Stop(i);
        tracks[i].mode = TrackState_Idle;
        MX_MUX_HW_DeInit(i);

        for (int j = 0; j < tracks[i].maxium_slot; j++) {
            vPortFree(tracks[i].slots[j].pObj);
        }
        vPortFree(tracks[i].slots);
        vPortFree(tracks[i].buffer);

        osMutexRelease(selfMutexId[i]);
        osMutexDelete(selfMutexId[i]);
    }
}

MUX_Slot_Id_t
MX_MUX_Start(MUX_Track_Id_t tid, MUX_Slot_Mode_t mode, const char* path)
{
    MUX_Track_t* pTrack = tracks + tid;
    for (int i = 0; i < pTrack->maxium_slot; i++) {
        if (pTrack->slots[i].mode != SlotMode_Idle)
            continue;

        MUX_FileObj_t* pObj = pTrack->slots[i].pObj;

        waitMutex(tid);
        if (mux_fileObj_open(pObj, path) == false)
            goto failed;

        if (mux_fileObj_seek(pObj, MX_MUX_WAV_FIX_OFFSET) == false) {
            mux_fileObj_close(pObj);
            goto failed;
        }
        pTrack->slots[i].mode = mode;
        clearAllCallback(&pTrack->slots[i]);
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
    if (tracks[tid].slots[sid].mode != SlotMode_Idle) {
        waitMutex(tid);
        tracks[tid].slots[sid].mode = SlotMode_Idle;
        mux_fileObj_close(tracks[tid].slots[sid].pObj);
        tracks[tid].slots[sid].callback[SlotCallback_Destory] = NULL;
        tracks[tid].slots[sid].callback[SlotCallback_Done] = NULL;
        clearAllCallback(&tracks[tid].slots[sid]);
        releaseMutex(tid);
    }
}

void MX_MUX_RegisterCallback(MUX_Track_Id_t tid, MUX_Slot_Id_t sid, MUX_Slot_CallbackWay_t way, MUX_Slot_Callback_t func)
{
    waitMutex(tid);
    tracks[tid].slots[sid].callback[way] = func;
    releaseMutex(tid);
}

void MX_MUX_UnregisterCallback(MUX_Track_Id_t tid, MUX_Slot_Id_t sid, MUX_Slot_CallbackWay_t way)
{
    waitMutex(tid);
    tracks[tid].slots[sid].callback[way] = NULL;
    releaseMutex(tid);
}

void MX_MUX_Handle(void const* arg)
{
    MUX_Track_t* pTrack = (MUX_Track_t*)arg;
    MUX_Slot_t* pSlot = NULL;

    int bufferSize = pTrack->bufferSize / 2;
    int bufferByteSize = bufferSize * sizeof(muxBuffer_t);

    int* storageBuffer = (int*)pvPortMalloc(sizeof(int) * bufferSize);
    muxBuffer_t* readBuffer = (muxBuffer_t*)pvPortMalloc(bufferByteSize);
    muxBuffer_t* destBuffer;
    float f;
    MX_MUX_HW_Start(pTrack->id, pTrack->buffer, pTrack->bufferSize);
    pTrack->mode = TrackState_Running;

    for (;;) {
        // wait for semaphore
        osSemaphoreWait(selfSemId[pTrack->id], osWaitForever);

        memset(storageBuffer, 0, sizeof(int) * bufferSize);

        waitMutex(pTrack->id);
        for (int i = 0; i < pTrack->maxium_slot; i++) {
            pSlot = &pTrack->slots[i];

            if (pSlot->mode == SlotMode_Idle)
                continue;
            /** pSlot->mode != SlotMode_Idel ***/

            int readedSize = mux_fileObj_read(pSlot->pObj, readBuffer, bufferByteSize);
            mux_convert_addToInt(readBuffer, storageBuffer, readedSize / sizeof(muxBuffer_t), &f);

            if (readedSize == bufferByteSize)
                continue;
            /** readSize < bufferByteSize ******/
            switch (pSlot->mode) {
            case SlotMode_Once:
                if (mux_fileObj_close(pSlot->pObj) == false)
                    break;
                pSlot->mode = SlotMode_Idle;
                callCallbackFunc(pSlot->callback[SlotCallback_Destory]);
                clearAllCallback(pSlot);
                break;
            case SlotMode_Loop:
                mux_fileObj_seek(pSlot->pObj, MX_MUX_WAV_FIX_OFFSET);
                callCallbackFunc(pSlot->callback[SlotCallback_Done]);
                break;
            }
        }
        destBuffer = pTrack->buffer + pTrack->pos * bufferSize;
        mux_convert_mergeToBuffer(storageBuffer, destBuffer, bufferSize, USR.config->Vol);
        releaseMutex(pTrack->id);
    }
}

bool MX_MUX_hasSlotsIdle(MUX_Track_Id_t tid)
{
    MUX_Track_t* pTrack = &tracks[tid];
    MUX_Slot_t* pSlot;
    for (int i = 0; i < pTrack->maxium_slot; i++)
    {
        pSlot = &pTrack->slots[i];
        if (pSlot->mode == SlotMode_Idle)
            return true;
    }
    return false;
}

void MX_MUX_HW_DMA_Callback(MUX_Track_Id_t tid)
{
    tracks[tid].pos = pos2;
    osSemaphoreRelease(selfSemId[tid]);
}

void MX_MUX_HW_DMA_HalfCallback(MUX_Track_Id_t tid)
{
    tracks[tid].pos = pos1;
    osSemaphoreRelease(selfSemId[tid]);
}
