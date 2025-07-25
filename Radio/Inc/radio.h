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
 * \file       radio.h
 * \brief      Generic radio driver ( radio abstraction )
 *
 * \version    2.0.B2
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Gregory Cristian on Apr 25 2013
 */
#ifndef __RADIO_H__
#define __RADIO_H__

/*!
 * SX1272 and SX1276 General parameters definition
 */
#define LORA 1 // [0: OFF, 1: ON]

/*!
 * RF process function return codes
 */
typedef enum
{
    RF_IDLE,                      // 空闲状态
    RF_BUSY,                      // 繁忙状态
    RF_RX_DONE,                   // 接收完成
    RF_RX_TIMEOUT,                // 接收超时
    RF_TX_DONE,                   // 发送完成
    RF_TX_TIMEOUT,                // 发送超时
    RF_LEN_ERROR,                 // 数据长度错误
    RF_CHANNEL_EMPTY,             // 信道空闲
    RF_CHANNEL_ACTIVITY_DETECTED, // 检测到信道活动
} tRFProcessReturnCodes;

/*!
 * Radio driver structure defining the different function pointers
 */
typedef struct sRadioDriver
{
    void (*Init)(void);
    void (*Reset)(void);
    void (*StartRx)(void);
    void (*StartTx)(void);
    void (*GetRxPacket)(void *buffer, uint16_t *size);
    void (*SetTxPacket)(const void *buffer, uint16_t size);
    uint32_t (*Process)(void);
    void (*EnterCadMode)(void);
    void (*parameterChange)(uint8_t syncword); // 用于参数修改
} tRadioDriver;

extern tRadioDriver *Radio;
/*!
 * \brief Initializes the RadioDriver structure with specific radio
 *        functions.
 *
 * \retval radioDriver Pointer to the radio driver vauriable
 */
tRadioDriver *RadioDriverInit(void);

#endif // __RADIO_H__
