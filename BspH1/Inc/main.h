/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PATH_CONFIG "0://SETTING.txt"
#define WAV_BOOT "0:/System/Boot.wav"
#define WAV_CHARGING "0:/System/charging.wav"
#define WAV_COLORSWITCH "0:/System/ColorSwitch.wav"
#define WAV_LOWPOWER "0:/System/lowpower.wav"
#define WAV_POWEROFF "0:/System/Poweroff.wav"
#define WAV_RECHARGE "0:/System/Recharge.wav"
#define WAV_ERROR "0:/System/alarm.wav"
#define Trigger_B_name "Swing"
#define Trigger_C_name "Clash"
#define Trigger_D_name "Blaster"
#define Trigger_E_name "Lockup"
#define SIMPLELED3_Pin GPIO_PIN_13
#define SIMPLELED3_GPIO_Port GPIOC
#define CHG_VALID_Pin GPIO_PIN_14
#define CHG_VALID_GPIO_Port GPIOC
#define SD_DETn_Pin GPIO_PIN_15
#define SD_DETn_GPIO_Port GPIOC
#define SIMPLELED1_Pin GPIO_PIN_0
#define SIMPLELED1_GPIO_Port GPIOD
#define SIMPLELED2_Pin GPIO_PIN_1
#define SIMPLELED2_GPIO_Port GPIOD
#define ADC_VBAT_Pin GPIO_PIN_0
#define ADC_VBAT_GPIO_Port GPIOA
#define SIMPLELED4_Pin GPIO_PIN_1
#define SIMPLELED4_GPIO_Port GPIOA
#define SIMPLELED5_Pin GPIO_PIN_2
#define SIMPLELED5_GPIO_Port GPIOA
#define AUDIO_ENABLE_Pin GPIO_PIN_3
#define AUDIO_ENABLE_GPIO_Port GPIOA
#define PaDacCh2_Pin GPIO_PIN_5
#define PaDacCh2_GPIO_Port GPIOA
#define SIMPLELED8_Pin GPIO_PIN_6
#define SIMPLELED8_GPIO_Port GPIOA
#define LS3DH_INT1_Pin GPIO_PIN_0
#define LS3DH_INT1_GPIO_Port GPIOB
#define LS3DH_INT2_Pin GPIO_PIN_1
#define LS3DH_INT2_GPIO_Port GPIOB
#define SENSOR_CSN_Pin GPIO_PIN_2
#define SENSOR_CSN_GPIO_Port GPIOB
#define SIMPLELED7_Pin GPIO_PIN_10
#define SIMPLELED7_GPIO_Port GPIOB
#define SIMPLELED6_Pin GPIO_PIN_11
#define SIMPLELED6_GPIO_Port GPIOB
#define USB_PULL_Pin GPIO_PIN_12
#define USB_PULL_GPIO_Port GPIOB
#define SUB_KEY_Pin GPIO_PIN_10
#define SUB_KEY_GPIO_Port GPIOA
#define TF_CSN_Pin GPIO_PIN_15
#define TF_CSN_GPIO_Port GPIOA
#define POWER_EN_Pin GPIO_PIN_8
#define POWER_EN_GPIO_Port GPIOB
#define POWER_KEY_Pin GPIO_PIN_9
#define POWER_KEY_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
