#include "key.h"
#include "stm32f1xx_hal.h"
#include "main.h"
keyStatus_t MX_KEY_GetStatus(keyId_t id)
{
    bool isPressed = false;
    switch (id)
    {
    case KEY_PWR:
        isPressed = HAL_GPIO_ReadPin(POWER_GPIO_Port, POWER_Pin) == GPIO_PIN_SET;
        break;
    case KEY_SUB:
        isPressed = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET;
    }
    return isPressed ? KEY_PRESS : KEY_RELEASE;
}
