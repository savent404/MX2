#ifndef __LEDNP_HAL_H_
#define __LEDNP_HAL_H_

#define SPI_DATAMODE 0
#define PWM_DATAMODE 1

typedef enum {
    NP_OpSuccess = 0,
    NP_ParaError,
    NP_OpFail,
    
}_eNPStatus;

typedef enum {
    NPIF_WaitInit = 0,
    NPIF_Idle,
    NPIF_Busy,
    NPIF_Error,
}_eNPIFStatus;

enum {
    NP_R=0,
    NP_G,
    NP_B,
};

typedef struct {
    void (* NpHwLL_Init)(unsigned short);
    void (* NpHwLL_Start)(unsigned char *, unsigned int);
    void (* NpHwLL_Stop)(void);
} _sNpDrv, *_pNpDrv;

typedef struct {
    unsigned char NpRGBOrder[3];//r:0, g:1, b:2
    unsigned char TimerClkFreq_Mhz;
    unsigned short NpNumber;
    unsigned short NpPeriod_Ns;
    unsigned short NpV0HighWitdh_Ns;
    unsigned short NpV1HighWitdh_Ns;
    unsigned short NpRstnWidth_Ns;
    unsigned short DataBufferSize;
    unsigned char *DataBuffer;    
} _sNpHwConfig, *_pNpHwConfig;

typedef struct {
    unsigned char NpRGBOrder[3];//r:0, g:1, b:2
    unsigned short NpNumber;
    unsigned short NpPeriod_Value;
    unsigned short NpV0_Value;
    unsigned short NpV1_Value;
    unsigned short NpRstHighPulse_PeriodNum;
    unsigned short NpRstLowPulse_PeriodNum;
    unsigned short DmaDataBufferSize;
    unsigned char *RGBDataBuffer;    
    unsigned char *DmaDataBuffer;    
    _eNPIFStatus NpIFStatus;
    _pNpDrv pNpDrv;
} _sNpParaConfig, *_pNpParaConfig;


#if PWM_DATAMODE
#define NP_RST_H_PERIODNUM 1
#define NP_RST_L_PERIODNUM_MAGIN 4
#else
    
#endif

_eNPStatus NP_DataIF_Init(_pNpHwConfig pNpHwConfig, _pNpParaConfig pNpParaConfig, _pNpDrv pNpDrv);
_eNPStatus NP_DataIF_EmgStop();
_eNPStatus NP_DataIF_Refresh(_pNpParaConfig NpParaConfig);
_eNPStatus NP_DataIF_GetStatus();
_eNPStatus NP_DataIF_DMAHalfIntHandler();
_eNPStatus NP_DataIF_DMAFullIntHandler();


#endif