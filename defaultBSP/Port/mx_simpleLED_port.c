#include "SimpleLED.h"
#include "stm32f1xx.h"

uint32_t pins = GPIO_PIN_0 |
                GPIO_PIN_1 |
                GPIO_PIN_2 |
                GPIO_PIN_3 |
                GPIO_PIN_14 |
                GPIO_PIN_15;

void SimpleLED_HW_Init(void)
{
    SimpleLED_HW_DeInit();

    GPIO_InitTypeDef gpiox;
    gpiox.Mode = GPIO_MODE_OUTPUT_PP;
    gpiox.Pull = GPIO_PULLUP;
    gpiox.Pin = pins;
    gpiox.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpiox);
}

void SimpleLED_HW_DeInit(void)
{
    HAL_GPIO_DeInit(GPIOC, pins);
}

void SimpleLED_HW_Opra(uint32_t led)
{
    uint16_t odr_buffer = 0;

    odr_buffer |= (led & 0x03) << 14;
    odr_buffer |= led >> 2;
    GPIOC->ODR &= 0x3FF0;
    GPIOC->ODR |= odr_buffer;
}
