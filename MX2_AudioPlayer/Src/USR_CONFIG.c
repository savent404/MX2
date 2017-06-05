#include "USR_CONFIG.h"
#include "ff.h"
#include "freeRTOS.h"
#include "DEBUG.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "AF.h"
#include "path.h"
PARA_DYNAMIC_t USR;

const PARA_STATIC_t STATIC_USR = {

  .vol_warning = 2110,   //3.4v / 2 = 1.7v
  .vol_poweroff = 1923, //3.1v / 2
  .vol_chargecomplete = 4096, //4.18v / 2
  .filelimits = {
    .bank_max = 2,
    .trigger_in_max = 10,
    .trigger_out_max = 10,
    .trigger_B_max = 10,
    .trigger_C_max = 10,
    .trigger_D_max = 10,
    .trigger_E_max = 10,
  }
};

#if _USE_LFN //使用Heap模式， 为Fatfs长文件名模式提供一个缓存
static TCHAR LFN_BUF[100];
#endif

// 字符串函数 补全gcc未提供的一些字符串处理函数
char* upper(char *src);
#ifdef __GNUC__
int strcasecmp(const char *src1, const char *src2);
int strncasecmp(char *src1, char *src2, int num);

#endif

static const char name_string[][10] = {
    /**< Position:0~5  */
    "Vol", "Tpon", "Tpoff", "Tout", "Tin", "Ts_switch",
    /**< Position:6~11 */
    "Tautoin", "Tautooff", "Tmute", "TLcolor", "TBfreeze", "TBMode",
    /**< Position:12~17 */
    "TCfreeze", "TCMode", "TDfreeze", "TDMode", "TEtrigger", "TEMode",
    /**< Position:18~23 */
    "TLon", "TLoff", "Lbright", "Ldeep", "LMode", "CH1_Delay",
    /**< Position:24~29 */
    "CH2_Delay", "CH3_Delay", "CH4_Delay", "Unknow", "Unknow", "Unknow",
    /**< Position:30~35 */
    "Unknow", "MD", "MT", "CD", "CT", "CL",
    /**< Position:36 */
    "CW"
};

/** \brief 获取循环音频有效负载量
 *
 * \param 动态配置信息结构体指针
 * \return Error Code
 *
 */
static uint8_t get_humsize(PARA_DYNAMIC_t* pt);
/** \brief 得到用户配置文件(config.txt)中的信息
 *
 * \param pt 用户动态配置参数，但初始化后不会更改
 * \return Error Code
 *
 */
static uint8_t get_config(PARA_DYNAMIC_t *pt, FIL *file);

/** \brief 得到Trigger相关信息
 *
 * \param triggerid 0~3, 对应TriggerB-TriggerE
 * \param Bank 需要扫描的Bank
 * \param pt 用户动态配置参数，但初始化后不会更改
 * \return Error Code
 *
 */
static uint8_t get_trigger_para(uint8_t triggerid, uint8_t Bank, PARA_DYNAMIC_t *pt);

/** \brief 得到Accent.txt中的信息
 *
 * \param Bank 需要扫描的Bank
 * \param pt 用户动态配置参数，但初始化后不会更改
 * \return Error Code
 *
 */
static uint8_t get_accent_para(uint8_t Bank, PARA_DYNAMIC_t *pt);

/** \brief Initialize User's Configuration Parameter
 *
 * \param  None
 * \return Error Code
 *
 */
uint8_t usr_config_init(void)
{
  FRESULT f_err;
  FIL *file = (FIL*)pvPortMalloc(sizeof(FIL));
  /**< 获取Bank数量 */
  {
    DIR dir;
    FILINFO info;
    #if _USE_LFN
    info.lfname = LFN_BUF;
    #endif
    /**< Open Dir:[0:/] */
    if ((f_err = f_opendir(&dir, "0:/")) != FR_OK)
    {
      DEBUG(0, "Can't Open Dir(0:/):%d", f_err);
      return 1;
    }
    /**< Get Number of Banks */
    USR.nBank = 0;
    while ((f_readdir(&dir, &info) == FR_OK) && info.fname[0] != '\0') {
      if ((info.fattrib & AM_DIR) && (info.fname[0] == 'B' || info.fname[0] == 'b')) USR.nBank += 1;
    }
    /**< Close Dir */
    if ((f_err = f_closedir(&dir)) != FR_OK)
    {
      DEBUG(0, "Can't Close Dir:%d", f_err);
      return 1;
    }
    /**< Bank number limits */
    if (STATIC_USR.filelimits.bank_max < USR.nBank) {
      USR.nBank = STATIC_USR.filelimits.bank_max;
    }
  }
  USR.humsize = (HumSize_t*)pvPortMalloc(sizeof(HumSize_t)*USR.nBank);
  USR.config = (USR_CONFIG_t*)pvPortMalloc(sizeof(USR_CONFIG_t));
  USR._config = (USR_CONFIG_t*)pvPortMalloc(sizeof(USR_CONFIG_t)*USR.nBank);
  USR.BankColor = (uint32_t*)pvPortMalloc(sizeof(uint32_t)*2*USR.nBank);
  USR.triggerB = (TRIGGER_PATH_t*)pvPortMalloc(sizeof(TRIGGER_PATH_t)*USR.nBank);
  USR.triggerC = (TRIGGER_PATH_t*)pvPortMalloc(sizeof(TRIGGER_PATH_t)*USR.nBank);
  USR.triggerD = (TRIGGER_PATH_t*)pvPortMalloc(sizeof(TRIGGER_PATH_t)*USR.nBank);
  USR.triggerE = (TRIGGER_PATH_t*)pvPortMalloc(sizeof(TRIGGER_PATH_t)*USR.nBank);
  USR.triggerIn = (TRIGGER_PATH_t*)pvPortMalloc(sizeof(TRIGGER_PATH_t)*USR.nBank);
  USR.triggerOut = (TRIGGER_PATH_t*)pvPortMalloc(sizeof(TRIGGER_PATH_t)*USR.nBank);

  /**< 获取Hum 有效负载 */
  if (get_humsize(&USR))
  {
    return 1;
  }

  /**< 获取CONFIG.txt中的信息,包含各种配置信息以及功率LED灯的颜色信息*/
  if ((f_err = f_open(file, PATH_CONFIG, FA_READ)) != FR_OK)
  {
    DEBUG(0, "Can't find %s:%d", PATH_CONFIG, f_err);
    return 1;
  }
  f_err = get_config(&USR, file);
  if (f_err) return f_err;
  if ((f_err = f_close(file)) != FR_OK)
  {
    DEBUG(0, "Can't close %s:%d", PATH_CONFIG, f_err);
    return 1;
  }

  int i;
  for (i = 0; i < USR.nBank; i++)
  {
    memcpy(USR._config + i, USR.config, sizeof(USR_CONFIG_t));
  }
  vPortFree(USR.config);
  for (i = 0; i < USR.nBank; i++)
  {
    char path[20];
    sprintf(path, "0:/Bank%d/"REPLACE_NAME, i+1);
    if (f_open(file, path, FA_READ) != FR_OK) continue;
    USR.config = USR._config + i;
    get_config(&USR, file);
    f_close(file);
  }

  /**< 获取各Bank下TriggerX的信息，包括总数以及各文件的文件名 */
  for (uint8_t nBank = 0; nBank < USR.nBank; nBank++)
  {
    f_err = get_trigger_para(0, nBank + 1, &USR); //Trigger B
    if (f_err) return f_err;
    f_err = get_trigger_para(1, nBank + 1, &USR); //Trigger C
    if (f_err) return f_err;
    f_err = get_trigger_para(2, nBank + 1, &USR); //Trigger D
    if (f_err) return f_err;
    f_err = get_trigger_para(3, nBank + 1, &USR); //Trigger E
    if (f_err) return f_err;
    f_err = get_trigger_para(4, nBank + 1, &USR); //Trigger In
    if (f_err) return f_err;
    f_err = get_trigger_para(5, nBank + 1, &USR); //Trigger Out
    if (f_err) return f_err;
  }

  /**< 获取每个bank中的Accent.txt, 包括动作延时时间以及动作信息 */
  USR.accent = (Accent_t*)pvPortMalloc(sizeof(Accent_t)*USR.nBank);
  for (uint8_t nBank = 0; nBank < USR.nBank; nBank++)
  {
    f_err = get_accent_para(nBank + 1, &USR);
    if (f_err) return f_err;
  } USR.config = USR._config;

  ///以上任意一个错误都是致命的
  ///所以不再对动态分配的内存进行回收，只有在正确返回的情况下考虑回收内存
  vPortFree(file);
  return 0;
}

static uint8_t get_humsize(PARA_DYNAMIC_t* pt)
{
  FIL file;
  FRESULT f_err;
  char path[30];
  uint32_t buf;
  UINT f_cnt;

  for (uint8_t i = 0; i < pt->nBank; i++)
  {
    sprintf(path, "0:/Bank%d/hum.wav", i+1);
    if((f_err = f_open(&file, path, FA_READ)) != FR_OK)
    {
      DEBUG(0, "Open %s Error:%d", path, f_err);
      return 1;
    } f_lseek(&file, sizeof(struct _AF_PCM) + 4);
    f_read(&file, &buf, 4, &f_cnt);
    pt->humsize[i] = buf;
    f_close(&file);
  }
  return 0;
}

static uint8_t get_trigger_para(uint8_t triggerid, uint8_t Bank, PARA_DYNAMIC_t *pt)
{
  DIR dir; FILINFO info; char path[25]; FRESULT f_err;
  #if _USE_LFN
  info.lfname = LFN_BUF;
  #endif
  uint8_t trigger_cnt = 0;
  switch (triggerid)
  {
    case 0: sprintf(path, "0://Bank%d/"TRIGGER(B), Bank); break;
    case 1: sprintf(path, "0://Bank%d/"TRIGGER(C), Bank); break;
    case 2: sprintf(path, "0://Bank%d/"TRIGGER(D), Bank); break;
    case 3: sprintf(path, "0://Bank%d/"TRIGGER(E), Bank); break;
    case 4: sprintf(path, "0://Bank%d/"TRIGGER(IN), Bank); break;
    case 5: sprintf(path, "0://Bank%d/"TRIGGER(OUT), Bank); break;
  }
  // if (triggerid < 4)
  //   sprintf(path, "0://Bank%d/Trigger_%c", Bank, triggerid + 'B');
  // else if (triggerid == 4)
  //   sprintf(path, "0://Bank%d/In", Bank);
  // else if (triggerid == 5)
  //   sprintf(path, "0://Bank%d/Out", Bank);

  if ((f_err = f_opendir(&dir, path)) != FR_OK) {
    DEBUG(0, "Open Bank%d Trigger%c Error:%d", Bank, triggerid +'B', f_err);
    return 1;
  }
  while ((f_err = f_readdir(&dir, &info)) == FR_OK && info.fname[0] != '\0') {
      trigger_cnt += 1;
  } f_closedir(&dir);

  // Number limits
  switch (triggerid) {
    case 0:
      if (trigger_cnt > TRIGGER_MAX_NUM(B))
        trigger_cnt = TRIGGER_MAX_NUM(B);
      break;
    case 1:
      if (trigger_cnt > TRIGGER_MAX_NUM(C))
        trigger_cnt = TRIGGER_MAX_NUM(C);
      break;
    case 2:
      if (trigger_cnt > TRIGGER_MAX_NUM(D))
        trigger_cnt = TRIGGER_MAX_NUM(D);
      break;
    case 3:
      if (trigger_cnt > TRIGGER_MAX_NUM(E))
        trigger_cnt = TRIGGER_MAX_NUM(E);
      break;
    case 4:
      if (trigger_cnt > TRIGGER_MAX_NUM(in))
        trigger_cnt = TRIGGER_MAX_NUM(in);
      break;
    case 5:
      if (trigger_cnt > TRIGGER_MAX_NUM(out))
        trigger_cnt = TRIGGER_MAX_NUM(out);
      break;
  }

  switch (triggerid) {
    case 0: (pt->triggerB + Bank - 1)->number = trigger_cnt;
            (pt->triggerB + Bank - 1)->path_arry = (char*)pvPortMalloc(sizeof(char)*30*trigger_cnt);
            break;
    case 1: (pt->triggerC + Bank - 1)->number = trigger_cnt;
            (pt->triggerC + Bank - 1)->path_arry = (char*)pvPortMalloc(sizeof(char)*30*trigger_cnt);
            break;
    case 2: (pt->triggerD + Bank - 1)->number = trigger_cnt;
            (pt->triggerD + Bank - 1)->path_arry = (char*)pvPortMalloc(sizeof(char)*30*trigger_cnt);
            break;
    case 3: (pt->triggerE + Bank - 1)->number = trigger_cnt;
            (pt->triggerE + Bank - 1)->path_arry = (char*)pvPortMalloc(sizeof(char)*30*trigger_cnt);
            break;
    case 4: (pt->triggerIn + Bank - 1)->number = trigger_cnt;
            (pt->triggerIn + Bank - 1)->path_arry  = (char*)pvPortMalloc((sizeof(char)*30*trigger_cnt));
            break;
    case 5: (pt->triggerOut + Bank - 1)->number = trigger_cnt;
            (pt->triggerOut + Bank - 1)->path_arry = (char*)pvPortMalloc((sizeof(char)*30*trigger_cnt));
            break;
  }


  if ((f_err = f_opendir(&dir, path)) != FR_OK) {
    DEBUG(0, "Open Bank%d Trigger%c Error:%d", Bank, triggerid+'B', f_err);
    return 1;
  }
  uint16_t i = 0;
  while ((f_err = f_readdir(&dir, &info)) == FR_OK && info.fname[0] != '\0' && ++i <= trigger_cnt) {
    switch (triggerid) {
      case 0: strcpy((pt->triggerB + Bank - 1)->path_arry + (30)*(i - 1), info.fname); break;
      case 1: strcpy((pt->triggerC + Bank - 1)->path_arry+ (30)*(i - 1), info.fname); break;
      case 2: strcpy((pt->triggerD + Bank - 1)->path_arry + (30)*(i - 1), info.fname); break;
      case 3: strcpy((pt->triggerE + Bank - 1)->path_arry + (30)*(i - 1), info.fname); break;
      case 4: strcpy((pt->triggerIn + Bank - 1)->path_arry + (30)*(i - 1), info.fname); break;
      case 5: strcpy((pt->triggerOut + Bank - 1)->path_arry + (30)*(i - 1), info.fname); break;
    }
  } f_closedir(&dir);
  return 0;
}
#ifdef __GUNC__
int strcasecmp(const char *src1, const char *src2)
{
  char *pt1 = (char*)pvPortMalloc(strlen(src1)*sizeof(char) + 1),
       *pt2 = (char*)pvPortMalloc(strlen(src1)*sizeof(char) + 1);
  strcpy(pt1, src1);
  strcpy(pt2, src2);
  int res = strcmp(upper(pt1), upper(pt2));
  vPortFree(pt1);
  vPortFree(pt2);
  return res;
}
#endif
static uint8_t get_accent_para(uint8_t Bank, PARA_DYNAMIC_t *pt)
{
  FIL file;
  FRESULT f_err;
  uint16_t cnt = 0;
  char path[30];

  sprintf(path, "0:/Bank%d/Accent.txt", Bank);
  if ((f_err = f_open(&file, path, FA_READ)) != FR_OK)
  {
    DEBUG(0, "Can't Open %s:%d", path, f_err);
    return 1;
  }
  while (f_gets(path, 30, &file) != 0)
  {
    if (path[0] == 'T' || path[0] == 't')
    {
      sscanf(path, "%*[^=]=%ld", &((pt->accent+Bank-1)->delay_ms));
    } else {
      cnt += 1;
    }
  }
  f_close(&file);
  (pt->accent+Bank-1)->N = cnt;
  (pt->accent+Bank-1)->arry = (uint8_t*)pvPortMalloc(sizeof(uint8_t)*cnt);


  cnt = 0;
  sprintf(path, "0:/Bank%d/Accent.txt", Bank);
  if ((f_err = f_open(&file, path, FA_READ)) != FR_OK)
  {
    DEBUG(0, "Can't Open %s:%d", path, f_err);
    return 1;
  }
  while (f_gets(path, 30, &file) != 0)
  {
    if (path[0] == 'T' || path[0] == 't')
    {
    } else {
      char *spt = path;
      uint8_t buf = 0;

      while (*spt == '0' || *spt == '1')
      {
        buf <<= 1;
        if (*spt == '1')
        {
          buf |= 1;
        } spt += 1;
      }
      (pt->accent+Bank-1)->arry[cnt] = buf;
      cnt += 1;
    }
  } f_close(&file);
  return 0;
}

char* upper(char *src)
{
  char *pt = src;
  while (*pt)
  {
    if (*pt <= 'z' && *pt >= 'a')
    {
      *pt -= 'z' - 'Z';
    } pt += 1;
  } return src;
}
#ifdef __GNUC__
int strncasecmp(char *src1, char *src2, int num)
{
  return strncmp(upper(src1), upper(src2), num);
}
#endif

static int GetName(char *line, char *name) {
  int res = 0, cnt = 0;
  if (!strncasecmp(line, "BANK", 4)) {
    sscanf(line, "%*[ BANKbank]%d", &res);
    return res;
  } else if (!strncasecmp(line, "FBANK", 5)) {
    sscanf(line, "%*[ FBANKfbank]%d", &res);
    return -res;
  }
  while (*line != '=') {
    if (*line == ' ' || *line == '\t') {
      line += 1;
    } if (*line == '\0') {
      *name = 0;
      return res;
    } if (cnt >= 100) {
      *name = 0;
      return res;
    } *name++ = *line++;
    cnt++;
  } *name = 0;
  return res;
}
static int GetMultiPara(char *line)
{
  int res = 0;
  char *pt = line;
  while (*pt != '=') pt += 1;
  pt += 1;
  while (*pt == ' ') pt += 1;
  while (*pt != '\n' && *pt != '/' && *pt != '\0') {
    if (*pt == '1') res |= 0x01;
    if (*pt == '2') res |= 0x02;
    if (*pt == '3') res |= 0x04;
    if (*pt == '4') res |= 0x08;
    pt += 1;
  }
  return res;
}
static uint8_t get_config(PARA_DYNAMIC_t *pt, FIL *file)
{
  char sbuffer[50];
  char name[20];
  char *spt;
  while ((spt = f_gets(sbuffer, 50, file)) != 0)
  {
    int8_t res = GetName(spt, name);
    if (res > 0) {
      uint16_t buf[4];
      if (res > USR.nBank) continue;
      sscanf(spt, "%*[^=]=%hd,%hd,%hd,%hd", buf,buf+1,buf+2,buf+3);
      pt->BankColor[res*4 + 0 - 4] = *(uint32_t*)(buf+0);
      pt->BankColor[res*4 + 1 - 4] = *(uint32_t*)(buf+1);
      continue;
    } else if (res < 0) {
      uint16_t buf[4];
      if (-res > USR.nBank) continue;
      sscanf(spt, "%*[^=]=%hd,%hd,%hd,%hd", buf,buf+1,buf+2,buf+3);
      pt->BankColor[-1*res*4 + 2 - 4] = *(uint32_t*)(buf+0);
      pt->BankColor[-1*res*4 + 3 - 4] = *(uint32_t*)(buf+1);
      continue;
    }
    for (res = 0; res < sizeof(name_string)/10; res++) {
        if (!strcasecmp(name, name_string[res])) break;
    }
    switch (res) {
      case 0: sscanf(spt,"%*[^=]=%d", &(pt->config->Vol));break;
      case 1: sscanf(spt,"%*[^=]=%hd", &(pt->config->Tpon));break;
      case 2: sscanf(spt,"%*[^=]=%hd", &(pt->config->Tpoff));break;
      case 3: sscanf(spt,"%*[^=]=%hd", &(pt->config->Tout));break;
      case 4: sscanf(spt,"%*[^=]=%hd", &(pt->config->Tin));break;
      case 5: sscanf(spt,"%*[^=]=%hd", &(pt->config->Ts_switch));break;
      case 6: sscanf(spt,"%*[^=]=%ld", &(pt->config->Tautoin));break;
      case 7: sscanf(spt,"%*[^=]=%ld", &(pt->config->Tautooff));break;
      case 8: sscanf(spt,"%*[^=]=%hd", &(pt->config->Tmute));break;
      case 9: sscanf(spt,"%*[^=]=%hd", &(pt->config->TLcolor));break;
      case 10: sscanf(spt,"%*[^=]=%hd", &(pt->config->TBfreeze));break;
      case 11: pt->config->TBMode=GetMultiPara(spt);break;
      case 12: sscanf(spt,"%*[^=]=%hd", &(pt->config->TCfreeze));break;
      case 13: pt->config->TCMode=GetMultiPara(spt);break;
      case 14: sscanf(spt,"%*[^=]=%hd", &(pt->config->TDfreeze));break;
      case 15: pt->config->TDMode=GetMultiPara(spt);break;
      case 16: sscanf(spt,"%*[^=]=%hd", &(pt->config->TEtrigger));break;
      case 17: pt->config->TEMode=GetMultiPara(spt);break;
      case 18: sscanf(spt,"%*[^=]=%hd", &(pt->config->TLon));break;
      case 19: sscanf(spt,"%*[^=]=%hd", &(pt->config->TLoff));break;
      case 20: sscanf(spt,"%*[^=]=%hd", &(pt->config->Lbright));break;
      case 21: sscanf(spt,"%*[^=]=%hd", &(pt->config->Ldeep));break;
      case 22: sscanf(spt,"%*[^=]=%hd", &(pt->config->LMode));break;
      case 23: sscanf(spt,"%*[^=]=%hd", (pt->config->ChDelay));break;
      case 24: sscanf(spt,"%*[^=]=%hd", (pt->config->ChDelay+1));break;
      case 25: sscanf(spt,"%*[^=]=%hd", (pt->config->ChDelay+2));break;
      case 26: sscanf(spt,"%*[^=]=%hd", (pt->config->ChDelay+3));break;
      /*case 27: sscanf(spt,"%*[^=]=%d", &(pt->config->Sl));break;
      case 28: sscanf(spt,"%*[^=]=%d", &(pt->config->Sh));break;
      case 29: sscanf(spt,"%*[^=]=%d", &(pt->config->Cl));break;
      case 30: sscanf(spt,"%*[^=]=%d", &(pt->config->Ch));break;*/
      case 31: sscanf(spt,"%*[^=]=%hd", &(pt->config->MD));break;
      case 32: sscanf(spt,"%*[^=]=%hd", &(pt->config->MT));break;
      case 33: sscanf(spt,"%*[^=]=%hd", &(pt->config->CD));break;
      case 34: sscanf(spt,"%*[^=]=%hd", &(pt->config->CT));break;
      case 35: sscanf(spt,"%*[^=]=%hd", &(pt->config->CL));break;
      case 36: sscanf(spt,"%*[^=]=%hd", &(pt->config->CW));break;
    }
  } return 0;
}
