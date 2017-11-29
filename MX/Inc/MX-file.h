#ifndef _MX_FILE_H_
#define _MX_FILE_H_

#ifndef LOG_TAG
#define LOG_TAG "MX-File"
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

#define MX_NEOPIXEL_ID_STR_FRAMECNT "Raw"
#define MX_NEOPIXEL_ID_STR_FRAMELEN "Col"
#define MX_NEOPIXEL_ID_STR_HZ "Freq"

#define MX_NEOPIXEL_ID_STR(name) (MX_NEOPIXEL_ID_STR_##name)
#define MX_NEOPIXEL_ID(id) (MX_NEOPIXEL_ID_##id)
typedef struct _MX_NeoPixel_Structure_t
{
  uint16_t frame_cnt;
  uint16_t frame_len;
  uint16_t Hz;

  // System PCB
  UINT payload_offset;
  char filepath[MX_FILE_NEOPIXEL_STRLEN];
  FIL  file;
} MX_NeoPixel_Structure_t;

/**
 * @brief  打开Neopixel文件
 * @NOTE   所有操作基于文件已打开的状态
 * @retvl  Error = false
 */
const MX_NeoPixel_Structure_t* MX_File_NeoPixel_OpenFile(const char *filepath);

/**
 * @brief  获取一个目录中前n个中的随机文件
 * @param  @num 限制的文件数量，0即不限制
 * @param  @filename 得到的文件路径
 */
bool MX_File_GetRandFileName(const char *dir, const int num, char *filename);
/**
 * @brief  关闭Neopixel文件
 */
bool MX_File_NeoPixel_CloseFile(MX_NeoPixel_Structure_t *pt);

/**
 * @brief  得到一行数据
 * @NOTE   在openfile操作后使用
 */
bool MX_File_NeoPixel_GetLine(const MX_NeoPixel_Structure_t *pt, uint16_t line, void *buffer, size_t maxsize);

/**
 * @brief  搜索匹配文件数量
 * @param  -dirpath 需搜索的文件夹路径
 * @param  -prefix  匹配文件名前缀
 * @param  -subfix  匹配文件名后缀
 * @note   不会递归搜索子文件夹
 * @retvl  匹配文件数量
 */
int MX_File_SearchFile(const char *dirpath, const char *prefix, const char *suffix);

/**
 * @brief  获取第'rank'个匹配文件的文件名
 * @param  -dirpath 需搜索的文件夹路径
 * @param  -prefix  匹配文件名前缀
 * @param  -subfix  匹配文件名后缀
 * @param  -rank    匹配第rank个文件(0 开始)
 * @param  -name    获取的文件名
 * @param  -maxStrlen 最大文件名长度
 * @note   不会递归搜索子文件夹
 */
bool MX_File_SearchFileName(const char *dirpath, const char *prefix, const char *suffix, uint16_t rank, char *name, uint8_t maxStrLen);
/**
 * @brief  搜索匹配文件夹数量
 * @param  -subdir 需搜索的文件夹路径
 * @param  -prefix 匹配文件夹名前缀
 * @param  -subfix 匹配文件夹名后缀
 * @note   不会递归搜索子文件夹
 * @retvl  匹配文件夹数量
 */
int MX_File_SearchDir(const char *subdir, const char *prefix, const char *suffix);
// fatfs LFN 支持
void MX_File_InfoLFN_Init(FILINFO *info);
void MX_File_InfoLFN_DeInit(FILINFO *info);

// 系统设置支持
int MX_File_GetBank(void);
void MX_File_SetBank(int bank);

// String.h 补全
char *upper(char *src);
#ifdef __GNUC__
int strcasecmp(const char *src1, const char *src2);
int strncasecmp(const char *src1, const char *src2, size_t num);
#endif

#endif
