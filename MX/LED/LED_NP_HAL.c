#include "LED_NP.h"

extern unsigned char DmaCpltFlag;

static void NpDataBuffer_Init(_pNpParaConfig pNpParaConfig);

_eNPStatus NP_DataIF_Init(_pNpHwConfig pNpHwConfig, _pNpParaConfig pNpParaConfig, _pNpDrv pNpDrv)
{
    unsigned short temp16;
    
    //parse parameters
    unsigned int RequiredBufferSize;
    pNpParaConfig->NpNumber = pNpHwConfig->NpNumber;
    pNpParaConfig->NpRstHighPulse_PeriodNum = NP_RST_H_PERIODNUM;
    pNpParaConfig->NpRstLowPulse_PeriodNum = pNpHwConfig->NpRstnWidth_Ns/pNpHwConfig->NpPeriod_Ns + NP_RST_L_PERIODNUM_MAGIN;
    
    RequiredBufferSize = (pNpParaConfig->NpNumber * 3 * 8 + pNpParaConfig->NpRstHighPulse_PeriodNum + pNpParaConfig->NpRstLowPulse_PeriodNum);

    if(RequiredBufferSize > pNpHwConfig->DataBufferSize) {
        return NP_ParaError;
    }
    
    pNpParaConfig->DmaDataBuffer = pNpHwConfig->DataBuffer;
    pNpParaConfig->DmaDataBufferSize = RequiredBufferSize;
    pNpParaConfig->NpRGBOrder[0] = pNpHwConfig->NpRGBOrder[0];
    pNpParaConfig->NpRGBOrder[1] = pNpHwConfig->NpRGBOrder[1];
    pNpParaConfig->NpRGBOrder[2] = pNpHwConfig->NpRGBOrder[2];    
    
    temp16=pNpHwConfig->NpV0HighWitdh_Ns*pNpHwConfig->TimerClkFreq_Mhz/1000;
    if(pNpHwConfig->NpV0HighWitdh_Ns*pNpHwConfig->TimerClkFreq_Mhz%1000) {
        temp16++;
    }
    
    pNpParaConfig->NpPeriod_Value = pNpHwConfig->TimerClkFreq_Mhz*pNpHwConfig->NpPeriod_Ns/1000;
    pNpParaConfig->NpV0_Value = temp16;
    pNpParaConfig->NpV1_Value = pNpParaConfig->NpPeriod_Value - pNpParaConfig->NpV0_Value;

    //buffer initial
    NpDataBuffer_Init(pNpParaConfig);
    

    //driver initial
    pNpParaConfig->pNpDrv = pNpDrv;
    pNpParaConfig->pNpDrv->NpHwLL_Init = Np_Tim_Init;
    pNpParaConfig->pNpDrv->NpHwLL_Start = Np_Tim_Start;
    pNpParaConfig->pNpDrv->NpHwLL_Stop = Np_Tim_Stop;


    pNpParaConfig->pNpDrv->NpHwLL_Init(pNpParaConfig->NpPeriod_Value);
    
    pNpParaConfig->NpIFStatus = NPIF_Idle;

    return NP_OpSuccess;
}

_eNPStatus NP_DataIF_Refresh(_pNpParaConfig pNpParaConfig)
{    
    pNpParaConfig->NpIFStatus = NPIF_Busy;
    pNpParaConfig->pNpDrv->NpHwLL_Start(pNpParaConfig->DmaDataBuffer, pNpParaConfig->DmaDataBufferSize);
    while(!DmaCpltFlag);
    DmaCpltFlag=0;
    pNpParaConfig->pNpDrv->NpHwLL_Stop();
    pNpParaConfig->NpIFStatus = NPIF_Idle;
    return NP_OpSuccess;
}

static void NpDataBuffer_Init(_pNpParaConfig pNpParaConfig)
{
    unsigned int i;
    unsigned int loopnum;
    
    unsigned int Value0 = (unsigned int)pNpParaConfig->NpV0_Value<<24 | (unsigned int)pNpParaConfig->NpV0_Value<<16 | \
                          (unsigned int)pNpParaConfig->NpV0_Value<<8  | (unsigned int)pNpParaConfig->NpV0_Value;
    unsigned char NpPerido = pNpParaConfig->NpPeriod_Value;
    
    unsigned char *ptr;
    
    ptr = pNpParaConfig->DmaDataBuffer;
    
    //fill all buffer with NP date R 0x00, G 0x00, B 0x00;
    loopnum = pNpParaConfig->NpNumber * 3 * 8 / 4;
    for(i=0;i<loopnum;i++) {
        *(unsigned int *)(ptr) = Value0;
        ptr+=4;
        //(pNpParaConfig->DataBuffer)[i+pNpParaConfig->DataBufferSize]=pNpParaConfig->NpV0_Value;
    }
    //fill high data before reset symbol
    loopnum += pNpParaConfig->NpRstHighPulse_PeriodNum;
    for(;i<loopnum;i++) {
        *ptr++=NpPerido;
        //(pNpParaConfig->DataBuffer)[i+pNpParaConfig->DataBufferSize]=pNpParaConfig->NpPeriod_Value;
    }
    //fill low data for reset symbol
    loopnum += pNpParaConfig->NpRstLowPulse_PeriodNum;
    for(;i<loopnum;i++) {
        *ptr++=0x00;
        //(pNpParaConfig->DataBuffer)[i+pNpParaConfig->DataBufferSize]=0x00;
    }    
}


