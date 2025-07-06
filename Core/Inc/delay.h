#ifndef _DELAY_H_
#define _DELAY_H_

#include <stdint.h>
#include "stm32f0xx_hal.h" // 根据你的具体芯片型号选择正确的HAL头文件

// extern volatile uint32_t TickCounter;
// extern volatile uint32_t ticktimer;

void Delay_Us(uint32_t delay);
void Delay_Ms(uint32_t delay);
void HAL_Delay_nMs(uint32_t Delay);

 


#endif
