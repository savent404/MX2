#pragma once

#include "MX_def.h"
#include "debug.h"
#include "ff.h"
#include "mux-def.h"
#include <stdbool.h>

MX_C_API bool
mux_fileObj_open(FIL* pFile, const char* filePath);

MX_C_API bool
mux_fileObj_close(FIL* pFile);

MX_C_API int
mux_fileObj_read(FIL* pFile, void* out, int size);

MX_C_API int
mux_fileObj_getSize(FIL* pFile);

MX_C_API int
mux_fileObj_tell(FIL* pFile);

MX_C_API bool
mux_fileObj_seek(FIL* pFile, int ofs);

MX_C_API unsigned
mux_fileObj_lastTime(FIL* pFile);
