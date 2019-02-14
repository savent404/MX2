/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "stm32f1xx_hal.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId DACTaskHandle;
osThreadId LEDTaskHandle;
osThreadId WavTaskHandle;
osMessageQId DAC_BufferHandle;
osMessageQId DAC_CMDHandle;
osMessageQId LED_CMDHandle;
osSemaphoreId DAC_Complete_FlagHandle;
osSemaphoreId VBAT_LOW_FLAGHandle;
osSemaphoreId SdOperate_Cplt_FlagHandle;
osSemaphoreId NpOperate_Cplt_FlagHandle;
osSemaphoreId LedOut_Cplt_FlagHandle;



/* USER CODE BEGIN Variables */
osThreadId SimpleLEDHandle;
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void DACOutput(void const * argument);
void LEDOpra(void const * argument);
void Wav_Task(void const * argument);
void Test_Task(void const * argument);

extern void MX_FATFS_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
//extern void SimpleLED_Handle(void const *arg);
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
   __ASM("BKPT 0");
   printf("[FreeRTOS]:a stack overflow is detected.\r\n");
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
   __ASM("BKPT 0");
   printf("[FreeRTOS]:Malloc Failed.\r\n");
}
/* USER CODE END 5 */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of DAC_Complete_Flag */
  osSemaphoreDef(DAC_Complete_Flag);
  DAC_Complete_FlagHandle = osSemaphoreCreate(osSemaphore(DAC_Complete_Flag), 1);

  /* definition and creation of VBAT_LOW_FLAG */
  osSemaphoreDef(VBAT_LOW_FLAG);
  VBAT_LOW_FLAGHandle = osSemaphoreCreate(osSemaphore(VBAT_LOW_FLAG), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  osSemaphoreDef(SdOperate_Cplt_Flag);
  SdOperate_Cplt_FlagHandle = osSemaphoreCreate(osSemaphore(SdOperate_Cplt_Flag), 1);

  osSemaphoreDef(NpOperate_Cplt_Flag);
  NpOperate_Cplt_FlagHandle = osSemaphoreCreate(osSemaphore(NpOperate_Cplt_Flag), 1);

  osSemaphoreDef(LedOut_Cplt_Flag);
  LedOut_Cplt_FlagHandle = osSemaphoreCreate(osSemaphore(LedOut_Cplt_Flag), 1);

  osSemaphoreWait(SdOperate_Cplt_FlagHandle, 0);
  osSemaphoreWait(NpOperate_Cplt_FlagHandle, 0);
  osSemaphoreWait(LedOut_Cplt_FlagHandle, 0);
  
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityHigh, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of DACTask */
  osThreadDef(DACTask, DACOutput, osPriorityNormal, 0, 128);
  DACTaskHandle = osThreadCreate(osThread(DACTask), NULL);

  /* definition and creation of LEDTask */
  osThreadDef(LEDTask, LEDOpra, osPriorityRealtime, 0, 128);
  LEDTaskHandle = osThreadCreate(osThread(LEDTask), NULL);

  /* definition and creation of WavTask */
  osThreadDef(WavTask, Wav_Task, osPriorityAboveNormal, 0, 1024);
  WavTaskHandle = osThreadCreate(osThread(WavTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(TestTask, Test_Task, osPriorityLow, 0, 1024);
  SimpleLEDHandle = osThreadCreate(osThread(TestTask), NULL);
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of DAC_Buffer */
  osMessageQDef(DAC_Buffer, 3, uint32_t);
  DAC_BufferHandle = osMessageCreate(osMessageQ(DAC_Buffer), NULL);

  /* definition and creation of DAC_CMD */
  osMessageQDef(DAC_CMD, 16, uint32_t);
  DAC_CMDHandle = osMessageCreate(osMessageQ(DAC_CMD), NULL);

  /* definition and creation of LED_CMD */
  osMessageQDef(LED_CMD, 16, uint32_t);
  LED_CMDHandle = osMessageCreate(osMessageQ(LED_CMD), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
__weak void StartDefaultTask(void const * argument)
{
  /* init code for FATFS */
  MX_FATFS_Init();

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* DACOutput function */
__weak void DACOutput(void const * argument)
{
  /* USER CODE BEGIN DACOutput */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END DACOutput */
}

/* LEDOpra function */
__weak void LEDOpra(void const * argument)
{
  /* USER CODE BEGIN LEDOpra */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END LEDOpra */
}

/* Wav_Task function */
__weak void Wav_Task(void const * argument)
{
  /* USER CODE BEGIN Wav_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END Wav_Task */
}

/* USER CODE BEGIN Application */
__weak void Test_Task(void const * argument)
{
  /* USER CODE BEGIN Wav_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END Wav_Task */
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
