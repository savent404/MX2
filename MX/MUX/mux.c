#include "mux.h"
#include "param.h"
#include <stdbool.h>

#include "ff.h"

osThreadId muxThreadId;
static osSemaphoreId dma_sem;
static osPoolId mux_file_poolId = NULL;

static const osPriority threadDefaultPriority = osPriorityLow;
static const osPriority threadHighPriority = osPriorityRealtime;

static MUX_Info_t info[MX_MUX_MAXIUM_TRACKID];
static uint16_t dmaBuffer[MX_MUX_BUFFSIZE * 2];
static enum {dmaPos_1 = 0, dmaPos_2 = 1} dmaPos;

__IO static enum { inCritical, outCritical } isCritical = outCritical;

static void suspendMuxThread(void);
static void resumeMuxThread(void);

void MX_MUX_Init(void)
{
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++)
    {
        info[i].id = i;
        info[i].mode = MUX_Mode_Idle;
        info[i].path = NULL;
        info[i].leftSize = 0;
        info[i].fileObj = NULL;
    }

    for (int i = 0; i < MX_MUX_BUFFSIZE*2; i++)
    {
        dmaBuffer[i] = 0x1000/2;
    }

    dmaPos = dmaPos_2;

    osSemaphoreDef(mux);
    dma_sem = osSemaphoreCreate(osSemaphore(mux), 1);

    osSemaphoreWait(dma_sem, 0);

#if defined (osBojectsExternal)
#   error "extern const osPoolDef_t"
#else
    const osPoolDef_t os_pool_def_mux = {
        MX_MUX_MAXIUM_TRACKID,
        MX_MUX_BUFFSIZE * sizeof(uint16_t),
        NULL,
    };
#endif

    if (mux_file_poolId == NULL)
        mux_file_poolId = osPoolCreate(&os_pool_def_mux);

    osThreadDef(mux, MX_MUX_Handle, threadDefaultPriority, 0, 1024);
    muxThreadId = osThreadCreate(osThread(mux), NULL);

    /** Start the hardware driver */
    MX_MUX_HW_Init(dmaBuffer, MX_MUX_BUFFSIZE * 2);
}

void MX_MUX_DeInit(void)
{
    suspendMuxThread();
    /** stop harddware first*/
    MX_MUX_HW_DeInit();
    osThreadTerminate(muxThreadId);
    osSemaphoreDelete(dma_sem);
    
    for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++)
    {
        if (!MX_MUX_isIdle((MUX_TrackId_t)(i)))
            MX_MUX_CleanUp(i);
    }
}

void MX_MUX_Start(MUX_TrackId_t id,
                            MUX_Mode_t    mode,
                            const char*   path)
{
    suspendMuxThread();
    if (!MX_MUX_isIdle(id))
    {
        MX_MUX_CleanUp(id);
    }
    MX_MUX_SetUp(id, mode, path);
    resumeMuxThread();
}

bool MX_MUX_isIdle(MUX_TrackId_t id)
{
    return info[id].mode == MUX_Mode_Idle;
}

void MX_MUX_GetStatus(MUX_TrackId_t id,
                                MUX_Info_t *ptr)
{
    memcpy(ptr, &info[id], sizeof(*info));
}

void MX_MUX_CleanUp(MUX_TrackId_t id)
{
    /** Close file first */
    if (info[id].fileObj != NULL)
    {
        FIL* pFile = (FIL*)(info[id].fileObj);
        f_close(pFile);
        osPoolFree(mux_file_poolId, pFile);
        info[id].fileObj = NULL;
    }
    /** Free file path */
    if (info[id].path != NULL)
    {
        free(info[id].path);
        info[id].path = NULL;
    }
    info[id].mode = MUX_Mode_Idle;
    info[id].leftSize = 0;
}

void MX_MUX_SetUp(MUX_TrackId_t id,
                  MUX_Mode_t mode,
                  const char* file)
{
    if (mode == MUX_Mode_Idle)
    {
        return;
    }
    /** Alloc file obj first */
    FIL* pFile = (FIL*)osPoolAlloc(mux_file_poolId);
    FRESULT res = f_open(pFile, file, FA_READ);

    if (res != FR_OK)
    {
        return;
    }
    info[id].fileObj = pFile;
    
    /** File Path alloc */
    info[id].path = (char*)malloc(strlen(file) + 1);
    strcpy(info[id].path, file);

    info[id].mode = mode;
    
    /** Read wav format */
    /** Wav format is 44 Bytes */
    info[id].leftSize = pFile->fsize - 44;

    /** reading wav format */
    f_lseek(pFile, 44);
    /** end of wav format reading */
}

void MX_MUX_Handle(void const* arg)
{
    float f = 1.0f;
    float factor = 16.0f * MX_MUX_MAXIUM_TRACKID;
    const int bitOffset = 0x1000/2; // 12-bit dac's offset
    int16_t readBuffer[MX_MUX_BUFFSIZE];
    int16_t storageBuffer[MX_MUX_BUFFSIZE];

    MUX_Info_t *ptr;
    uint16_t *pDma;
    int16_t *pStorage;
    int offset;

    for (;;) {

        // wait for dma callback
        osSemaphoreWait(dma_sem, osWaitForever);

        // track info pointer
        ptr = info;

        // set vol
        offset = 4 + 3 - USR.config->Vol;
        if (USR.config->Vol == 0) {
            offset = 15;
        }

        // reset storage buffer
        memset(storageBuffer, 0, sizeof(storageBuffer));

        for (int i = 0; i < MX_MUX_MAXIUM_TRACKID; i++) {
            // read file buffer
            if (ptr->fileObj) {
                FIL *pFile = (FIL *) (ptr->fileObj);
                bool canRead = !f_eof(pFile);
                if (canRead) {
                    UINT b;
                    memset(readBuffer, 0, sizeof(readBuffer));
                    isCritical = inCritical;
                    /* 若剩余比特小于sizeof(readBuffer),fatfs读到EOF后就会结束读取操作
                     * 不会影响执行的结果
                     */
                    f_read(pFile, readBuffer, sizeof(readBuffer), &b);
                    isCritical = outCritical;
                    ptr->leftSize -= b;

                    // after read data, merge this track into dma buffer
                    int16_t *pRead = readBuffer;
                    pStorage = storageBuffer;
                    int buf;
                    for (int j = 0; j < MX_MUX_BUFFSIZE; j++)
                    {
                        buf = *pStorage + (*pRead++ / MX_MUX_MAXIUM_TRACKID);

                        if (buf > INT16_MAX / 2) {
                            f = (float) INT16_MAX / 2 / (float) buf;
                            buf = INT16_MAX / 2;
                        }
                        buf *= f;
                        if (buf < INT16_MIN / 2) {
                            f = (float) INT16_MIN / 2 / (float) buf;
                            buf = INT16_MIN / 2;
                        }
                        if (f < 1.0f) {
                            f += (1.0f - f) / factor;
                        }
                        *pStorage++ = (int16_t)buf;
                    }
                }
            }
            ptr++;
        }

        pDma = (uint16_t*)(dmaBuffer + MX_MUX_BUFFSIZE * (int)dmaPos);
        pStorage = storageBuffer;
        for (int i = 0; i < MX_MUX_BUFFSIZE; i++)
        {
            *pDma++ = (uint16_t)((*pStorage++ >> offset) + bitOffset);
        }
    }
}
void MX_MUX_HalfCallback(void)
{
    // first block is ready to reload.
    dmaPos = dmaPos_1;
    osSemaphoreRelease(dma_sem);
}

void MX_MUX_Callback(void)
{
    dmaPos = dmaPos_2;
    osSemaphoreRelease(dma_sem);
}

void suspendMuxThread(void)
{
    while (isCritical == inCritical)
    {
        osThreadSetPriority(muxThreadId, threadHighPriority);
        osDelay(1);
    }
    osThreadSuspend(muxThreadId);
    osThreadSetPriority(muxThreadId, threadHighPriority);
}
void resumeMuxThread(void)
{
    osThreadResume(muxThreadId);
}
