#include "mux_wavops.h"
#include "debug.h"
#include "mux_fileobj.h"

#undef MODULE_NAME
#define MODULE_NAME "[MUX-WAVOPS]\t"
#define MUX_OPS_DEBUG(level, format, ...) DEBUG(level, MODULE_NAME format, ##__VA_ARGS__)

static inline uint32_t readU32FromBuffer(char* array, int offset)
{
    return *(uint32_t*)(array + offset);
}
static inline uint16_t readU16FromBuffer(char* arry, int offset)
{
    return *(uint16_t*)(arry + offset);
}

MX_C_API bool
mux_wavOps_open(MUX_FileObj_t* ptr, const char* filePath)
{
    unsigned char chunkBuffer[ MX_MUX_WAV_FIX_OFFSET ];
    if (!mux_fileObj_open(&ptr->fileObj, filePath)) {
        return false;
    }

    // check if wav is valid and fill structure
    if (mux_fileObj_read(&ptr->fileObj, chunkBuffer, sizeof(chunkBuffer)) < MX_MUX_WAV_FIX_OFFSET)
        goto openFaild;
    if (strncmp(chunkBuffer, "RIFF", 4))
        goto openFaild;
    if (strncmp(chunkBuffer + 8, "WAVE", 4))
        goto openFaild;
    if (strncmp(chunkBuffer + 12, "fmt", 3))
        goto openFaild;
    if (readU16FromBuffer(chunkBuffer, 20) != 1)
        goto openFaild;
    if (strncmp(chunkBuffer + 36, "data", 4))
        goto openFaild;
    ptr->info.channels      = readU16FromBuffer(chunkBuffer, 22);
    ptr->info.samplesPreSec = readU32FromBuffer(chunkBuffer, 24);
    ptr->info.blockAlign    = readU16FromBuffer(chunkBuffer, 32);
    ptr->info.bitsPreSample = readU16FromBuffer(chunkBuffer, 34);
    ptr->info.dataOffset    = MX_MUX_WAV_FIX_OFFSET;
    ptr->info.dataSize      = readU32FromBuffer(chunkBuffer, 40);

    MUX_OPS_DEBUG(5, "Info:");
    MUX_OPS_DEBUG(5, "\tchannels:%u", ptr->info.channels);
    MUX_OPS_DEBUG(5, "\tsample pre seconds:%ud", ptr->info.samplesPreSec);
    MUX_OPS_DEBUG(5, "\tBlockSize:%u", ptr->info.blockAlign);
    MUX_OPS_DEBUG(5, "\tBit pre sample:%u", ptr->info.bitsPreSample);
    MUX_OPS_DEBUG(5, "\tData Bytes:%u", ptr->info.dataSize);
    MUX_OPS_DEBUG(5, "\tData offset bytes:%u", ptr->info.dataOffset);
    return true;
openFaild:
    MUX_OPS_DEBUG(5, "can't read infomation from this file:%s", filePath);
    return false;
}

MX_C_API bool
mux_wavOps_close(MUX_FileObj_t* ptr)
{
    return mux_fileObj_close(&ptr->fileObj);
}

MX_C_API int
mux_wavOps_read(MUX_FileObj_t* ptr, void* out, int size)
{
    return mux_fileObj_read(&ptr->fileObj, out, size);
}

MX_C_API int
mux_wavOps_getSize(MUX_FileObj_t* ptr)
{
    return ptr->info.dataSize;
}

MX_C_API int
mux_wavOps_tell(MUX_FileObj_t* ptr)
{
    return mux_fileObj_tell(&ptr->fileObj) - ptr->info.dataOffset;
}

MX_C_API int
mux_wavOps_seek(MUX_FileObj_t* ptr, int ofs)
{
    return mux_fileObj_seek(&ptr->fileObj, ofs + ptr->info.dataOffset);
}

MX_C_API unsigned
mux_wavOps_lastTime(MUX_FileObj_t* ptr)
{
    unsigned bytesPreSec = ptr->info.blockAlign * ptr->info.samplesPreSec;
    unsigned leftSize    = (mux_wavOps_getSize(ptr) - mux_wavOps_tell(ptr)) * 1000;

    return leftSize / bytesPreSec;
}