/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

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
#define RF_DIO0_IO_Pin GPIO_PIN_0
#define RF_DIO0_IO_GPIO_Port GPIOA
#define RF_DIO0_IO_EXTI_IRQn EXTI0_1_IRQn
#define RF_DIO1_IO_Pin GPIO_PIN_1
#define RF_DIO1_IO_GPIO_Port GPIOA
#define RF_MODE_PIN_Pin GPIO_PIN_2
#define RF_MODE_PIN_GPIO_Port GPIOA
#define RF_DIO2_IO_Pin GPIO_PIN_6
#define RF_DIO2_IO_GPIO_Port GPIOA
#define RF_DIO3_IO_Pin GPIO_PIN_0
#define RF_DIO3_IO_GPIO_Port GPIOB
#define RF_RST_IO_Pin GPIO_PIN_2
#define RF_RST_IO_GPIO_Port GPIOB
#define RF_NSS_IO_Pin GPIO_PIN_12
#define RF_NSS_IO_GPIO_Port GPIOB
#define RF_CLK_AF_Pin GPIO_PIN_13
#define RF_CLK_AF_GPIO_Port GPIOB
#define RF_MISO_AF_Pin GPIO_PIN_14
#define RF_MISO_AF_GPIO_Port GPIOB
#define RF_MOSI_AF_Pin GPIO_PIN_15
#define RF_MOSI_AF_GPIO_Port GPIOB
#define UART1_TX_PIN_Pin GPIO_PIN_9
#define UART1_TX_PIN_GPIO_Port GPIOA
#define UART1_RX_PIN_Pin GPIO_PIN_10
#define UART1_RX_PIN_GPIO_Port GPIOA
#define SWDIO_PIN_Pin GPIO_PIN_13
#define SWDIO_PIN_GPIO_Port GPIOA
#define LED1_PIN_Pin GPIO_PIN_15
#define LED1_PIN_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
