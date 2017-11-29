#include "USR_CONFIG.h"
#include <stdbool.h>

PARA_DYNAMIC_t USR;

const PARA_STATIC_t STATIC_USR = {

    .vol_warning = 2240,        //3.28v / 2 = 1.64v
    .vol_poweroff = 2116,       //3.1v / 2 = 1.55v
    .vol_chargecomplete = 2853, //4.18v / 2 = 2.09v
};

File_NumberLimits_t LIMITS_USR = {
        .bank_max = 3,
        .trigger_in_max = 10,
        .trigger_out_max = 10,
        .trigger_B_max = 10,
        .trigger_C_max = 10,
        .trigger_D_max = 10,
        .trigger_E_max = 10,
        .trigger_F_max = 10,
        .trigger_in_x_max = 10,
        .trigger_in_y_max = 10,
        .trigger_in_z_max = 10,
        .trigger_out_x_max = 10,
        .trigger_out_y_max = 10,
        .trigger_out_z_max = 10,
};
#define CONST_KEY_LEN 25
static const char name_string[][CONST_KEY_LEN] = {
    /**< Position:0~5  */
    "Vol", "Tpon", "Tpoff", "Tout", "Tin", "Ts_switch",
    /**< Position:6~11 */
    "Tautoin", "Tautooff", "Tmute", "TLcolor", "TBfreeze", "TBMode",
    /**< Position:12~17 */
    "TCfreeze", "TCMode", "TDfreeze", "TDMode", "TEtrigger", "TEMode",
    /**< Position:18~23 */
    "TLon", "TLoff", "Lbright", "Ldeep", "LMode", "CH1_Delay",
    /**< Position:24~29 */
    "CH2_Delay", "CH3_Delay", "CH4_Delay", "T_Breath", "Out_Delay", "LEDMASK",
    /**< Position:30~35 */
    "DriverMode", "MD", "MT", "CD", "CT", "CL",
    /**< Position:36~39*/
    "CW", "Direction", "ShakeOutG", "ShakeInG",
    /**< Position:40~43*/
    "LockupHold", "Lowpower", "PowerSavingPerecnts", "Bank_max"};

/** \brief 获取循环音频有效负载量
 *
 * \param 动态配置信息结构体指针
 * \return Error Code
 *
 */
static uint8_t get_humsize(PARA_DYNAMIC_t *pt);

/** \brief 设定一些默认配置参数
 *
 * \param pt 用户动态配置参数，在动态获取之前需设定一些默认参数
 * \return None
 *
 */
static void set_config(PARA_DYNAMIC_t *pt);

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

  USR.filelimits = LIMITS_USR;

  FRESULT f_err;
  FIL *file = (FIL *)pvPortMalloc(sizeof(FIL));

  USR.bank_now = MX_File_GetBank();

  // 获取bank数量
  USR.nBank = MX_File_SearchDir("0:/", "Bank", NULL);
  if (USR.filelimits.bank_max < USR.nBank && USR.filelimits.bank_max != 0)
  {
    USR.nBank = USR.filelimits.bank_max;
  }

  USR.bank_now %= USR.nBank;

  log_v("Bank now:%d", USR.bank_now);

  uint32_t req_mem = 0;
#define REQUES_MEM(sum, x) (sum += (x))

  USR.humsize = (HumSize_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(HumSize_t) * USR.nBank));
  USR.config = (USR_CONFIG_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(USR_CONFIG_t)));
  USR._config = (USR_CONFIG_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(USR_CONFIG_t) * USR.nBank));
  USR.BankColor = (uint32_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(uint32_t) * 6 * USR.nBank));
  USR.triggerB = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerC = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerD = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerE = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerF = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerIn = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerIn_X = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerIn_Y = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerIn_Z = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerOut = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerOut_X = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerOut_Y = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.triggerOut_Z = (TRIGGER_PATH_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(TRIGGER_PATH_t)));
  USR.accent = (Accent_t *)pvPortMalloc(REQUES_MEM(req_mem, sizeof(Accent_t) * USR.nBank));
  log_i("malloc for USR structure cost %d", req_mem);
  set_config(&USR);
  /**< 获取Hum 有效负载 */
  if (get_humsize(&USR))
  {
    return 1;
  }

  /**< 获取CONFIG.txt中的信息,包含各种配置信息以及功率LED灯的颜色信息*/
  if ((f_err = f_open(file, PATH_CONFIG, FA_READ)) != FR_OK)
  {
    log_e("Can't find %s:%d", PATH_CONFIG, f_err);
    return 1;
  }

  f_err = get_config(&USR, file);
  if (f_err)
    return f_err;
  if ((f_err = f_close(file)) != FR_OK)
  {
    log_e("Can't close %s:%d", PATH_CONFIG, f_err);
    return 1;
  }

  for (int i = 0; i < USR.nBank; i++)
  {
    memcpy(USR._config + i, USR.config, sizeof(USR_CONFIG_t));
  }
  vPortFree(USR.config);
  for (int i = 0; i < USR.nBank; i++)
  {
    char path[20];
    sprintf(path, "0:/Bank%d/" REPLACE_NAME, i + 1);
    log_v("Try to read effect file:%s", path);
    if (f_open(file, path, FA_READ) != FR_OK)
      continue;
    USR.config = USR._config + i;
    get_config(&USR, file);
    f_close(file);
  }

  /**< 获取当前Bank下TriggerX的信息，包括总数以及各文件的文件名 */
  for (int i = 0; i < 12; i++)
  {
    get_trigger_para(i, USR.bank_now + 1, &USR);
  }

  /**< 获取每个bank中的Accent.txt, 包括动作延时时间以及动作信息 */
  for (uint8_t nBank = 0; nBank < USR.nBank; nBank++)
  {
    f_err = get_accent_para(nBank, &USR);
    if (f_err)
      return f_err;
  }
  USR.config = USR._config;

  ///以上任意一个错误都是致命的
  ///所以不再对动态分配的内存进行回收，只有在正确返回的情况下考虑回收内存
  vPortFree(file);
  return 0;
}

/**
 * @brief  更新Bank动态数据
 */
uint8_t usr_config_update(void)
{
  /**< 获取当前Bank下TriggerX的信息，包括总数以及各文件的文件名 */
  for (int i = 0; i < 12; i++)
  {
    get_trigger_para(i, USR.bank_now + 1, &USR);
  }
  return 0;
}

static uint8_t get_humsize(PARA_DYNAMIC_t *pt)
{
  FIL file;
  FRESULT f_err;
  char path[30];
  uint32_t buf;
  UINT f_cnt;

  for (uint8_t i = 0; i < pt->nBank; i++)
  {
    sprintf(path, "0:/Bank%d/hum.wav", i + 1);
    if ((f_err = f_open(&file, path, FA_READ)) != FR_OK)
    {
      log_e("Open %s Error:%d", path, f_err);
      return 1;
    }
    f_lseek(&file, sizeof(struct _AF_PCM) + 4);
    f_read(&file, &buf, 4, &f_cnt);
    pt->humsize[i] = buf;
    f_close(&file);
  }
  return 0;
}

/**
 * @note  分配将先释放之前分配的内存
 */
static uint8_t get_trigger_para(uint8_t triggerid, uint8_t Bank, PARA_DYNAMIC_t *pt)
{
  uint32_t mem_req = 0;
  uint32_t max_trigger_cnt = 0;
  TRIGGER_PATH_t *pTriggerPath;
  bool goto_flag = false;
  uint8_t trigger_cnt = 0;
  DIR dir;
  FILINFO info;
  char path[25];
  FRESULT f_err;

  MX_File_InfoLFN_Init(&info);

get_trigger_again:

  if (goto_flag == false)
  {
    switch (triggerid)
    {
    case 0:
      sprintf(path, "0://Bank%d/" TRIGGER(B), Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(B);
      pTriggerPath = pt->triggerB;
      break;
    case 1:
      sprintf(path, "0://Bank%d/" TRIGGER(C), Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(C);
      pTriggerPath = pt->triggerC;
      break;
    case 2:
      sprintf(path, "0://Bank%d/" TRIGGER(D), Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(D);
      pTriggerPath = pt->triggerD;
      break;
    case 3:
      sprintf(path, "0://Bank%d/" TRIGGER(E), Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(E);
      pTriggerPath = pt->triggerE;
      break;
    case 4:
      sprintf(path, "0://Bank%d/" TRIGGER(F), Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(F);
      pTriggerPath = pt->triggerF;
      break;
    case 5:
      sprintf(path, "0://Bank%d/" TRIGGER(IN), Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(in);
      pTriggerPath = pt->triggerIn;
      break;
    case 6:
      sprintf(path, "0://Bank%d/" TRIGGER(IN) "/X", Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(in_x);
      pTriggerPath = pt->triggerIn_X;
      break;
    case 7:
      sprintf(path, "0://Bank%d/" TRIGGER(IN) "/Y", Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(in_y);
      pTriggerPath = pt->triggerIn_Y;
      break;
    case 8:
      sprintf(path, "0://Bank%d/" TRIGGER(IN) "/Z", Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(in_z);
      pTriggerPath = pt->triggerIn_Z;
      break;

    case 9:
      sprintf(path, "0://Bank%d/" TRIGGER(OUT), Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(out);
      pTriggerPath = pt->triggerOut;
      break;
    case 10:
      sprintf(path, "0://Bank%d/" TRIGGER(OUT) "/X", Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(out_x);
      pTriggerPath = pt->triggerOut_X;
      break;
    case 11:
      sprintf(path, "0://Bank%d/" TRIGGER(OUT) "/Y", Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(out_y);
      pTriggerPath = pt->triggerOut_Y;
      break;
    case 12:
      sprintf(path, "0://Bank%d/" TRIGGER(OUT) "/Z", Bank);
      max_trigger_cnt = TRIGGER_MAX_NUM(out_z);
      pTriggerPath = pt->triggerOut_Z;
      break;
    }
    vPortFree(pTriggerPath->path_arry);
    vPortFree(pTriggerPath->path_ptr);
    pTriggerPath->number = 0;
    pTriggerPath->path_arry = NULL;
    pTriggerPath->path_ptr = NULL;
  }
  else
  {
    pTriggerPath->number = trigger_cnt;
    pTriggerPath->path_arry = (char *)pvPortMalloc(sizeof(char) * mem_req);
    pTriggerPath->path_ptr = (char **)pvPortMalloc(sizeof(char *) * trigger_cnt);

    log_i("malloc for %s cost %d", path, mem_req + sizeof(char *) * trigger_cnt);
  }
  trigger_cnt = 0;
  mem_req = 0;

  if ((f_err = f_opendir(&dir, path)) != FR_OK)
  {
    log_e("Open Bank%d Trigger:%d Error:%d", Bank, triggerid, f_err);
    MX_File_InfoLFN_DeInit(&info);
    return 1;
  }
  while ((f_err = f_readdir(&dir, &info)) == FR_OK && info.fname[0] != '\0')
  {
    char *pt = info.lfname[0] != 0 ? info.lfname : info.fname;

    if (info.fattrib & AM_DIR)
      continue;

    char buf[5];
    int pos = strlen(pt);
    if (pos < 4)
      continue;
    strncpy(buf, pt + pos - 4, 4);
    if (strncasecmp(buf, ".wav", 4))
      continue;
    // Trigger number limits
    if (trigger_cnt >= max_trigger_cnt)
      break;

    // 防止长文件名导致内存溢出，限制文件名长度为30
    int num = strlen(pt);
    if (num >= 30)
      continue;

    if (goto_flag == true)
    {
      pTriggerPath->path_ptr[trigger_cnt] = pTriggerPath->path_arry + mem_req;
      strcpy(pTriggerPath->path_ptr[trigger_cnt], pt);
    }
    trigger_cnt += 1;
    mem_req += num + 1;
  }
  f_closedir(&dir);

  if (goto_flag == false)
  {
    goto_flag = true;
    goto get_trigger_again;
  }

  MX_File_InfoLFN_DeInit(&info);
  return 0;
}

static uint8_t __get_accent_para(char Bank, char *filepath, SimpleLED_Acction_t **pt)
{
  FIL file;
  FRESULT f_err;
  uint8_t cnt = 0;
  char path[30];

  sprintf(path, "0:/Bank%d/Accent/%s", Bank, filepath);
  if ((f_err = f_open(&file, path, FA_READ)) != FR_OK)
  {
    if (f_err == FR_NO_FILE)
    {
      *pt = NULL;
      return 0;
    }
    else
    {
      log_v("Can't open accent file(%s):(%d)", path, (int)f_err);
      return 0;
    }
  }
  else
  {
    *pt = (SimpleLED_Acction_t *)pvPortMalloc(sizeof(SimpleLED_Acction_t));
    f_gets(path, 30, &file); // 第一行喂狗
    f_gets(path, 30, &file); // 读取间隔周期
    if (path[0] == 'T' || path[0] == 't')
    {
      int d;
      sscanf(path, "%*[^=]=%ld", &d);
      (*pt)->Delay = d;
      // sscanf(path, "%*[^=]=%ld", &((*pt)->Delay));
      while (f_gets(path, 30, &file) != 0)
      {
        if (path[0] >= '0' || path[1] <= '9')
          cnt++;
      }

      (*pt)->Action = (uint8_t *)pvPortMalloc(cnt);
      (*pt)->Num = cnt;

      f_lseek(&file, 0);
      f_gets(path, 30, &file);
      f_gets(path, 30, &file);

      uint8_t *act = (*pt)->Action;
      while (f_gets(path, 30, &file) != 0)
      {
        int buf;
        uint8_t ans = 0;
        sscanf(path, "%d", &buf);
        if (buf % 2)
          ans |= 0x01;
        buf /= 10;
        if (buf % 2)
          ans |= 0x02;
        buf /= 10;
        if (buf % 2)
          ans |= 0x04;
        buf /= 10;
        if (buf % 2)
          ans |= 0x08;
        buf /= 10;
        if (buf % 2)
          ans |= 0x10;
        buf /= 10;
        if (buf % 2)
          ans |= 0x20;
        buf /= 10;
        if (buf % 2)
          ans |= 0x40;
        buf /= 10;
        if (buf % 2)
          ans |= 0x80;

        *act++ = ans;
      }
      f_close(&file);
    }
    return 0;
  }
}
static uint8_t get_accent_para(uint8_t Bank, PARA_DYNAMIC_t *pt)
{
  __get_accent_para(Bank + 1, "Standby.txt", &((pt->accent + Bank)->Standby));
  __get_accent_para(Bank + 1, "On.txt", &((pt->accent + Bank)->On));
  __get_accent_para(Bank + 1, "Lockup.txt", &((pt->accent + Bank)->Lockup));
  __get_accent_para(Bank + 1, "Clash.txt", &((pt->accent + Bank)->Clash));
  return 0;
}

// char *upper(char *src)
// {
//   char *pt = src;
//   while (*pt)
//   {
//     if (*pt <= 'z' && *pt >= 'a')
//     {
//       *pt -= 'z' - 'Z';
//     }
//     pt += 1;
//   }
//   return src;
// }
// #ifdef __GNUC__
// int strncasecmp(const char *src1,const char *src2, size_t num)
// {
//   return strncmp(upper(src1), upper(src2), num);
// }
// #endif

static int GetName(char *line, char *name)
{
  int res = 0, cnt = 0;
  if (!strncasecmp(line, "BANK", 4))
  {
    sscanf(line, "%*[ BANKbank]%d", &res);
    return res * 3 + 0;
  }
  else if (!strncasecmp(line, "FBANK", 5))
  {
    sscanf(line, "%*[ FBANKfbank]%d", &res);
    return res * 3 + 1;
  }
  else if (!strncasecmp(line, "LBANK", 5))
  {
    sscanf(line, "%*[ LBANKlbank]%d", &res);
    return res * 3 + 2;
  }
  while (*line != '=')
  {
    if (*line == ' ' || *line == '\t')
    {
      line += 1;
    }
    if (*line == '\0')
    {
      *name = 0;
      return res;
    }
    if (cnt >= 100)
    {
      *name = 0;
      return res;
    }
    *name++ = *line++;
    cnt++;
  }
  *name = 0;
  return res;
}
static int GetMultiPara(char *line)
{
  int res = 0;
  char *pt = line;
  while (*pt != '=')
    pt += 1;
  pt += 1;
  while (*pt == ' ')
    pt += 1;
  while (*pt != '\n' && *pt != '/' && *pt != '\0')
  {
    if (*pt == '1')
      res |= 0x01;
    if (*pt == '2')
      res |= 0x02;
    if (*pt == '3')
      res |= 0x04;
    if (*pt == '4')
      res |= 0x08;
    pt += 1;
  }
  return res;
}
static void set_config(PARA_DYNAMIC_t *pt)
{
  memset(pt->config, 0, sizeof(USR_CONFIG_t));
  memset(pt->accent, 0, sizeof(Accent_t)*pt->nBank);
  pt->config->T_Breath = 2000; //LMode呼吸周期默认为2s
  pt->config->Out_Delay = 200; //Out 循环音延时200ms
  pt->config->SimpleLED_MASK = 0xFF;
  pt->config->DriverMode = 255;
  pt->config->Direction = 0;
  pt->config->ShakeOutG = 0;
  pt->config->ShakeInG = 0;
  pt->config->LockupHold = 0;
  pt->config->Lowpower = STATIC_USR.vol_warning;
  pt->config->PowerSavingPerrecnts = 0;

  pt->config->Vol = 3;
  pt->config->Tpon = 800;
  pt->config->Tpoff = 800;
  pt->config->Tout = 200;
  pt->config->Tin = 500;
  pt->config->Ts_switch = 500;
  pt->config->Tautoin = 60000;
  pt->config->Tautooff = 120000;
  pt->config->Tmute = 500;
  pt->config->TLcolor = 200;
  pt->config->TBfreeze = 200;
  pt->config->TBMode = 1;
  pt->config->TCfreeze = 150;
  pt->config->TCMode = 0x06;
  pt->config->TDfreeze = 150;
  pt->config->TDMode = 0x06;
  pt->config->TEtrigger = 600;
  pt->config->TEMode = 0x08;
  pt->config->TLon = 2000;
  pt->config->TLoff = 700;
  pt->config->Lbright = 1023;
  pt->config->Ldeep = 768;
  pt->config->LMode = 2;
  pt->config->T_Breath = 1000;
  pt->config->MD = 30;
  pt->config->MT = 2;
  pt->config->CD = 4;
  pt->config->CT = 30;
  pt->config->CL = 30;
  pt->config->CW = 0;

  if (USR.nBank > 0)
  {
    pt->BankColor[0] = (1023) << 16 + 1023;
    pt->BankColor[1] = 0;

    pt->BankColor[2] = 0;
    pt->BankColor[3] = (1023) << 16 + 1023;
  }
}
static uint8_t get_config(PARA_DYNAMIC_t *pt, FIL *file)
{
  char sbuffer[CONST_KEY_LEN + 30];
  char name[CONST_KEY_LEN];
  char *spt;
  bool *bank_color_bool = (bool *)malloc(sizeof(bool) * USR.nBank);
  bool *fbank_color_bool = (bool *)malloc(sizeof(bool) * USR.nBank);
  bool *lbank_color_bool = (bool *)malloc(sizeof(bool) * USR.nBank);

  for (int i = 0; i < USR.nBank; i++)
  {
    bank_color_bool[i] = false;
    fbank_color_bool[i] = false;
    lbank_color_bool[i] = false;
  }

  while ((spt = f_gets(sbuffer, 50, file)) != 0)
  {
    int8_t res = GetName(spt, name);
    if (res > 0 && res % 3 == 0)
    {
      res = res / 3;
      uint16_t buf[4];
      if (res > USR.nBank)
        continue;
      sscanf(spt, "%*[^=]=%hd,%hd,%hd,%hd", buf, buf + 1, buf + 2, buf + 3);
      pt->BankColor[res * 6 + 0 - 6] = *(uint32_t *)(buf + 0);
      pt->BankColor[res * 6 + 1 - 6] = *(uint32_t *)(buf + 2);
      bank_color_bool[res] = true;
      continue;
    }
    else if (res > 0 && res % 3 == 1)
    {
      res = (res - 1) / 3;
      uint16_t buf[4];
      if (res > USR.nBank)
        continue;
      sscanf(spt, "%*[^=]=%hd,%hd,%hd,%hd", buf, buf + 1, buf + 2, buf + 3);
      pt->BankColor[res * 6 + 2 - 6] = *(uint32_t *)(buf + 0);
      pt->BankColor[res * 6 + 3 - 6] = *(uint32_t *)(buf + 2);
      fbank_color_bool[res] = true;
      continue;
    }
    else if (res > 0 && res % 3 == 2)
    {
      res = (res - 2) / 3;
      uint16_t buf[4];
      if (res > USR.nBank)
        continue;
      sscanf(spt, "%*[^=]=%hd,%hd,%hd,%hd", buf, buf + 1, buf + 2, buf + 3);
      pt->BankColor[res * 6 + 4 - 6] = *(uint32_t *)(buf + 0);
      pt->BankColor[res * 6 + 5 - 6] = *(uint32_t *)(buf + 2);
      lbank_color_bool[res] = true;
      continue;
    }
    for (res = 0; res < sizeof(name_string) / CONST_KEY_LEN; res++)
    {
      if (!strcasecmp(name, name_string[res]))
        break;
    }
    switch (res)
    {
    case 0:
      sscanf(spt, "%*[^=]=%d", &(pt->config->Vol));
      break;
    case 1:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Tpon));
      break;
    case 2:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Tpoff));
      break;
    case 3:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Tout));
      break;
    case 4:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Tin));
      break;
    case 5:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Ts_switch));
      break;
    case 6:
      sscanf(spt, "%*[^=]=%ld", &(pt->config->Tautoin));
      break;
    case 7:
      sscanf(spt, "%*[^=]=%ld", &(pt->config->Tautooff));
      break;
    case 8:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Tmute));
      break;
    case 9:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->TLcolor));
      break;
    case 10:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->TBfreeze));
      break;
    case 11:
      pt->config->TBMode = GetMultiPara(spt);
      break;
    case 12:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->TCfreeze));
      break;
    case 13:
      pt->config->TCMode = GetMultiPara(spt);
      break;
    case 14:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->TDfreeze));
      break;
    case 15:
      pt->config->TDMode = GetMultiPara(spt);
      break;
    case 16:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->TEtrigger));
      break;
    case 17:
      pt->config->TEMode = GetMultiPara(spt);
      break;
    case 18:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->TLon));
      break;
    case 19:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->TLoff));
      break;
    case 20:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Lbright));
      break;
    case 21:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Ldeep));
      break;
    case 22:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->LMode));
      break;
    case 23:
      sscanf(spt, "%*[^=]=%hd", (pt->config->ChDelay));
      break;
    case 24:
      sscanf(spt, "%*[^=]=%hd", (pt->config->ChDelay + 1));
      break;
    case 25:
      sscanf(spt, "%*[^=]=%hd", (pt->config->ChDelay + 2));
      break;
    case 26:
      sscanf(spt, "%*[^=]=%hd", (pt->config->ChDelay + 3));
      break;
    case 27:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->T_Breath));
      break;
    case 28:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Out_Delay));
      break;
    case 29:
      sscanf(spt, "%*[^=]=%*[^xX]%*c%x", &(pt->config->SimpleLED_MASK));
      break;
    case 30:
      sscanf(spt, "%*[^=]=%d", &(pt->config->DriverMode));
      break;
    case 31:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->MD));
      break;
    case 32:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->MT));
      break;
    case 33:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->CD));
      break;
    case 34:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->CT));
      break;
    case 35:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->CL));
      break;
    case 36:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->CW));
      break;
    case 37:
      sscanf(spt, "%*[^=]=%d", &(pt->config->Direction));
      break;
    case 38:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->ShakeOutG));
      break;
    case 39:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->ShakeInG));
      break;
    case 40:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->LockupHold));
      break;
    case 41:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->Lowpower));
      break;
    case 42:
      sscanf(spt, "%*[^=]=%hd", &(pt->config->PowerSavingPerrecnts));
      break;
    case 43:
      sscanf(spt, "%*[^=]=%hd", &(pt->filelimits.bank_max));
      break;
    }
  }

  for (int i = 1; i < USR.nBank; i++)
  {
    if (bank_color_bool[i] == false)
    {
      pt->BankColor[i * 6 + 0 - 6] = (pt->BankColor[0]);
      pt->BankColor[i * 6 + 1 - 6] = (pt->BankColor[1]);
    }
    if (fbank_color_bool[i] == false)
    {
      pt->BankColor[i * 6 + 2 - 6] = (pt->BankColor[2]);
      pt->BankColor[i * 6 + 3 - 6] = (pt->BankColor[3]);
    }
    if (lbank_color_bool[i] == false)
    {
      pt->BankColor[i * 6 + 4 - 6] = (pt->BankColor[4]);
      pt->BankColor[i * 6 + 5 - 6] = (pt->BankColor[5]);
    }
  }

  free(bank_color_bool);
  free(fbank_color_bool);
  free(lbank_color_bool);

  return 0;
}
