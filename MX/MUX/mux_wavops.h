#pragma once

#include "mux_fileobj.h"
#include <stdbool.h>
#include <stdint.h>

MX_C_API bool
mux_wavOps_open(MUX_FileObj_t*, const char* filePath);

MX_C_API bool
mux_wavOps_close(MUX_FileObj_t*);

MX_C_API int
mux_wavOps_read(MUX_FileObj_t*, void* out, int size);

MX_C_API int
mux_wavOps_getSize(MUX_FileObj_t*);

MX_C_API int
mux_wavOps_tell(MUX_FileObj_t*);

MX_C_API int
mux_wavOps_seek(MUX_FileObj_t*, int ofs);

MX_C_API unsigned
mux_wavOps_lastTime(MUX_FileObj_t*);