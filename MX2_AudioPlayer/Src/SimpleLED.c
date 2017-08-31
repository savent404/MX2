#include "SimpleLED.h"
#include "USR_CONFIG.h"
#include "cmsis_os.h"
#include "stm32f1xx.h"

// TODO: LED2 LED3 为晶振输入IO，未处理复用引脚造成无法debug之前，不对LED2 LED3进行操作

static SimpleLED_Acction_t *pacction;
static uint32_t cnt;
static bool loopflag;

static SimpleLED_Acction_t* GetAction(SimpleLED_Status_t status);
static void SimpleLED_Opra(uint8_t led);


void SimpleLED_ChangeStatus(SimpleLED_Status_t status)
{
  pacction = GetAction(status);
}

void SimpleLED_Handle(void const *arg)
{
  SimpleLED_Init();

  pacction = GetAction(SIMPLELED_STATUS_SLEEP);

  while (1)
  {

    if ((int)pacction == 0)
    {
      SimpleLED_Opra(0x00);
      osDelay(10);
    }
    else
    {
      if (loopflag == false && cnt < pacction->Num)
        SimpleLED_Opra(*(pacction->Action + cnt++) & USR.SimpleLED_MASK);
      else if (loopflag == true)
        SimpleLED_Opra(*(pacction->Action + (cnt++ % pacction->Num)) & USR.SimpleLED_MASK);
      osDelay(pacction->Delay);
    }
  }
}

static SimpleLED_Acction_t* GetAction(SimpleLED_Status_t status)
{
  SimpleLED_Acction_t *ans;
  switch (status)
  {
    case SIMPLELED_STATUS_SLEEP:
      ans = NULL;
      break;
    case SIMPLELED_STATUS_STANDBY:
      ans = USR.accent->Standby + USR.bank_now;
      loopflag = true;
      break;
    case SIMPLELED_STATUS_ON:
      ans = USR.accent->On + USR.bank_now;
      loopflag = true;
      break;
    case SIMPLELED_STATUS_CLASH:
      ans = USR.accent->Clash + USR.bank_now;
      loopflag = false;
      break;
    case SIMPLELED_STATUS_LOCKUP:
      ans = USR.accent->Lockup + USR.bank_now;
      loopflag = true;
      break;
    default:
      ans = NULL;
      loopflag = false;
  }

  cnt = 0;
  return ans;
}

static void SimpleLED_Opra(uint8_t led)
{
  // LED 4~7
  GPIOC->ODR |= led >> 4;
  GPIOC->ODR &= led >> 4 | 0xFFF0;

  // LED 0~1
  GPIOC->ODR |= ((uint16_t)led & 0x03) << 14;
  GPIOC->ODR &= ((uint16_t)led & 0x03) << 14 | 0x3FFF;
}

void SimpleLED_Init(void)
{
  GPIO_InitTypeDef gpiox;

  // LED 4~7
  gpiox.Pin = 0x0F;
  gpiox.Mode = GPIO_MODE_OUTPUT_PP;
  gpiox.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &gpiox);

  gpiox.Pin = 0x3000;
  HAL_GPIO_Init(GPIOC, &gpiox);
}

void SimpleLED_DeInit(void)
{
  // LED 4~7
  HAL_GPIO_DeInit(GPIOC, 0x0F);

  // LED 0~1
  HAL_GPIO_DeInit(GPIOC, 0x3000);
}
