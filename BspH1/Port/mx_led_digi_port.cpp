#include "stdbool.h"
#include "stdint.h"

#include "main.h"
#include "tim.h"
#include "cmsis_os.h"

#include "MX_LEDDigitCfg.h"

#include "color.hpp"
#include "LED_NP.h"

//Red: 0x00, Green: 0x01, Blue: 0x02
//Use a byte to describe the color
//Bit(5,4): the first color, Bit(3,2): the middle color, Bit(1,0) the last color
#define NP_RGB  ((0 << 4) | (1 << 2) | (2)) // 0x06
#define NP_RBG  ((0 << 4) | (2 << 2) | (1)) // 0x09
#define NP_GRB  ((1 << 4) | (0 << 2) | (2)) // 0x52
#define NP_GBR  ((2 << 4) | (0 << 2) | (1)) // 0xA1
#define NP_BRG  ((1 << 4) | (2 << 2) | (0)) // 0x58
#define NP_BGR  ((2 << 4) | (1 << 2) | (0)) // 0xA4


typedef struct {
    uint8_t NpRGBOrder;
    uint16_t NpNumber;
    uint16_t NpPeriod_Ns;
    uint16_t NpV0HighWitdh_Ns;
    uint16_t NpV1HighWitdh_Ns;
    uint16_t TimerClkFreq_Mhz;
    uint32_t NpRstnWidth_Ns; 
} NpHwConfig_s, *pNpHwConfig_s;

typedef struct {
    uint8_t NpRGBOrder;
    uint16_t NpNumber;
    uint16_t NpPeriod_Value;
    uint16_t NpV0_Value;
    uint16_t NpV1_Value;
    uint16_t NpRstPeriod_Num;
    uint16_t DmaDataBufferSize;
} NpParaConfig_s, *pNpParaConfig_s;

extern TIM_HandleTypeDef htim4;
extern DMA_HandleTypeDef hdma_tim4_ch2;

const NpHwConfig_s NpHwConfig = {
    .NpRGBOrder=NP_GRB,
    .NpNumber=50,
    .NpPeriod_Ns=1250,
    .NpV0HighWitdh_Ns=320,
    .NpV1HighWitdh_Ns=640,
    .TimerClkFreq_Mhz=120,
    .NpRstnWidth_Ns=80000,
};


extern osSemaphoreId NpOperate_Cplt_FlagHandle;

static uint8_t LEDDigitBuffer[MX_LEDDIGI_MAXNUM*MX_LEDDIGI_COLORNUM*MX_LEDDIGI_COLORWIDTH];
static NpParaConfig_s NpParaConfig;

static void LED_NP_TIM_Init(void);
static void LED_NP_Buffer_Init(pNpParaConfig_s pNpParaConfig);
static void LED_NP_Para_Decode(pNpHwConfig_s pNpHwConfig, pNpParaConfig_s pNpParaConfig);
__INLINE static void Pixel2TimData(uint8_t * rgb, uint8_t * buffer);
static void NpData_Refresh(const RGB* arg, int num);
static HAL_StatusTypeDef NP_PWM_Start_DMA(void);

bool LED_NP_HW_Init(int npnum)
{    
    LED_NP_Para_Decode((pNpHwConfig_s)&NpHwConfig, &NpParaConfig);
    LED_NP_TIM_Init();
    LED_NP_Buffer_Init(&NpParaConfig);

    /* Configure DMA Channel source address */
    hdma_tim4_ch2.Instance->CPAR = (uint32_t)&(htim4.Instance->CCR2);
    /* Configure DMA Channel destination address */
    hdma_tim4_ch2.Instance->CMAR = (uint32_t)LEDDigitBuffer;
    /* Set the DMA Period elapsed callback */
    hdma_tim4_ch2.XferCpltCallback = TIM_DMADelayPulseCplt;
    /* Set the DMA error callback */
    hdma_tim4_ch2.XferErrorCallback = TIM_DMAError ;

    __HAL_DMA_DISABLE_IT(&hdma_tim4_ch2, DMA_IT_HT);
    
    NP_PWM_Start_DMA();
    osSemaphoreWait(NpOperate_Cplt_FlagHandle, osWaitForever);
    HAL_TIM_PWM_Stop_DMA(&htim4, TIM_CHANNEL_2);

    return true;
}

bool LED_NP_HW_DeInit(void)
{
    return true;
}

bool LED_NP_HW_Update(const void* arg, int npnum)
{
    const RGB* ptrColor = static_cast<const RGB*> (arg);
    
    NpData_Refresh(ptrColor, npnum);
    NP_PWM_Start_DMA();
    osSemaphoreWait(NpOperate_Cplt_FlagHandle, osWaitForever);
    HAL_TIM_PWM_Stop_DMA(&htim4, TIM_CHANNEL_2);

    return true;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    osSemaphoreRelease(NpOperate_Cplt_FlagHandle);
}


static void LED_NP_TIM_Init(void)
{
    MX_TIM4_Init();
    htim4.Instance->ARR=NpParaConfig.NpPeriod_Value;
    htim4.Instance->PSC=0x0000;
}


static void LED_NP_Buffer_Init(pNpParaConfig_s pNpParaConfig)
{
    unsigned int i;
    unsigned int loopnum;
    
    unsigned int Value0 = (unsigned int)pNpParaConfig->NpV0_Value<<24 | (unsigned int)pNpParaConfig->NpV0_Value<<16 | \
                          (unsigned int)pNpParaConfig->NpV0_Value<<8  | (unsigned int)pNpParaConfig->NpV0_Value;
    
    unsigned char *ptr;
    
    ptr = LEDDigitBuffer;
    
    //fill all buffer with NP date R 0x00, G 0x00, B 0x00;
    loopnum = pNpParaConfig->NpNumber * MX_LEDDIGI_COLORNUM * MX_LEDDIGI_COLORWIDTH / 4;
    for(i=0;i<loopnum;i++) {
        *(unsigned int *)(ptr) = Value0;
        ptr+=4;
    }
    //fill low data for reset symbol
    loopnum += pNpParaConfig->NpRstPeriod_Num;
    for(;i<loopnum;i++) {
        *ptr++=0x00;
    }
    *ptr++=0xFE;
    loopnum += pNpParaConfig->NpRstPeriod_Num+1;
    for(;i<loopnum;i++) {
        *ptr++=0x00;
    }
}

static HAL_StatusTypeDef NP_PWM_Start_DMA(void)
{
  DMA_HandleTypeDef *hdma;

  htim4.Instance->CCR2=0x0000;
  htim4.Instance->EGR=TIM_EGR_UG;
  htim4.Instance->CNT=0x0001;
  htim4.Instance->CCR2=LEDDigitBuffer[0];

  hdma=htim4.hdma[TIM_DMA_ID_CC2];

  if((htim4.State == HAL_TIM_STATE_BUSY))
  {
     return HAL_BUSY;
  }
  else if((htim4.State == HAL_TIM_STATE_READY))
  {
      htim4.State = HAL_TIM_STATE_BUSY;
  }

  /* Process locked */
  __HAL_LOCK(hdma);
  
  if(HAL_DMA_STATE_READY == hdma->State)
  {
    /* Change DMA peripheral state */
    hdma->State = HAL_DMA_STATE_BUSY;
    hdma->ErrorCode = HAL_DMA_ERROR_NONE;
    
    /* Disable the peripheral */
    __HAL_DMA_DISABLE(hdma);

    hdma->Instance->CNDTR = NpParaConfig.DmaDataBufferSize;
    
    /* Clear all flags */
    hdma->DmaBaseAddress->IFCR = (DMA_ISR_GIF1 << hdma->ChannelIndex);
    
    __HAL_DMA_ENABLE_IT(hdma, (DMA_IT_TC | DMA_IT_TE));
    
    /* Enable the Peripheral */
    __HAL_DMA_ENABLE(hdma);
  }
  else
  {      
    /* Process Unlocked */
    __HAL_UNLOCK(hdma); 

    /* Remain BUSY */
    return HAL_BUSY;
  }    

  /* Enable the TIM Capture/Compare 2 DMA request */
  __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_CC2);

  /* Enable the Capture compare channel */
  TIM_CCxChannelCmd(htim4.Instance, TIM_CHANNEL_2, TIM_CCx_ENABLE);

  if(IS_TIM_BREAK_INSTANCE(htim4.Instance) != RESET)
  {
    /* Enable the main output */
    __HAL_TIM_MOE_ENABLE(&htim4);
  }

  /* Enable the Peripheral */
  __HAL_TIM_ENABLE(&htim4);

  /* Return function status */
  return HAL_OK;
}

static void LED_NP_Para_Decode(pNpHwConfig_s pNpHwConfig, pNpParaConfig_s pNpParaConfig)
{
    uint16_t temp16;
    
    //parse parameters
    unsigned int RequiredBufferSize;
    pNpParaConfig->NpNumber = pNpHwConfig->NpNumber;
    pNpParaConfig->NpRstPeriod_Num = pNpHwConfig->NpRstnWidth_Ns/pNpHwConfig->NpPeriod_Ns + MX_LEDDIGI_RSTMAGIN;
    
    RequiredBufferSize = (pNpParaConfig->NpNumber * MX_LEDDIGI_COLORNUM * MX_LEDDIGI_COLORWIDTH + pNpParaConfig->NpRstPeriod_Num +\
                          1 + pNpParaConfig->NpRstPeriod_Num);

    pNpParaConfig->DmaDataBufferSize = RequiredBufferSize;
    pNpParaConfig->NpRGBOrder = pNpHwConfig->NpRGBOrder;   
    
    temp16=pNpHwConfig->NpV0HighWitdh_Ns*pNpHwConfig->TimerClkFreq_Mhz/1000;
    if(pNpHwConfig->NpV0HighWitdh_Ns*pNpHwConfig->TimerClkFreq_Mhz%1000) {
        temp16++;
    }
    
    pNpParaConfig->NpPeriod_Value = pNpHwConfig->TimerClkFreq_Mhz*pNpHwConfig->NpPeriod_Ns/1000;
    pNpParaConfig->NpV0_Value = temp16;
    pNpParaConfig->NpV1_Value = pNpParaConfig->NpPeriod_Value - pNpParaConfig->NpV0_Value;
}


__INLINE static void Pixel2TimData(uint8_t * rgb, uint8_t * buffer)
{
    for(int j = 0; j < 3; j++) {
        for (int k = 0; k < 8; k++) {
          if (*rgb & 0x80)
            *buffer++ = NpParaConfig.NpV1_Value;
          else
            *buffer++ = NpParaConfig.NpV0_Value;
          *rgb <<= 1;            
        }
        rgb++;
    }
}

static void NpData_Refresh(const RGB* ptrColor, int num)
{
    uint8_t* databufpoint = LEDDigitBuffer;
    uint8_t Color[3];
    uint8_t RGBOrder;

    RGBOrder=NpParaConfig.NpRGBOrder;
    
    for(uint16_t i=0;i<num;i++) {
        switch(RGBOrder) {
        case NP_RGB:
            Color[0]=ptrColor->wR();
            Color[1]=ptrColor->wG();            
            Color[2]=ptrColor->wB();
            break;
        case NP_RBG:
            Color[0]=ptrColor->wR();
            Color[1]=ptrColor->wB();            
            Color[2]=ptrColor->wG();
        case NP_GRB:
            Color[0]=ptrColor->wG();
            Color[1]=ptrColor->wR();            
            Color[2]=ptrColor->wB();
            break;
        case NP_GBR:
            Color[0]=ptrColor->wG();
            Color[1]=ptrColor->wB();            
            Color[2]=ptrColor->wR();
            break;
        case NP_BRG:
            Color[0]=ptrColor->wB();
            Color[1]=ptrColor->wR();            
            Color[2]=ptrColor->wG();
            break;
        case NP_BGR:
            Color[0]=ptrColor->wB();
            Color[1]=ptrColor->wG();            
            Color[2]=ptrColor->wR();
            break;
        }

        Pixel2TimData(Color, databufpoint);
        databufpoint += 24;
        ptrColor++;
    }
}



