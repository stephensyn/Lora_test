/*
 * 以下固件提供：(1) "按原样"提供，无任何担保；以及
 * (2)提供编码信息以指导和方便客户使用。
 * 因此，SEMTECH不对因使用此固件内容和/或客户在其产品中使用此编码信息而产生的任何直接、间接或后果性损害承担责任。
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx12xxEiger.h
 * \brief      头文件定义
 *
 * \version    1.0
 * \date       2012年11月21日
 * \author     Miguel Luis
 */
#ifndef __SX12XXEIGER_H__
#define __SX12XXEIGER_H__

#include <stdint.h>
#include <stdbool.h>

// 根据不同的芯片型号包含对应的头文件
#if defined( STM32F4XX ) || defined( STM32F429_439xx )
    #include "stm32f4xx.h"
#elif defined( STM32F2XX )
    #include "stm32f2xx.h"
#else
   #include "stm32f0xx.h"
#endif

// 定义是否使用USB
#define USE_USB                                     1

// 定义备份寄存器用于引导加载器
#if defined( STM32F4XX ) || defined( STM32F2XX ) || defined( STM32F429_439xx )
#define BACKUP_REG_BOOTLOADER                       RTC_BKP_DR0      /* 引导加载器进入 */
#else
#define BACKUP_REG_BOOTLOADER                       BKP_DR1          /* 引导加载器进入 */
#endif

// 固件版本定义
#define FW_VERSION                                  "2.1.0"
// 开发板名称定义
#define SK_NAME                                     "SX12xxEiger"

/*!
 * 函数返回码定义
 */
typedef enum
{
    SX_OK,          // 操作成功
    SX_ERROR,       // 操作失败
    SX_BUSY,        // 系统忙
    SX_EMPTY,       // 空
    SX_DONE,        // 完成
    SX_TIMEOUT,     // 超时
    SX_UNSUPPORTED, // 不支持
    SX_WAIT,        // 等待
    SX_CLOSE,       // 关闭
    SX_YES,         // 是
    SX_NO,          // 否
}tReturnCodes;

// 全局变量，计时器计数器
extern volatile uint32_t TickCounter;

/**
  * @brief   GCC/RAISONANCE的小型printf函数
  */
#ifdef __GNUC__
/* 使用GCC/RAISONANCE时，小型printf（选项LD Linker->Libraries->Small printf设置为'Yes'）调用__io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

#endif /* __GNUC__ */

/*!
 * \brief 计算一个介于min和max之间的随机数
 *
 * \param [IN] min 范围的最小值
 * \param [IN] max 范围的最大值
 * \retval random 返回介于min和max之间的随机值
 */
uint32_t randr( uint32_t min, uint32_t max );

#endif // __SX12XXEIGER_H__
