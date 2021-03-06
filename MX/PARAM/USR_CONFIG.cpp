#include "FreeRTOS.h"
#include "debug.h"
#include "ff.h"
#include "param.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <strings.h>
#include "AF.h"
#include "fatfs_ultis.h"
#include "path.h"

PARA_DYNAMIC_t USR;

const PARA_STATIC_t STATIC_USR = {
    .vol_warning        = 3300,
    .vol_poweroff       = 3000,
    .vol_chargecomplete = 4150,
    .filelimits         = {
        .bank_max         = 99,
        .trigger_OUT_max  = 16,
        .trigger_IN_max   = 16,
        .trigger_B_max    = 16,
        .trigger_C_max    = 16,
        .trigger_D_max    = 16,
        .trigger_E_max    = 16,
        .trigger_STAB_max = 16,
        .trigger_HUM_max  = 16,
    }
};

static const char name_string[][ 20 ] = {
    /**< Position:0~5  */
    "Vol",
    "Tpon",
    "Tpoff",
    "Tout",
    "Tin",
    "Ts_switch",
    /**< Position:6~11 */
    "Tautoin",
    "Tautooff",
    "Tmute",
    "TLcolor",
    "TBfreeze",
    "TBMode",
    /**< Position:12~17 */
    "TCfreeze",
    "TCMode",
    "TDfreeze",
    "TDMode",
    "TEtrigger",
    "TEMode",
    /**< Position:18~23 */
    "TLon",
    "TLoff",
    "Lbright",
    "Ldeep",
    "LMode",
    "CH1_Delay",
    /**< Position:24~29 */
    "CH2_Delay",
    "CH3_Delay",
    "CH4_Delay",
    "T_Breath",
    "Out_Delay",
    "LEDMASK",
    /**< Position:30~35 */
    "Unknow",
    "MD",
    "MT",
    "CD",
    "CT",
    "CL",
    /**< Position:36~40*/
    "CW",
    "null",
    "null",
    "null",
    "GB",
    /**< Position:41~45*/
    "ST",
    "SPL1",
    "SPL2",
    "SPL3",
    "SPL4",

    /**< Position:46~50*/
    "SwingThreshold_H",
    "SwingThreshold_L",
    "SwingAdjN",
    "TFfreeze",
    "StabThreshold",
    /**< Position:51*/
    "StabWindow",
};
static const char name_colorswitch[][ 10 ] = {
    "MC",
    "SC",
    "TC",
};

/** \brief 获取循环音频有效负载量
 *
 * \param 动态配置信息结构体指针
 * \return Error Code
 *
 */
static uint8_t get_humsize(PARA_DYNAMIC_t* pt);

/** \brief 设定一些默认配置参数
 *
 * \param pt 用户动态配置参数，在动态获取之前需设定一些默认参数
 * \return None
 *
 */
static void set_config(PARA_DYNAMIC_t* pt);

/** \brief 得到用户配置文件(config.txt)中的信息
 *
 * \param pt 用户动态配置参数，但初始化后不会更改
 * \return Error Code
 *
 */
static uint8_t get_config(PARA_DYNAMIC_t* pt, FIL* file);

// get_config的变种，只获取新的颜色索引
static uint8_t get_colorSwitch_config(PARA_DYNAMIC_t* pt, FIL* file);

/** \brief 得到Accent.txt中的信息
 *
 * \param Bank 需要扫描的Bank
 * \param pt 用户动态配置参数，但初始化后不会更改
 * \return Error Code
 *
 */
static uint8_t get_accent_para(uint8_t Bank, PARA_DYNAMIC_t* pt);

/** \brief Initialize User's Configuration Parameter
 *
 * \param  None
 * \return Error Code
 *
 */
uint8_t usr_config_init(void)
{
    FRESULT f_err;
    FIL*    file = (FIL*)pvPortMalloc(sizeof(FIL));
    char    path[ 32 ];
    /**< 获取Bank数量 */
    {
        DIR      dir;
        FILINFO* info = fatfs_allocFileInfo();
        /**< Open Dir:[0:/] */
        if ((f_err = f_opendir(&dir, "0:/")) != FR_OK) {
            DEBUG(0, "Can't Open Dir(0:/):%d", f_err);
            fatfs_freeFileInfo(info);
            return 1;
        }
        /**< Get Number of Banks */
        USR.nBank = 0;
        while ((f_readdir(&dir, info) == FR_OK) && info->fname[ 0 ] != '\0') {
            if ((info->fattrib & AM_DIR) && (info->fname[ 0 ] == 'B' || info->fname[ 0 ] == 'b'))
                USR.nBank += 1;
        }
        /**< Close Dir */
        if ((f_err = f_closedir(&dir)) != FR_OK) {
            DEBUG(0, "Can't Close Dir:%d", f_err);
            fatfs_freeFileInfo(info);
            return 1;
        }
        fatfs_freeFileInfo(info);
        /**< Bank number limits */
        if (STATIC_USR.filelimits.bank_max < USR.nBank && STATIC_USR.filelimits.bank_max) {
            USR.nBank = STATIC_USR.filelimits.bank_max;
        }
    }
    USR.humsize   = (HumSize_t*)pvPortMalloc(sizeof(HumSize_t) * USR.nBank);
    USR._config   = (USR_CONFIG_t*)pvPortMalloc(sizeof(USR_CONFIG_t) * 3);
    USR.BankColor = (uint32_t*)pvPortMalloc(sizeof(uint32_t) * 4 * USR.nBank);
    /**
   * We need storage CONFIG.txt data now,
   * USE USR.config Init this
   */
    USR.config = &(USR.backUpConfig);

    /**< 获取CONFIG.txt中的信息,包含各种配置信息以及功率LED灯的颜色信息*/
    if ((f_err = f_open(file, PATH_CONFIG, FA_READ)) != FR_OK) {
        DEBUG(0, "Can't find %s:%d", PATH_CONFIG, f_err);
        return 1;
    }
    set_config(&USR);
    f_err = (FRESULT)get_config(&USR, file);
    if (f_err)
        return f_err;
    f_close(file);

    /**< 获取硬件配置参数 */
    sprintf(path, "%s/" PATH_HWCONFIG, MX_PARAM_GetPrefix());
    USR.hwParam = triggerSets_readHW(path);

    /**< 获取每个bank中的Accent.txt, 包括动作延时时间以及动作信息 */
    USR.accent = (Accent_t*)pvPortMalloc(sizeof(Accent_t) * USR.nBank);
    for (uint8_t nBank = 0; nBank < USR.nBank; nBank++) {
        f_err = (FRESULT)get_accent_para(nBank, &USR);
        if (f_err)
            return f_err;
    }

    if (1) {
        MX_ColorMatrix_Update("0:/" PATH_COLORMATRIX, &USR.colorMatrix);
        USR.colorMatrix.bankNum    = USR.nBank;
        USR.colorMatrix.colorIndex = (int*)pvPortMalloc(sizeof(int) * USR.nBank);
        for (int i = 0; i < USR.nBank; i++) {
            USR.colorMatrix.colorIndex[ i ] = 0;
        }
    }

    // usr_config_init(0, 1);
    usr_init_bank(0, 1);

    ///以上任意一个错误都是致命的
    ///所以不再对动态分配的内存进行回收，只有在正确返回的情况下考虑回收内存
    vPortFree(file);
    return 0;
}

#ifdef __STRICT_ANSI__
int strcasecmp(const char* src1, const char* src2)
{
    char *pt1 = (char*)pvPortMalloc(strlen(src1) * sizeof(char) + 1),
         *pt2 = (char*)pvPortMalloc(strlen(src1) * sizeof(char) + 1);
    strcpy(pt1, src1);
    strcpy(pt2, src2);
    int res = strcasecmp(pt1, pt2);
    vPortFree(pt1);
    vPortFree(pt2);
    return res;
}
#endif

static uint8_t __get_accent_para(char Bank, char* filepath, SimpleLED_Acction_t** pt)
{
    FIL     file;
    FRESULT f_err;
    uint8_t cnt = 0;
    char    path[ 30 ];

    sprintf(path, "0:/Bank%d/Accent/%s", Bank, filepath);
    if ((f_err = f_open(&file, path, FA_READ)) != FR_OK) {
        if (f_err == FR_NO_FILE) {
            *pt = NULL;
            return 0;
        } else {
            DEBUG(4, "Open File(%s) Error:%d", path, (int)f_err);
            return 0;
        }
    } else {
        *pt = (SimpleLED_Acction_t*)pvPortMalloc(sizeof(SimpleLED_Acction_t));
        f_gets(path, 30, &file); // 第一行喂狗
        f_gets(path, 30, &file); // 读取间隔周期
        if (path[ 0 ] == 'T' || path[ 0 ] == 't') {
            int d;
            sscanf(path, "%*[^=]=%ld", &d);
            (*pt)->Delay = d;
            // sscanf(path, "%*[^=]=%ld", &((*pt)->Delay));
            while (f_gets(path, 30, &file) != 0) {
                if (path[ 0 ] >= '0' || path[ 1 ] <= '9')
                    cnt++;
            }

            (*pt)->Action = (uint8_t*)pvPortMalloc(cnt);
            (*pt)->Num    = cnt;

            f_lseek(&file, 0);
            f_gets(path, 30, &file);
            f_gets(path, 30, &file);

            uint8_t* act = (*pt)->Action;
            while (f_gets(path, 30, &file) != 0) {
                int     buf;
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
static uint8_t get_accent_para(uint8_t Bank, PARA_DYNAMIC_t* pt)
{
    __get_accent_para(Bank + 1, "Standby.txt", &((pt->accent + Bank)->Standby));
    __get_accent_para(Bank + 1, "On.txt", &((pt->accent + Bank)->On));
    __get_accent_para(Bank + 1, "Lockup.txt", &((pt->accent + Bank)->Lockup));
    __get_accent_para(Bank + 1, "Clash.txt", &((pt->accent + Bank)->Clash));
    return 0;
}

static int GetName(char* line, char* name)
{
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
        }
        if (*line == '\0') {
            *name = 0;
            return res;
        }
        if (cnt >= 100) {
            *name = 0;
            return res;
        }
        *name++ = *line++;
        cnt++;
    }
    *name = 0;
    return res;
}
static int GetMultiPara(char* line)
{
    int   res = 0;
    char* pt  = line;
    while (*pt != '=')
        pt += 1;
    pt += 1;
    while (*pt == ' ')
        pt += 1;
    while (*pt != '\n' && *pt != '/' && *pt != '\0') {
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
static void set_config(PARA_DYNAMIC_t* pt)
{
    pt->config->SwingThreshold_L = 50;
    pt->config->SwingThreshold_H = 360;
    pt->config->SwingAdjN        = 2;
    pt->config->T_Breath         = 2000; //LMode呼吸周期默认为2s
    pt->config->Out_Delay        = 200; //Out 循环音延时200ms
    pt->config->SimpleLED_MASK   = 0xFF;
    pt->config->GB               = 0x80;
    pt->config->MD               = 0x60;
    pt->config->MT               = 0x3F;
    pt->config->CD               = 0x03;
    pt->config->CT               = 0x1F;
    pt->config->CL               = 0x00;
    pt->config->CW               = 0x00;
    pt->config->ST               = 200;
    pt->config->SPL1             = 100;
    pt->config->SPL2             = 200;
    pt->config->SPL3             = 300;
    pt->config->SPL4             = 400;
    pt->config->TFfreeze         = 200;
    pt->config->StabThreshold    = 2000,
    pt->config->StabWindow       = 3;
}
static uint8_t get_config(PARA_DYNAMIC_t* pt, FIL* file)
{
    char  sbuffer[ 50 ];
    char  name[ 20 ];
    char* spt;
    while ((spt = f_gets(sbuffer, 50, file)) != 0) {
        int8_t res = GetName(spt, name);
        if (res > 0) {
            uint16_t buf[ 4 ];
            if (res > USR.nBank)
                continue;
            sscanf(spt, "%*[^=]=%hd,%hd,%hd,%hd", buf, buf + 1, buf + 2, buf + 3);
            pt->BankColor[ res * 4 + 0 - 4 ] = *(uint32_t*)(buf + 0);
            pt->BankColor[ res * 4 + 1 - 4 ] = *(uint32_t*)(buf + 2);
            continue;
        } else if (res < 0) {
            uint16_t buf[ 4 ];
            if (-res > USR.nBank)
                continue;
            sscanf(spt, "%*[^=]=%hd,%hd,%hd,%hd", buf, buf + 1, buf + 2, buf + 3);
            pt->BankColor[ -1 * res * 4 + 2 - 4 ] = *(uint32_t*)(buf + 0);
            pt->BankColor[ -1 * res * 4 + 3 - 4 ] = *(uint32_t*)(buf + 2);
            continue;
        }
        for (res = 0; res < sizeof(name_string) / 10; res++) {
            if (!strcasecmp(name, name_string[ res ]))
                break;
        }
        switch (res) {
        case 0:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->Vol));
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
            sscanf(spt, "%*[^=]=%hd", &(pt->config->Tautoin));
            break;
        case 7:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->Tautooff));
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
        /*case 30: sscanf(spt,"%*[^=]=%d", &(pt->config->Ch));break;*/
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
        /*
      case 37: sscanf(spt,"%*[^=]=%d", &(pt->config->MCIndex));break;
      case 38: sscanf(spt,"%*[^=]=%d", &(pt->config->SCIndex));break;
      case 39: sscanf(spt,"%*[^=]=%d", &(pt->config->TCIndex));break;
      */
        case 40:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->GB));
            break;
        case 41:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->ST));
            break;
        case 42:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->SPL1));
            break;
        case 43:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->SPL2));
            break;
        case 44:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->SPL3));
            break;
        case 45:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->SPL4));
            break;
        case 46:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->SwingThreshold_H));
            break;
        case 47:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->SwingThreshold_L));
            break;
        case 48:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->SwingAdjN));
            break;
        case 49:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->TFfreeze));
            break;
        case 50:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->StabThreshold));
            break;
        case 51:
            sscanf(spt, "%*[^=]=%hd", &(pt->config->StabWindow));
            break;
        }
    }
    return 0;
}

/**
 * @brief  切换bank时更新各项usr参数
 * @note   包括第一次参数的初始化，所以需要注意某些指针的默认值应当为NULL
 * @param  dest 目的bank
 * @param  freshFlag != 0 强制初始化前后两个bank,否则采取尽可能复制的方法
 */
uint8_t usr_switch_bank(int dest)
{
    if (usr_init_bank(dest, 1))
        return 1;

    USR.config   = USR._config + 1;
    USR.bank_now = dest;

    usr_update_triggerPah(dest);

    return 0;
}

/**
 * @brief 初始化某个bank的特定参数
 * @warning 某些初始化函数使用了USR.config,需注意线程安全
 * @param bankPos 读取的Bank Pos
 * @param storagePos 保持位置[0..2] 代表 前-现在-后
 * @return !0 即有错
 */
uint8_t usr_init_bank(int bankPos, int storagePos)
{
    // fatfs stuff
    FIL     file;
    FRESULT f_err;

    // string buffer, used in file path almostly
    char strBuffer[ 128 ];

    // storagePos < 0, storagePos would be set as 1
    if (storagePos < 0)
        storagePos = 1;

    // Range check, storagePos should be [0, 2]
    if (storagePos > 2)
        return 1;

    // set BANK config ptr
    USR.config        = USR._config + storagePos;
    USR.np_colorIndex = USR.colorMatrix.colorIndex[ bankPos ];

    // Load Parameter default and setted in CONFIG.txt
    // Then read from EFFECT.txt in specific dir
    sprintf(strBuffer, "0:/Bank%d/" REPLACE_NAME, bankPos + 1);
    memcpy(USR.config, &(USR.backUpConfig), sizeof(USR_CONFIG_t));
    f_err = f_open(&file, strBuffer, FA_READ);
    if (f_err)
        return f_err;
    set_config(&USR);
    f_err = (FRESULT)get_config(&USR, &file);
    if (f_err) {
        f_close(&file);
        return f_err;
    }
    f_close(&file);
    return 0;
}

#define DEINIT_PATH(name)            \
    if (name) {                      \
        MX_TriggerPath_DeInit(name); \
        name = NULL;                 \
    }

#define UPDATE_COLOR(name, path, bankPos)                                           \
    do {                                                                            \
        TRIGGERPATH_Type_t bg = TRIGGERPATH_BG;                                     \
        TRIGGERPATH_Type_t tg = TRIGGERPATH_TG;                                     \
        TRIGGERPATH_Type_t ft = TRIGGERPATH_FT;                                     \
        sprintf(path, "0:/Bank%d/%s", bankPos + 1, TRIGGER(name));                  \
        DEINIT_PATH(USR.triggerBG##name);                                           \
        DEINIT_PATH(USR.triggerTG##name);                                           \
        DEINIT_PATH(USR.triggerFT##name);                                           \
        USR.triggerBG##name = MX_TriggerPath_Init(path, TRIGGER_MAX_NUM(name), bg); \
        USR.triggerTG##name = MX_TriggerPath_Init(path, TRIGGER_MAX_NUM(name), tg); \
        USR.triggerFT##name = MX_TriggerPath_Init(path, TRIGGER_MAX_NUM(name), ft); \
    } while (0);

#define UPDATE_TRIGGER(name, path, bankPos, waveType)                                \
    do {                                                                             \
        TRIGGERPATH_Type_t wav = waveType;                                           \
        TRIGGERPATH_Type_t bg  = TRIGGERPATH_BG;                                     \
        TRIGGERPATH_Type_t tg  = TRIGGERPATH_TG;                                     \
        TRIGGERPATH_Type_t ft  = TRIGGERPATH_FT;                                     \
        sprintf(path, "0:/Bank%d/%s", bankPos + 1, TRIGGER(name));                   \
        DEINIT_PATH(USR.trigger##name);                                              \
        DEINIT_PATH(USR.triggerBG##name);                                            \
        DEINIT_PATH(USR.triggerTG##name);                                            \
        DEINIT_PATH(USR.triggerFT##name);                                            \
        USR.trigger##name   = MX_TriggerPath_Init(path, TRIGGER_MAX_NUM(name), wav); \
        USR.triggerBG##name = MX_TriggerPath_Init(path, TRIGGER_MAX_NUM(name), bg);  \
        USR.triggerTG##name = MX_TriggerPath_Init(path, TRIGGER_MAX_NUM(name), tg);  \
        USR.triggerFT##name = MX_TriggerPath_Init(path, TRIGGER_MAX_NUM(name), ft);  \
    } while (0);

uint8_t usr_update_triggerPah(int bankPos)
{
    char path[ 64 ];
    UPDATE_TRIGGER(HUM, path, bankPos, TRIGGERPATH_WAV);
    UPDATE_TRIGGER(B, path, bankPos, TRIGGERPATH_PAIRWAV);
    UPDATE_TRIGGER(C, path, bankPos, TRIGGERPATH_WAV);
    UPDATE_TRIGGER(D, path, bankPos, TRIGGERPATH_WAV);
    UPDATE_TRIGGER(E, path, bankPos, TRIGGERPATH_WAV);
    UPDATE_TRIGGER(IN, path, bankPos, TRIGGERPATH_WAV);
    UPDATE_TRIGGER(OUT, path, bankPos, TRIGGERPATH_WAV);
    UPDATE_TRIGGER(STAB, path, bankPos, TRIGGERPATH_WAV);
    return 0;
}
