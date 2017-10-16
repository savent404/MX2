#ifndef _MX_FILE_H_
#define _MX_FILE_H_

#ifndef LOG_TAG
#define LOG_TAG "MXFile"
#endif

#include "DEBUG.h"
#include "fatfs.h"
#include "mx-config.h"
#include <stdbool.h>

#ifndef MX_FILE_NEOPIXEL_STRNUM
#define MX_FILE_NEOPIXEL_STRNUM (2)
#endif

#ifndef MX_FILE_NEOPIXEL_STRLEN
#define MX_FILE_NEOPIXEL_STRLEN (20)
#endif

typedef enum {
  MX_NEOPIXEL_ID_FRAMECNT = 0,
  MX_NEOPIXEL_ID_FRAMELEN = 1,
  MX_NEOPIXEL_ID_HZ = 2,
  MX_NEOPIXEL_ID_NULL
} MX_NeoPixel_ID_t;

#define MX_NEOPIXEL_ID_STR_FRAMCNT "CNT"
#define MX_NEOPIXEL_ID_STR_FRAMLEN "LEN"
#define MX_NEOPIXEL_ID_STR_HZ "HZ0"

#define MX_NEOPIXEL_ID_STR(name) (MX_NEOPIXEL_ID_STR_##name)
#define MX_NEOPIXEL_ID(id) (MX_NEOPIXEL_ID_##id)
typedef struct _MX_NeoPixel_Structure_t
{
  uint16_t frame_cnt;
  uint16_t frame_len;
  uint16_t Hz;
} MX_NeoPixel_Structure_t;

/**
 * @brief  打开Neopixel文件
 * @NOTE   所有操作基于文件已打开的状态
 * @retvl  Error = false
 */
bool MX_File_NeoPixel_OpenFile(const char *filepath);

/**
 * @brief  关闭Neopixel文件
 */
bool MX_File_NeoPixel_CloseFile(const char *filepath);

/**
 * @brief  得到文件描述
 * @NOTE   在openfile操作后使用
 */
bool MX_File_NeoPixel_GetInfo(MX_NeoPixel_Structure_t *pt, const char *filepath);

/**
 * @brief  得到一行数据
 * @NOTE   在openfile操作后使用
 */
bool MX_File_NeoPixel_GetLine(const char *filepath, uint16_t line, void *buffer, size_t maxsize);

#endif
