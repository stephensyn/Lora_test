#include "delay.h"


void Delay_Us(uint32_t delay)
{
  uint32_t tickstart = HAL_GetTick();
  while ((HAL_GetTick() - tickstart) < (delay / 1000)); // 微秒转换为毫秒
}

void Delay_Ms(uint32_t delay)
{
  HAL_Delay(delay); // 使用HAL库提供的延迟函数
}

void HAL_Delay_nMs(uint32_t Delay)
{
  HAL_Delay(Delay); // HAL库已经提供了毫秒级延迟函数
}

