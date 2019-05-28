#include "mux_fileobj.h"

MX_C_API bool
mux_fileObj_open(FIL* pFile, const char* filePath)
{
    FRESULT res = f_open(pFile, filePath, FA_READ);
    if (res != FR_OK) {
        DEBUG(5, "mux open file:%s error:%d", filePath, (int)res);
        return false;
    }
    return true;
}

MX_C_API bool
mux_fileObj_close(FIL* pFile)
{
    FRESULT res = f_close(pFile);
    if (res != FR_OK) {
        DEBUG(5, "mux close file error:%d", (int)res);
        return false;
    }
    return true;
}

MX_C_API int
mux_fileObj_read(FIL* pFile, void* out, int size)
{
    UINT    cnt;
    FRESULT res = f_read(pFile, out, (UINT)size, &cnt);
    return (int)cnt;
}

MX_C_API int
mux_fileObj_getSize(FIL* pFile)
{
    return (int)(f_size((FIL*)pFile));
}

MX_C_API int
mux_fileObj_tell(FIL* pFile)
{
    return (int)(f_tell((FIL*)pFile));
}

MX_C_API bool
mux_fileObj_seek(FIL* pFile, int ofs)
{
    FRESULT res = f_lseek(pFile, (DWORD)ofs);
    if (res != FR_OK) {
        DEBUG(5, "mux seek error:%s", (int)res);
        return false;
    }
    return true;
}

MX_C_API unsigned
mux_fileObj_lastTime(FIL* pFile)
{
    unsigned long leftSize = f_size((FIL*)pFile);
    return (unsigned)(leftSize / sizeof(uint16_t) / 22);
}
