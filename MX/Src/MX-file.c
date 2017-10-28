#include "mx-file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MX_NeoPixel_Structure_t fmap[MX_FILE_NEOPIXEL_STRNUM];

static void keycatch(MX_NeoPixel_Structure_t *pt, const char *buffer, uint8_t len);
/**
 * @brief  打开Neopixel文件
 * @NOTE   所有操作基于文件已打开的状态
 * @retvl  Error = false
 */
const MX_NeoPixel_Structure_t *MX_File_NeoPixel_OpenFile(const char *filepath)
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
      char buffer[50];
      UINT cnt;

      res = f_read(&(fmap[i].file), buffer, 2, &cnt);
      if (res != FR_OK)
      {
        log_w("can't read file");
      }
      buffer[2] = '\0';
      fmap[i].payload_offset = atoi(buffer);
      res = f_read(&(fmap[i].file), buffer, fmap[i].payload_offset - 2, &cnt);
      buffer[fmap[i].payload_offset - 1] = '\0';
      keycatch(fmap + i, buffer, fmap[i].payload_offset - 1);
      return (const MX_NeoPixel_Structure_t *)(fmap + i);
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
  if (read_len != cnt)
  {
    log_e("read error: want(%d), give(%d)", read_len, cnt);
  }
  taskEXIT_CRITICAL();
  return true;
}
static bool isChara(int a)
{
  if (a <= 'z' && a >= 'a')
    return true;
  else if (a <= 'Z' && a >= 'A')
    return true;
  else if (a <= '9' && a >= '0')
    return true;
  else
    return false;
}

bool MX_File_GetRandFileName(const char *dir, const int num, char *filename)
{
  static DIR d;
  static FILINFO info;
  FRESULT res;
  int total_num = 0;

  if ((res = f_opendir(&d, dir)) != FR_OK)
  {
    log_w("can't open dir(%s):%d", dir, (int)res);
    return false;
  }

  while ((res = f_readdir(&d, &info)) == FR_OK)
    if (info.fattrib & AM_DIR == 0)
      total_num++;

  if ((res = f_closedir(&d)) != FR_OK)
  {
    log_e("can't close dir(%s):%d", dir, (int)res);
    return false;
  }

  if (num && total_num > num)
    total_num = num;

  total_num = rand() % total_num;

  if ((res = f_opendir(&d, dir)) != FR_OK)
  {
    log_w("can't open dir(%s):%d", dir, (int)res);
    return false;
  }

  for (int i = 0; i < total_num;)
  {
    f_readdir(&d, &info);
    if (info.fattrib & AM_DIR == 0)
      i++;
  }

  if ((res = f_closedir(&d)) != FR_OK)
  {
    log_e("can't close dir(%s):%d", dir, (int)res);
    return false;
  }

  filename[0] = '\0';

  strcpy(filename, info.fname);

  return true;
}

static void keycatch(MX_NeoPixel_Structure_t *pt, const char *buffer, uint8_t len)
{
  uint8_t flag = 0;
  uint8_t step = 0;
  char buf[2][10];
  uint8_t pos[2] = {0, 0};
  for (int i = 0; i < len; i++)
  {
    if (buffer[i] == '{')
    {
      flag = 1;
    }
    else if (buffer[i] == '}')
    {
      flag = 2;
      break;
    }
    else if (buffer[i] == ':')
    {
      step += 1;
    }
    else if (buffer[i] == ';')
    {
      buf[0][pos[0]] = '\0';
      buf[1][pos[1]] = '\0';
      log_v("searching key %s:%s", buf[0], buf[1]);
      if (!strcasecmp(MX_NEOPIXEL_ID_STR(FRAMECNT), buf[0]))
      {
        pt->frame_cnt = atoi(buf[1]);
      }
      else if (!strcasecmp(MX_NEOPIXEL_ID_STR(FRAMELEN), buf[0]))
      {
        pt->frame_len = atoi(buf[1]);
      }
      else if (!strcasecmp(MX_NEOPIXEL_ID_STR(HZ), buf[0]))
      {
        pt->Hz = atoi(buf[1]);
      }
      else
      {
        log_w("not match key %s:%s", buf[0], buf[1]);
      }
      step = 0;
      pos[0] = 0, pos[1] = 0;
    }

    if (isChara(buffer[i]) && flag == 1)
    {
      buf[step][pos[step]] = buffer[i];
      pos[step] += 1;
    }
    else
    {
      continue;
    }
  }
}
// fatfs LFN支持
void MX_File_InfoLFN_Init(FILINFO *info)
{
#if _USE_LFN
  info->lfname = (TCHAR *)pvPortMalloc(sizeof(TCHAR) * 120);
  if (info->lfname == NULL)
    log_e("no enough mem");
#endif
}
void MX_File_InfoLFN_DeInit(FILINFO *info)
{
#if _USE_LFN
  vPortFree(info->lfname);
#endif
}

// String.h 补全
char *upper(char *src)
{
  char *pt = src;
  while (*pt)
  {
    if (*pt <= 'z' && *pt >= 'a')
    {
      *pt -= 'z' - 'Z';
    }
    pt += 1;
  }
  return src;
}

#ifdef __GNUC__
int strcasecmp(const char *src1, const char *src2)
{
  char *pt1 = (char *)pvPortMalloc(strlen(src1) * sizeof(char) + 1),
       *pt2 = (char *)pvPortMalloc(strlen(src1) * sizeof(char) + 1);
  strcpy(pt1, src1);
  strcpy(pt2, src2);
  int res = strcmp(upper(pt1), upper(pt2));
  vPortFree(pt1);
  vPortFree(pt2);
  return res;
}

int strncasecmp(const char *src1, const char *src2, size_t num)
{
  return strncmp(upper(src1), upper(src2), num);
}
#endif
