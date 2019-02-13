#include "param.h"
#include "debug.h"
#include "ff.h"
#include "fatfs.h"

FATFS fatfs;

__MX_WEAK bool MX_PARAM_Init(void)
{
    MX_FATFS_Init();

  int f_err;
  if ((f_err = f_mount(&fatfs, "0:/", 1)) != FR_OK)
  {
    DEBUG(0, "mount error:%d", f_err);
    return false;
  }
  if ((f_err = usr_config_init()) != 0)
  {
    DEBUG(0, "User Configuration has some problem:%d", f_err);
    return false;
  }
  return true;
}

__MX_WEAK const char* MX_PARAM_GetPrefix(void)
{
    return "0:";
}
