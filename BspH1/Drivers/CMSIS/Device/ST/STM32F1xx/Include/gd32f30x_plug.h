#ifndef __GD32F30X_PLUG_H
#define __GD32F30X_PLUG_H

#define REG32(addr)                  (*(volatile uint32_t *)(uint32_t)(addr))
#define REG16(addr)                  (*(volatile uint16_t *)(uint32_t)(addr))
#define REG8(addr)                   (*(volatile uint8_t *)(uint32_t)(addr))
#define BIT(x)                ((uint32_t)((uint32_t)0x01U<<(x)))
#define BITS(start, end)      ((0xFFFFFFFFUL << (start)) & (0xFFFFFFFFUL >> (31U - (uint32_t)(end)))) 

#define __CM4_REV               0x0001   /*!< Core revision r0p1                           */
#define __MPU_PRESENT           0U       /*!< Other STM32 devices does not provide an MPU  */
#define __NVIC_PRIO_BITS        4U       /*!< STM32 uses 4 Bits for the Priority Levels    */
#define __Vendor_SysTickConfig  0U       /*!< Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT           1        /*!< FPU                                          */ 

#ifndef __STM32F103xE_H
#include "stm32f103xe.h"
#endif

#define RCU_BASE              (RCC_BASE)  /*!< RCU base  */

#define RCU                   RCU_BASE

#define APB1_BUS_BASE         ((uint32_t)0x40000000U)        /*!< apb1 base address                */
#define CTC_BASE              (APB1_BUS_BASE + 0x0000C800U)  /*!< CTC base address                 */


#define ErrStatus ErrorStatus

#endif
