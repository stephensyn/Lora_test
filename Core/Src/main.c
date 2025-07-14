/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Sx1276-1278.h"
#include "delay.h"
#include "user.h"

#include "sx1276-LoRa.h"

#include "uart_dma_queue.h" // 添加UART DMA队列头文件
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/**************************************************************************************************************************************
Demo 程序流程  EnableMaster=true  为主机端，主机端发�?�一�??"PING"数据后切换到接收，等待从机返回的应答"PONG"数据LED闪烁

               EnableMaster=false 为从机端，从机端接收到主机端发过来的"PING"数据后LED闪烁并发送一�??"PONG"数据作为应答
***************************************************************************************************************************************/

// 提示: FSK与LORA模式切换只需要�?�过radio.h 文件中的宏定义修�??   #define LORA       1// [0: OFF, 1: ON] 1：LORA模式 0:FSK模式

// #define EnableMaster GPIO_ReadInputDataBit(RF_MODE_PORT, RF_MODE_PIN) // 主从选择�??
#define EnableMaster HAL_GPIO_ReadPin(RF_MODE_PIN_GPIO_Port, RF_MODE_PIN_Pin) // 读取主从选择脚状�??

tRadioDriver *Radio = 0;
const uint8_t PingMsg[] = "PING123456"; // 主机端发送的PING数据包内容
const uint8_t PongMsg[] = "PONG123456"; // 从机端发送的PONG数据包内容
uint16_t num_rx = 0;
uint8_t RXBuffer[RF_BUFFER_SIZE]; // RX buffer
uint8_t TXBuffer[RF_BUFFER_SIZE]; // TX buffer
uint16_t crc_value;

uint16_t masterSendTime = 2;  // 主机端发送数据包的次数
uint16_t masterSendCount = 0; // 从机端发送数据包的计数

uint8_t temParameter = 0x00; // 用于暂存带修改的参数

static uint8_t last_rx_data[3][256];  // 存储最近3次接收的数据
static uint16_t last_rx_len[3] = {0}; // 存储最近3次接收数据的长度
static uint8_t rx_count = 0;          // 接收计数器

extern tLoRaSettings LoRaSettings; // 声明为外部变量
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
// 检查连续3次接收数据是否都不同的函数
static bool check_three_different_rx(uint8_t *current_data, uint16_t current_len)
{
  if (rx_count < 3)
  {
    return false; // 还没有收到3次数据
  }

  // 检查当前数据与前3次数据是否都不同
  for (int i = 0; i < 3; i++)
  {
    if (current_len == last_rx_len[i] &&
        memcmp(current_data, last_rx_data[i], current_len) == 0)
    {
      return false; // 发现相同数据
    }
  }
  return true; // 都不同
}

// 更新接收数据历史记录
static void update_rx_history(uint8_t *data, uint16_t len)
{
  static uint8_t history_index = 0;

  // 复制数据到历史记录
  memcpy(last_rx_data[history_index], data, len);
  last_rx_len[history_index] = len;

  // 更新索引（循环使用）
  history_index = (history_index + 1) % 3;

  // 增加计数（最大为3）
  if (rx_count < 3)
  {
    rx_count++;
  }
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LedToggle(void)
{
  // GPIO_WriteBit(LED1_PORT, LED1_PIN, Bit_RESET); // LED闪烁
  HAL_GPIO_WritePin(LED1_PIN_GPIO_Port, LED1_PIN_Pin, GPIO_PIN_RESET);
  // HAL_Delay_nMs(1000);
  HAL_Delay_nMs(10);
  // GPIO_WriteBit(LED1_PORT, LED1_PIN, Bit_SET);
  HAL_GPIO_WritePin(LED1_PIN_GPIO_Port, LED1_PIN_Pin, GPIO_PIN_SET);
}

/*
 * Manages the master operation
 */
void OnMaster(void)
{
  switch (Radio->Process())
  {
  case RF_TX_DONE:

    Radio->StartRx();
    break;

  case RF_RX_DONE:
    Radio->GetRxPacket(RXBuffer, (uint16_t *)&num_rx);

    if (num_rx > 0)
    {
      // HAL_UART_Transmit(&huart1, RXBuffer, num_rx, HAL_MAX_DELAY);
      HAL_UART_Transmit_DMA(&huart1, RXBuffer, num_rx); // 使用DMA发送数据包
      // crc_value = RXBuffer[num_rx - 2];
      // crc_value <<= 8;
      // crc_value |= RXBuffer[num_rx - 1];
      // if (crc_value == RadioComputeCRC(RXBuffer, num_rx - 2, CRC_TYPE_IBM)) // CRC check
      // {
      // if (strncmp((const char *)RXBuffer, (const char *)PongMsg, strlen((const char *)PongMsg)) == 0)
      // {
      LedToggle(); // LED闪烁

      // Send the next PING frame
      strncpy((char *)TXBuffer, PingMsg, sizeof(TXBuffer));

      // crc_value = RadioComputeCRC(TXBuffer, 4, CRC_TYPE_IBM); // 计算得出要发送数据包CRC值
      // TXBuffer[4] = crc_value >> 8;
      // TXBuffer[5] = crc_value;

      // if ((masterSendCount < masterSendTime))
      // {
      /* code */
      Radio->SetTxPacket(TXBuffer, strlen((const char *)TXBuffer)); // 使用字符串长度作为数据包长度
      //   masterSendCount++;
      // }

      // }
      // }
    }
    break;

  case RF_RX_TIMEOUT:
    // Send the next PING frame
    strncpy((char *)TXBuffer, PingMsg, sizeof(TXBuffer)); // 确保TXBuffer正确初始化

    // crc_value = RadioComputeCRC(TXBuffer, 4, CRC_TYPE_IBM); // 计算得出要发送数据包CRC值
    // TXBuffer[4] = crc_value >> 8;
    // TXBuffer[5] = crc_value;

    // if ((masterSendCount < masterSendTime))
    // {
    /* code */
    Radio->SetTxPacket(TXBuffer, strlen((const char *)TXBuffer));
    //   masterSendCount++;
    // }
    break;

  default:
    break;
  }
}

/*
 * Manages the slave operation
 */
void OnSlave(void)
{
  switch (Radio->Process())
  {
  case RF_IDLE:
    // 进入载波侦听模式
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"\r\nRF_IDLE\r\n", 12); // 调试输出
    Radio->EnterCadMode();                                            // 进入载波侦听模式
    break;
  case RF_BUSY:

    break;

  case RF_CHANNEL_ACTIVITY_DETECTED:
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"\r\nRF_CHANNEL_ACTIVITY_DETECTED\r\n", 34); // 调试输出
    // 进入接收模式
    Radio->StartRx();
    break;
  case RF_CHANNEL_EMPTY:
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"\r\nRF_CHANNEL_EMPTY\r\n", 22); // 调试输出
    // LoRaSettings.SyncWordValue++;         // 增加同步字参数
    // Radio->parameterChange(temParameter); // 重新设置同步字
    Radio->EnterCadMode(); // 进入载波侦听模式
    break;

  case RF_RX_DONE:

    Radio->GetRxPacket(RXBuffer, (uint16_t *)&num_rx);

    if (num_rx > 0)
    {

      // 检查是否连续3次接收到不同数据
      // if (check_three_different_rx(RXBuffer, num_rx))
      // {

      //   LoRaSettings.SyncWordValue++; // 增加同步字参数
      //   Radio->parameterChange(0x10); // 重新设置同步字
      // }

      // 更新接收历史记录
      // update_rx_history(RXBuffer, num_rx);

      // HAL_UART_Transmit(&huart1, RXBuffer, num_rx, HAL_MAX_DELAY);
      // HAL_UART_Transmit_DMA(&huart1, RXBuffer, num_rx);                 // 使用DMA发送数据包
      // 在RXBuffer末尾添加换行符和字符串结束符
      RXBuffer[num_rx] = '\n';     // 添加换行符
      RXBuffer[num_rx + 1] = '\0'; // 添加字符串结束符，确保安全
      UART_TxQueue_Enqueue(RXBuffer, num_rx + 1);
      // HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"RECIVED INF\r\n", 14); // 调试输出
      UART_TxQueue_Enqueue((uint8_t *)"RECIVED INF\r", 13); // 将调试信息放入队列中
      // crc_value = RXBuffer[num_rx - 2];
      // crc_value <<= 8;
      // crc_value |= RXBuffer[num_rx - 1];
      // if (crc_value == RadioComputeCRC(RXBuffer, num_rx - 2, CRC_TYPE_IBM)) // CRC check
      // {
      // if (strncmp((const char *)RXBuffer, (const char *)PingMsg, strlen((const char *)PingMsg)) == 0)
      // {
      LedToggle(); // LED闪烁

      // Ack with PONG frame
      // strncpy((char *)TXBuffer, PongMsg, sizeof(TXBuffer));

      // crc_value = RadioComputeCRC(TXBuffer, 4, CRC_TYPE_IBM); // 计算得出要发送数据包CRC值
      // TXBuffer[4] = crc_value >> 8;
      // TXBuffer[5] = crc_value;

      // Radio->SetTxPacket(TXBuffer, strlen((const char *)TXBuffer));
      // }
      // }
    }
    else
    {
      // No data received,changed syncWord start RX again
      // if (temParameter == 0xff)
      // {
      //   temParameter = 0x00; // 设置一个新的同步字
      // }
      // else
      // {
      //   temParameter++; // 恢复为默认同步字
      // }
      // Radio->parameterChange(temParameter); // 重新设置同步字
      // Radio->EnterCadMode(); // 进入载波侦听模式
    }
    // HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"RF_RX_DONE\r", 13); // 调试输出
    UART_TxQueue_Enqueue((uint8_t *)"RF_RX_DONE\r", 12); // 将调试信息放入队列中
    break;

  case RF_RX_TIMEOUT:
    // HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"RF_RX_TIMEOUT\r", 16); // 调试输出
    UART_TxQueue_Enqueue((uint8_t *)"RF_RX_TIMEOUT\r", 15); // 将调试信息放入队列中
    // 重新进入接收模式
    Radio->StartRx();
    // 改变同步字
    // LoRaSettings.SyncWordValue++;         // 增加同步字参数
    // Radio->parameterChange(temParameter); // 重新设置同步字
    // Radio->EnterCadMode(); // 进入载波侦听模式

    break;

  default:
    break;
  }
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  if (UART_TxQueue_Init() != 0)
  {
    // 处理初始化失败
    while (1)
      ;
  }
  MX_SPI2_Init();
  MX_NVIC_Init(); // NVIC配置

  /* USER CODE BEGIN 2 */
  Radio = RadioDriverInit();
  if (Radio != NULL)
  {
    Radio->Init();
    // RFLRState = RFLR_STATE_IDLE; // LoRa状态机初始化为IDLE状态
  }
  else
  {
    // Handle null pointer error, e.g., log or halt execution
    // printf("Radio driver initialization failed!\n");
    while (1)
      ; // Infinite loop to indicate error
  }

  if (EnableMaster)
  {
    // Correctly initialize the TXBuffer with "PING_TEST1"
    strncpy((char *)TXBuffer, "PING123456", sizeof(TXBuffer)); // TXBuffer初始化

    Radio->SetTxPacket(TXBuffer, strlen((const char *)TXBuffer)); // 使用字符串长度作为数据包长度
    // masterSendCount++;
  }
  else
  {
    Radio->EnterCadMode(); // 进入载波侦听模式

    // Radio->StartRx();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    if (EnableMaster)
    {
      // HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"Enter Master Mode\r\n", 20); // 主机端模式提示
      OnMaster();
    }
    else
    {

      OnSlave();
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }

  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the Systick
   */
  HAL_SYSTICK_Config(6000);                                 //
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8); // Systick时钟源为HCLK/8
}

/* USER CODE BEGIN 4 */
/**
 * @brief NVIC Configuration.
 * @retval None
 */
static void MX_NVIC_Init(void)
{
  /* EXTI0_1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
