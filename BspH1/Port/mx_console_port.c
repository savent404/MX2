#include "main.h"
#include "cmsis_os.h"
#include "console.h"
#include <stdint.h>

UART_HandleTypeDef huart3;


void MX_Console_HW_Init(void);
void MX_Console_DeInit(void);
void MX_Console_Print(uint8_t *string, uint16_t size, bool);


void MX_Console_HW_Init(void)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        Error_Handler();
    }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uartHandle->Instance==USART3)
    {
    /* USER CODE BEGIN USART3_MspInit 0 */

    /* USER CODE END USART3_MspInit 0 */
      /* USART3 clock enable */
      __HAL_RCC_USART3_CLK_ENABLE();

      __HAL_RCC_GPIOB_CLK_ENABLE();
      /**USART3 GPIO Configuration    
      PB10     ------> USART3_TX
      PB11     ------> USART3_RX 
      */
      GPIO_InitStruct.Pin = GPIO_PIN_10;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      GPIO_InitStruct.Pin = GPIO_PIN_11;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      /* USART3 interrupt Init */
      HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
      HAL_NVIC_EnableIRQ(USART3_IRQn);
    /* USER CODE BEGIN USART3_MspInit 1 */

    /* USER CODE END USART3_MspInit 1 */
    }
}

void MX_Console_DeInit(void)
{
    HAL_UART_DeInit(&huart3);
}


void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();
  
    /**USART3 GPIO Configuration    
    PA9     ------> USART3_TX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);

  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

void MX_Console_Print(uint8_t* string, uint16_t size, bool is_stderr)
{
    int isRunning = osKernelRunning();
    if (!is_stderr && isRunning) {
        MX_ConsoleTX_WaitMutex();
    }

    if (is_stderr)
        HAL_UART_Transmit(&huart3, string, size, 1000);
    else
        // TODO: better to be DMA
        HAL_UART_Transmit(&huart3, string, size, 1000);

    if (!is_stderr && isRunning) {
        //MX_ConsoleTX_WaitSem(); // wait uitll tx cplt
        MX_ConsoleTX_FreeMutex();
    }
}


static char* ptrRxBuffer = NULL;
static int rxBufferSize = 0;
char* MX_Console_Gets(char* buffer, int maxiumSize)
{

    int  isRunning = osKernelRunning();
    if (isRunning) {
        MX_ConsoleRX_WaitMutex();
    }
    if (isRunning) {
        ptrRxBuffer = buffer;
        rxBufferSize = maxiumSize;

        HAL_UART_Receive_IT(&huart3, (uint8_t*)ptrRxBuffer, 1);
        MX_ConsoleRX_WaitSem();
        *ptrRxBuffer = '\0';
    } else {
        char c   = '\0';
        int  pos = 0;
        do {
            HAL_UART_Receive(&huart3, (uint8_t*)&c, 1, osWaitForever);
            buffer[ pos++ ] = c;
        } while (c != '\n' || pos >= maxiumSize - 1);
        buffer[ pos ] = c;
    }
    if (isRunning) {
        MX_ConsoleRX_FreeMutex();
    }
    return buffer;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if (osKernelRunning())
        MX_ConsoleTX_FreeSem();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if (!ptrRxBuffer || rxBufferSize <= 1 || *ptrRxBuffer == '\n' || *ptrRxBuffer == '\r') {
        if (osKernelRunning())
            MX_ConsoleRX_FreeSem();
    } else {
        ptrRxBuffer++;
        rxBufferSize--;
        HAL_UART_Receive_IT(&huart3, (uint8_t*)ptrRxBuffer, 1);
    }
}
