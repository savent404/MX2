#include "SimpleLED.h"
#include "USR_CONFIG.h"
#include "cmsis_os.h"
#include "stm32f1xx.h"

// TODO: LED2 LED3 为晶振输入IO，未处理复用引脚造成无法debug之前，不对LED2 LED3进行操作

static SimpleLED_Acction_t *pacction;
static int32_t SimpleLED_Timer_cnt = 0;
static int32_t SimpleLED_LED_cnt = 0;
static bool loopflag;
static SimpleLED_Acction_t *GetAction(SimpleLED_Status_t status);
static void SimpleLED_Opra(uint8_t led);

void SimpleLED_ChangeStatus(SimpleLED_Status_t status)
{
  pacction = GetAction(status);
}

void SimpleLED_Callback(void const *arg)
{
  if ((SimpleLED_Timer_cnt -= (int)arg) > 0)
    return;

  if ((int)pacction == 0)
  {
    SimpleLED_Opra(0x00);
    SimpleLED_Timer_cnt = 100;
    return;
  }
  else
  {
    if (loopflag == false && SimpleLED_LED_cnt < pacction->Num)
      SimpleLED_Opra(*(pacction->Action + SimpleLED_LED_cnt++) & USR.config->SimpleLED_MASK);
    else if (loopflag == false)
      pacction = GetAction(SIMPLELED_STATUS_ON);
    else if (loopflag == true)
      SimpleLED_Opra(*(pacction->Action + (SimpleLED_LED_cnt++ % pacction->Num)) & USR.config->SimpleLED_MASK);
    SimpleLED_Timer_cnt = pacction->Delay;
  }
}

void SimpleLED_Handle(void const *arg)
{
  SimpleLED_DeInit();
  SimpleLED_Init();

  while (1)
  {
    osDelay(10);
    SimpleLED_Callback(10);
  }
}

static SimpleLED_Acction_t *GetAction(SimpleLED_Status_t status)
{
  SimpleLED_Acction_t *ans;
  switch (status)
  {
  case SIMPLELED_STATUS_SLEEP:
    ans = NULL;
    break;
  case SIMPLELED_STATUS_STANDBY:
    ans = (USR.accent + USR.bank_now)->Standby;
    loopflag = true;
    break;
  case SIMPLELED_STATUS_ON:
    ans = (USR.accent + USR.bank_now)->On;
    loopflag = true;
    break;
  case SIMPLELED_STATUS_CLASH:
    ans = (USR.accent + USR.bank_now)->Clash;
    loopflag = false;
    break;
  case SIMPLELED_STATUS_LOCKUP:
    ans = (USR.accent + USR.bank_now)->Lockup;
    loopflag = true;
    break;
  default:
    ans = NULL;
    loopflag = false;
  }

  SimpleLED_LED_cnt = 0;
  SimpleLED_Timer_cnt = 0;
  return ans;
}

static void SimpleLED_Opra(uint8_t led)
{
  uint16_t odr_buffer;
  odr_buffer = 0;
  // LED 0~1
  odr_buffer |= (led & 0x03) << 14;
  // LED 2~5
  odr_buffer |= led >> 2;

  GPIOC->ODR &= 0x3FF0;
  GPIOC->ODR |= odr_buffer;
}

void SimpleLED_Init(void)
{
  GPIO_InitTypeDef gpiox;

  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_14 | GPIO_PIN_15);
  gpiox.Mode = GPIO_MODE_OUTPUT_PP;
  gpiox.Pull = GPIO_PULLUP;
  gpiox.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_14 | GPIO_PIN_15;
  gpiox.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &gpiox);

  //gpiox.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  //HAL_GPIO_Init(GPIOD, &gpiox);
  //__HAL_AFIO_REMAP_PD01_ENABLE();
  pacction = GetAction(SIMPLELED_STATUS_SLEEP);
}

void SimpleLED_DeInit(void)
{
  // LED 4~7
  HAL_GPIO_DeInit(GPIOC, 0x0F);

  // LED 0~1
  HAL_GPIO_DeInit(GPIOC, 0xC000);

  // LED 2~3
  HAL_GPIO_DeInit(GPIOD, 0x0003);
}
