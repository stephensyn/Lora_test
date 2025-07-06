/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    spi.c
 * @brief   This file provides code for the configuration
 *          of the SPI instances.
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
#include "spi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef hspi2;

/* SPI2 init function */
void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  hspi2.Instance = SPI2;                                  // SPI2外设实例
  hspi2.Init.Mode = SPI_MODE_MASTER;                      // 配置为主机模式
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;            // 双线全双工模式
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;                // 数据大小为8位
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;              // 时钟极性为低电平空闲
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;                  // 时钟相位为第一个边沿采样
  hspi2.Init.NSS = SPI_NSS_SOFT;                          // 软件管理NSS信号
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; // 波特率预分频为8
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;                 // 数据从MSB（最高有效位）开始传输
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;                 // 禁用TI模式

  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; // 启用CRC校验
  hspi2.Init.CRCPolynomial = 7;                           // CRC多项式设置为7
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;         // CRC长度与数据大小一致
  // hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;            // 启用NSS脉冲模式

  // hspi1.Instance = SPI1;
  // hspi1.Init.Mode = SPI_MODE_MASTER;
  // hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  // hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  // hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  // hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  // hspi1.Init.NSS = SPI_NSS_SOFT;
  // hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  // hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  // hspi1.Init.TIMode = SPI_TIMODE_DISABLE;

  // hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  // hspi2.Init.CRCPolynomial = 10;

  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (spiHandle->Instance == SPI2)
  {
    /* USER CODE BEGIN SPI2_MspInit 0 */

    /* USER CODE END SPI2_MspInit 0 */
    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin = RF_CLK_AF_Pin | RF_MISO_AF_Pin | RF_MOSI_AF_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN SPI2_MspInit 1 */

    /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *spiHandle)
{

  if (spiHandle->Instance == SPI2)
  {
    /* USER CODE BEGIN SPI2_MspDeInit 0 */

    /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, RF_CLK_AF_Pin | RF_MISO_AF_Pin | RF_MOSI_AF_Pin);

    /* USER CODE BEGIN SPI2_MspDeInit 1 */

    /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
uint8_t HAL_SPI_TransmitReceiveByte(SPI_HandleTypeDef *hspi, uint8_t InputData)
{
  uint8_t OutputData = 0;

  if (HAL_SPI_TransmitReceive(hspi, &InputData, &OutputData, 1, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler(); // Handle error if transmission fails
  }
  return OutputData;
}
/* USER CODE END 1 */
