#include "console.h"
#include "CLI.h"

static osThreadId    selfId;
static osMutexId     mutex_tx;
static osMutexId     mutex_rx;
static osSemaphoreId UartTx_Cplt_FlagHandle;
static osSemaphoreId UartRx_Cplt_FlagHandle;

MX_C_API BaseType_t CLI_trigger(char*       pcWriteBuffer,
                                size_t      xWriteBufferLen,
                                const char* pcCommandString);
MX_C_API BaseType_t CLI_CpuUsage(char*       pcWriteBuffer,
                                 size_t      xWriteBufferLen,
                                 const char* pcCommandString);
MX_C_API BaseType_t CLI_CpuUsage_c(char*       pcWriteBuffer,
                                   size_t      xWriteBufferLen,
                                   const char* pcCommandString);

static const CLI_Command_Definition_t cmds[] = {
    { "trigger", "trigger\r\n\ta cmd to exec trigger\r\n\tlike: trigger out\r\n\r\n", CLI_trigger, 1 },
    { "cpu", "cpu\r\n\ta cmd to show cpu usage\r\n\r\n", CLI_CpuUsage, 0 },
    { "ccpu", "ccpu {count} {interval:ms}\r\n\ta cmd to continus measure cpu usage\r\n\r\n", CLI_CpuUsage_c, 2 },
};

__MX_WEAK void MX_Console_Print(uint8_t* string, uint16_t size)
{
    osMutexWait(mutex_tx, osWaitForever);
    // TODO:....
    osMutexRelease(mutex_tx);
}
__MX_WEAK char* MX_Console_Gets(char* buffer, int maxiumSize)
{
    char c   = '\0';
    int  pos = 0;
    osMutexWait(mutex_rx, osWaitForever);
    do {
        //TODO: Get a char
        buffer[ pos++ ] = c;
        osDelay(osWaitForever);
    } while (c != '\n' || pos >= maxiumSize - 1);
    buffer[ pos ] = '\0';
    osMutexRelease(mutex_rx);
    return buffer;
}

int MX_Console_Printf(const char* format, ...)
{
    static char buffer[ 128 ] = { 0 };
    uint16_t    res;
    va_list     args;

    va_start(args, format);
    res = (uint16_t)vsprintf(buffer, format, args);
    va_end(args);

    MX_Console_Print((uint8_t*)buffer, res);

    return res;
}

void MX_Console_Scanf(char* format, ...)
{
    static char buffer[ 128 ] = { 0 };

    MX_Console_Gets(buffer, sizeof(buffer));

    va_list args;
    va_start(args, format);
    vsscanf(buffer, format, args);
    va_end(args);
}

void MX_Console_Init(void)
{
    osThreadDef(console, MX_Console_Handle, osPriorityLow, 0, 512);
    osMutexDef(console_mutex);
    osSemaphoreDef(console_sem);

    mutex_tx = osMutexCreate(osMutex(console_mutex));
    mutex_rx = osMutexCreate(osMutex(console_mutex));

    UartTx_Cplt_FlagHandle = osSemaphoreCreate(osSemaphore(console_sem), 1);
    UartRx_Cplt_FlagHandle = osSemaphoreCreate(osSemaphore(console_sem), 1);

    osSemaphoreWait(UartTx_Cplt_FlagHandle, 0);
    osSemaphoreWait(UartRx_Cplt_FlagHandle, 0);

    selfId = osThreadCreate(osThread(console), NULL);
}

void MX_Console_Handle(void const* arg)
{
    static char buffer[ 128 ];
    static char outBuffer[ 128 ];
    BaseType_t  ret;
    for (int i = 0; i < sizeof(cmds) / sizeof(cmds[ 0 ]); i++)
        FreeRTOS_CLIRegisterCommand(&cmds[ i ]);
    while (1) {
        MX_Console_Gets(buffer, sizeof(buffer));
        do {
            ret = FreeRTOS_CLIProcessCommand(buffer, outBuffer, sizeof(outBuffer));
            MX_Console_Print(reinterpret_cast<uint8_t*>(&outBuffer[ 0 ]), strlen(outBuffer));
        } while (ret != pdFALSE);
    }
}

BaseType_t CLI_trigger(char*       pcWriteBuffer,
                       size_t      xWriteBufferLen,
                       const char* pcCommandString)
{
    BaseType_t  len;
    const char* triggerName = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len);

    // search from EventId_t
    for (EventId_t id : EventId_t::_values()) {
        if (!strcasecmp(triggerName, id._to_string())) {
            MX_Event_Put(id, 0);
            break;
        }
    }
    return pdFALSE;
}
MX_C_API BaseType_t CLI_CpuUsage(char*       pcWriteBuffer,
                                 size_t      xWriteBufferLen,
                                 const char* pcCommandString)
{
    extern uint16_t osGetCPUUsage(void);
    auto            min = [](int a, int b) { return a > b ? b : a; };
    char            buffer[ 64 ];
    sprintf(buffer, "CPU usage:\t%d%%\r\n", osGetCPUUsage());
    strncpy(pcWriteBuffer, buffer, min(strlen(buffer) + 1, xWriteBufferLen));
    return pdFALSE;
}

MX_C_API BaseType_t CLI_CpuUsage_c(char*       pcWriteBuffer,
                                   size_t      xWriteBufferLen,
                                   const char* pcCommandString)
{
    static bool     isrunning = false;
    static int      cnt;
    static int      interval;
    extern uint16_t osGetCPUUsage(void);
    auto            min = [](int a, int b) { return a > b ? b : a; };
    char            buffer[ 64 ];

    if (!isrunning) {
        BaseType_t len;
        cnt       = atoi(FreeRTOS_CLIGetParameter(pcCommandString, 1, &len));
        interval  = atoi(FreeRTOS_CLIGetParameter(pcCommandString, 2, &len));
        isrunning = true;
    }

    if (cnt-- <= 0) {
        cnt       = 0;
        isrunning = false;
        return pdFALSE;
    }

    CLI_CpuUsage(pcWriteBuffer, xWriteBufferLen, nullptr);

    int length = strlen(pcWriteBuffer) - 2;
    pcWriteBuffer[length] = '\0';

    sprintf(buffer, "\tleftCnt:%d\r\n", cnt);

    strncat(pcWriteBuffer, buffer, xWriteBufferLen - length - 1);

    if (isrunning) {
        osDelay(interval);
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}

void MX_ConsoleTX_WaitMutex(void)
{

    osMutexWait(mutex_tx, osWaitForever);
}

void MX_ConsoleTX_FreeMutex(void)
{

    osMutexRelease(mutex_tx);
}

void MX_ConsoleRX_WaitMutex(void)
{

    osMutexWait(mutex_rx, osWaitForever);
}

void MX_ConsoleRX_FreeMutex(void)
{
    osMutexRelease(mutex_rx);
}

void MX_ConsoleTX_WaitSem(void)
{
    osSemaphoreWait(UartTx_Cplt_FlagHandle, osWaitForever);
}

void MX_ConsoleTX_FreeSem(void)
{
    osSemaphoreRelease(UartTx_Cplt_FlagHandle);
}

void MX_ConsoleRX_WaitSem(void)
{
    osSemaphoreWait(UartRx_Cplt_FlagHandle, osWaitForever);
}

void MX_ConsoleRX_FreeSem(void)
{
    osSemaphoreRelease(UartRx_Cplt_FlagHandle);
}
