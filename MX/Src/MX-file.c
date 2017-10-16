#include "MX-file.h"
#include <string.h>
static FIL neopixel_file_stru[MX_FILE_NEOPIXEL_STRNUM];
static char neopixel_file_path[MX_FILE_NEOPIXEL_STRNUM][MX_FILE_NEOPIXEL_STRLEN];
static bool neopixel_file_lock[MX_FILE_NEOPIXEL_STRNUM] = {false, false};

/**
 * @brief  读字符串得到ID值
 */
static MX_NeoPixel_ID_t getID(const char *str)
{
  if (!strncasecmp(MX_NEOPIXEL_ID_STR(FRAMECNT), str))
  {
    return MX_NEOPIXEL_ID(FRAMECNT);
  }
  else if (!strncasecmp(MX_NEOPIXEL_ID_STR(FRAMLEN), str))
  {
    return MX_NEOPIXEL_ID(FRAMLEN);
  }
  else if (!strncasecmp(MX_NEOPIXEL_ID_STR(HZ), str))
  {
    return MX_NEOPIXEL_ID(HZ);
  }

  return MX_NEOPIXEL_ID(NULL);
}

/**
 * @brief  注册一个文件路径
 * @retvl  0--注册成功
 *         1--已存在
 *         -1-注册失败
 */
static int register_path(const char *str)
{
  int retvl = -1;

#if USE_DEBUG
  if (strlen(str) == 0)
    elog_w("Input file path is NULL");
  else if (strlen(str) >= MX_FILE_NEOPIXEL_STRLEN)
    elog_w("Input file path is too long");
#endif

  for (int i = 0; i < MX_FILE_NEOPIXEL_STRNUM; i++)
  {
    if ((strlen(neopixel_file_path[i]) == 0) && (neopixel_file_lock[i] == false))
    {
      strcpy(neopixel_file_path[i], str);
      neopixel_file_lock[i] = true;
      retvl = 0;
      break;
    }
    else if ((!strcasecmp(str, neopixel_file_path[i])) &&
             (neopixel_file_lock[i] == true))
    {
      retvl = 1;
      break;
    }
  }
  return retvl;
}
/**
 * @brief  注销一个文件路径
 * @retvl  0--注销成功
 *         1--未注册的路径
 *         -1-注册失败
 */
static int deregister_path(const char *str)
{
  int retvl = -1;

#if USE_DEBUG
  if (strlen(str) == 0)
    elog_w("Input file path is NULL");
  else if (strlen(str) >= MX_FILE_NEOPIXEL_STRLEN)
    elog_w("Input file path is too long");
#endif

  for (int i = 0; i < MX_FILE_NEOPIXEL_STRNUM; i++)
  {
    if (!strcasecmp(str, neopixel_file_path[i]))
    {
      if (neopixel_file_lock[i] == true)
        retvl = 0;
      else
        retvl = 1;
      neopixel_file_lock[i] = false;
      neopixel_file_path[i][0] = '\0';
      break;
    }
  }
  return retvl;
}

/**
 * @brief  得到一个文件路径指针
 * @retvl  FIL *
 */
static FIL *getFilePath(const char *path)
{
#if USE_DEBUG
  if (strlen(str) == 0)
    elog_w("Input file path is NULL");
  else if (strlen(str) >= MX_FILE_NEOPIXEL_STRLEN)
    elog_w("Input file path is too long");
#endif

  for (int i = 0; i < MX_FILE_NEOPIXEL_STRNUM; i++)
  {
    if (!strcasecmp(path, neopixel_file_path[i]) && neopixel_file_lock[i] == true)
    {
      return &neopixel_file_stru[i];
    }
  }

  elog_w("Can't fine file path:%s", path);

  return NULL;
}

bool MX_File_NeoPixel_OpenFile(const char *filepath)
{
  int res = register_path(filepath);

  if (res > 0)
  {
    log_w("Reopen a file:%s", filepath);
    return true;
  }
  else if (res < 0)
  {
    log_w("Can't register a file path");
    return false;
  }

  FIL *fpt = getFilePath(filepath);

  FRESULT res = f_open(fpt, (const TCHAR *)filepath, FA_READ);

  if (res != FR_OK)
  {
    log_w("Can't open neopixel file:%s:%d", filepath, res);
    deregister_path(filepath);
    return false;
  }

  return true;
}

bool MX_File_NeoPixel_CloseFile(const char *filepath)
{
  FIL *fpt = getFilePath(filepath);

  FRESULT res = f_close(fpt);

  if (res != FR_OK)
  {
    return false;
  }

  return true;
}

bool MX_File_NeoPixel_GetInfo(MX_NeoPixel_Structure_t *pt, const char *filepath)
{
  return true;
}

bool MX_File_NeoPixel_GetLine(const char *filepath, uint16_t line, void *buffer, size_t maxsize)
{
  return true;
}
