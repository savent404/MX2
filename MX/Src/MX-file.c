#include "mx-file.h"
#include <string.h>

static MX_NeoPixel_Structure_t fmap[MX_FILE_NEOPIXEL_STRNUM];

/**
 * @brief  打开Neopixel文件
 * @NOTE   所有操作基于文件已打开的状态
 * @retvl  Error = false
 */
const MX_NeoPixel_Structure_t* MX_File_NeoPixel_OpenFile(const char *filepath)
{
  if (strlen(filepath) > MX_FILE_NEOPIXEL_STRLEN)
  {
    log_e("Buffer overflow");
    return NULL;
  }
  else if (strlen(filepath) == 0)
  {
    log_w("path is empty");
    return NULL;
  }

  for (int i = 0; i < MX_FILE_NEOPIXEL_STRNUM; i++)
  {
    if (!strlen(fmap[i].filepath))
    {
      FRESULT res = f_open(&(fmap[i].file), filepath, FA_READ);
      if (res != FR_OK)
      {
        log_w("Open file error");
        return NULL;
      }
      strcpy(fmap[i].filepath, filepath);

      // get info
      fmap[i].frame_cnt = 50;
      fmap[i].frame_len = 50;
      fmap[i].Hz = 20;
      fmap[i].payload_offset = 0x1E;
      return (const MX_NeoPixel_Structure_t*)(fmap + i);
    }
  }
  return NULL;
}

/**
 * @brief  关闭Neopixel文件
 */
bool MX_File_NeoPixel_CloseFile(MX_NeoPixel_Structure_t *pt)
{
  f_close(&(pt->file));
  pt->filepath[0] = '\0';
  return true;
}

/**
 * @brief  得到一行数据
 * @NOTE   在openfile操作后使用
 */
bool MX_File_NeoPixel_GetLine(const MX_NeoPixel_Structure_t *pt, uint16_t line, void *buffer, size_t maxsize)
{
  FRESULT res;
  UINT read_len;
  UINT cnt;

  taskENTER_CRITICAL();
  res = f_lseek(&(pt->file), pt->payload_offset + pt->frame_len * 3 * (line % (pt->frame_cnt)));
  if (res != FR_OK)
  {
    taskEXIT_CRITICAL();
    log_w("seek file error:%d", (int)res);
    return false;
  }

  read_len = pt->frame_len * 3;
  if (maxsize != 0 && read_len > maxsize)
  {
    read_len = maxsize;
  }
  res = f_read(&(pt->file), buffer, read_len, &cnt);
  if (res != FR_OK)
  {
    taskEXIT_CRITICAL();
    log_w("read file error:%d", (int)res);
    return false;
  }
  taskEXIT_CRITICAL();
  return true;
}
