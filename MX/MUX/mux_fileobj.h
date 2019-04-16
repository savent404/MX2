#pragma once

#include "MX_def.h"
#include "debug.h"
#include "ff.h"
#include "mux-def.h"
#include <stdbool.h>

MX_C_API bool
mux_fileObj_open(MUX_FileObj_t* pFile, const char* filePath);

MX_C_API bool
mux_fileObj_close(MUX_FileObj_t* pFile);

MX_C_API int
mux_fileObj_read(MUX_FileObj_t* pFile, void* out, int size);

MX_C_API int
mux_fileObj_getSize(MUX_FileObj_t* pFile);

MX_C_API int
mux_fileObj_tell(MUX_FileObj_t* pFile);

MX_C_API bool
mux_fileObj_seek(MUX_FileObj_t* pFile, int ofs);

MX_C_API unsigned
mux_fileObj_lastTime(MUX_FileObj_t* pFile);
