#include "key.h"
#include "stm32f1xx_hal.h"
#include "main.h"
keyStatus_t MX_KEY_GetStatus(keyId_t id)
{
    uint8_t i;
    uint8_t presscnt=0x00;
    uint8_t releasecnt=0x00;
    switch (id)
    {
    case KEY_PWR:
         for(i=0;i<3;i++) {
            if(GPIO_PIN_SET == HAL_GPIO_ReadPin(POWER_KEY_GPIO_Port, POWER_KEY_Pin)) {
                presscnt++;
            }
            else {
                releasecnt++;
            }
        }
        break;
    case KEY_SUB:
        for(i=0;i<3;i++) {
            if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(SUB_KEY_GPIO_Port, SUB_KEY_Pin)) {
                presscnt++;
            }
            else {
                releasecnt++;
            }
        }
    }
    return (presscnt>releasecnt) ? KEY_PRESS : KEY_RELEASE;
}
