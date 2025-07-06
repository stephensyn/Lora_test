/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       platform.h
 * \brief        
 *
 * \version    1.0
 * \date       Nov 21 2012
 * \author     Miguel Luis
 */
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifndef __GNUC__
#define inline
#endif

/*!
 * 平台定义
 */
#define Bleeper                                     3
#define SX1243ska                                   2
#define SX12xxEiger                                 1
#define SX12000DVK                                  0

/*!
 * 平台选择。请取消注释 PLATFORM 定义并选择您的平台，
 * 或在编译器定义选项中添加/更改 PLATFORM 定义。
 */

#define PLATFORM                                    SX12xxEiger

#if( PLATFORM == SX12xxEiger )
/*!
 * 无线电选择。请取消注释所需的无线电并注释其他选项，
 * 或在编译器定义选项中添加/更改所需的无线电定义。
 */
//#define USE_SX1232_RADIO
//#define USE_SX1272_RADIO
#define USE_SX1276_RADIO
//#define USE_SX1243_RADIO

/*!
 * 模块选择。SX1276 有三个现有模块。
 * 请将连接的模块设置为值 1，并将其他模块设置为 0。
 */
#ifdef USE_SX1276_RADIO
#define MODULE_SX1276RF1IAS                         0
#define MODULE_SX1276RF1JAS                         0  //868
#define MODULE_SX1276RF1KAS                         1  //434
#endif

    #include "sx12xxEiger.h"
    #define USE_UART                                0

#elif( PLATFORM == SX12000DVK )
/*!
 * 无线电选择。请取消注释所需的无线电并注释其他选项，
 * 或在编译器定义选项中添加/更改所需的无线电定义。
 */
//#define USE_SX1232_RADIO
#define USE_SX1272_RADIO
//#define USE_SX1276_RADIO
//#define USE_SX1243_RADIO

    #include "sx1200dvk.h"

#elif( PLATFORM == SX1243ska )

#elif( PLATFORM == Bleeper )
    #define USE_SX1272_RADIO
    
    #include "bleeper/bleeper.h"
    #define USE_UART                                0

#else
    #error "缺少定义：平台 (例如 SX12xxEiger)"
#endif

#endif // __PLATFORM_H__
