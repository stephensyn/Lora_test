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
 * \file       sx1276-LoRa.c
 * \brief      SX1276 RF chip driver mode LoRa
 *
 * \version    2.0.0
 * \date       May 6 2013
 * \author     Gregory Cristian
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include <string.h>

#include "platform.h"

#if defined(USE_SX1276_RADIO)

#include "radio.h"
#include "sx1276-Hal.h"
#include "sx1276.h"
#include "sx1276-LoRaMisc.h"
#include "sx1276-LoRa.h"
#include "Delay.h"

#include "usart.h"

/*!
 * Constant values need to compute the RSSI value
 */
#define RSSI_OFFSET_LF -164.0
#define RSSI_OFFSET_HF -157.0

/*!
 * Frequency hopping frequencies table
 */
const int32_t HoppingFrequencies[] =
    {
        916500000,
        923500000,
        906500000,
        917500000,
        917500000,
        909000000,
        903000000,
        916000000,
        912500000,
        926000000,
        925000000,
        909500000,
        913000000,
        918500000,
        918500000,
        902500000,
        911500000,
        926500000,
        902500000,
        922000000,
        924000000,
        903500000,
        913000000,
        922000000,
        926000000,
        910000000,
        920000000,
        922500000,
        911000000,
        922000000,
        909500000,
        926000000,
        922000000,
        918000000,
        925500000,
        908000000,
        917500000,
        926500000,
        908500000,
        916000000,
        905500000,
        916000000,
        903000000,
        905000000,
        915000000,
        913000000,
        907000000,
        910000000,
        926500000,
        925500000,
        911000000,
};

// Default settings
tLoRaSettings LoRaSettings =
    {
        434000000, // RFFrequency
        0,         // Power
        8,         // SignalBw [0: 7.8kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,
                   // 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved]
        10,        // SpreadingFactor [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
        2,         // ErrorCoding [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
        true,      // CrcOn [0: OFF, 1: ON]
        true,      // ImplicitHeaderOn [0: OFF, 1: ON]
        1,         // RxSingleOn [0: Continuous, 1 Single]
        0,         // FreqHopOn [0: OFF, 1: ON]
        4,         // HopPeriod Hops every frequency hopping period symbols
        1000,      // TxPacketTimeout
        500,       // RxPacketTimeout
        10,        // PayloadLength (used for implicit header mode)
        0x03,      // SyncWord (LORA [0x34: 868 MHz, 0x12: 433 MHz, 0x18: 780 MHz, 0x24: 470 MHz, 0x34: 868 MHz])
};

/*!
 * SX1276 LoRa registers variable
 */
tSX1276LR *SX1276LR;

/*!
 * Local RF buffer for communication support
 */
static uint8_t RFBuffer[RF_BUFFER_SIZE];

/*!
 * RF state machine variable
 */
uint8_t RFLRState = RFLR_STATE_IDLE;

/*!
 * Rx management support variables
 */
static uint16_t RxPacketSize = 0;
static int8_t RxPacketSnrEstimate;
static double RxPacketRssiValue;
static uint8_t RxGain = 1;
static uint32_t RxTimeoutTimer = 0;
/*!
 * PacketTimeout Stores the Rx window time value for packet reception
 */
static uint32_t PacketTimeout;

/*!
 * Tx management support variables
 */
static uint16_t TxPacketSize = 0;

void SX1276LoRaInit(void)
{
    RFLRState = RFLR_STATE_IDLE; // LoRa状态机初始化为IDLE状态
    SX1276LoRaSetDefaults();
    SX1276ReadBuffer(REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1);

    // set the RF settings
    SX1276LR->RegLna = RFLR_LNA_GAIN_G1;

    SX1276WriteBuffer(REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1);

    SX1276LoRaSetRFFrequency(LoRaSettings.RFFrequency);
    SX1276LoRaSetSpreadingFactor(LoRaSettings.SpreadingFactor); // SF6 only operates in implicit header mode.
    SX1276LoRaSetErrorCoding(LoRaSettings.ErrorCoding);
    SX1276LoRaSetPacketCrcOn(LoRaSettings.CrcOn);
    SX1276LoRaSetSignalBandwidth(LoRaSettings.SignalBw);

    SX1276LoRaSetSyncWord(LoRaSettings.SyncWordValue);

    SX1276LoRaSetImplicitHeaderOn(LoRaSettings.ImplicitHeaderOn);
    SX1276LoRaSetSymbTimeout(0x3FF);
    SX1276LoRaSetPayloadLength(LoRaSettings.PayloadLength);
    SX1276LoRaSetLowDatarateOptimize(true);

#if ((MODULE_SX1276RF1IAS == 1) || (MODULE_SX1276RF1KAS == 1))
    if (LoRaSettings.RFFrequency > 860000000)
    {
        SX1276LoRaSetPAOutput(RFLR_PACONFIG_PASELECT_RFO);
        SX1276LoRaSetPa20dBm(false);
        LoRaSettings.Power = 14;
        SX1276LoRaSetRFPower(LoRaSettings.Power);
    }
    else
    {
        SX1276LoRaSetPAOutput(RFLR_PACONFIG_PASELECT_PABOOST);
        SX1276LoRaSetPa20dBm(true);
        LoRaSettings.Power = 20;
        SX1276LoRaSetRFPower(LoRaSettings.Power);
    }
#elif (MODULE_SX1276RF1JAS == 1)
    if (LoRaSettings.RFFrequency > 860000000)
    {
        SX1276LoRaSetPAOutput(RFLR_PACONFIG_PASELECT_PABOOST);
        SX1276LoRaSetPa20dBm(true);
        LoRaSettings.Power = 20;
        SX1276LoRaSetRFPower(LoRaSettings.Power);
    }
    else
    {
        SX1276LoRaSetPAOutput(RFLR_PACONFIG_PASELECT_RFO);
        SX1276LoRaSetPa20dBm(false);
        LoRaSettings.Power = 14;
        SX1276LoRaSetRFPower(LoRaSettings.Power);
    }
#endif

    SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
}

void SX1276LoRaSetDefaults(void)
{
    // REMARK: See SX1276 datasheet for modified default values.

    SX1276Read(REG_LR_VERSION, &SX1276LR->RegVersion);
}

void SX1276LoRaReset(void)
{
    uint32_t startTick;

    SX1276SetReset(RADIO_RESET_ON);

    // Wait 1ms
    startTick = GET_TICK_COUNT();
    while ((GET_TICK_COUNT() - startTick) < TICK_RATE_MS(200))
        ;

    SX1276SetReset(RADIO_RESET_OFF);

    // Wait 6ms
    startTick = GET_TICK_COUNT();
    while ((GET_TICK_COUNT() - startTick) < TICK_RATE_MS(20))
        ;
}

void SX1276LoRaSetOpMode(uint8_t opMode)
{
    static uint8_t opModePrev = RFLR_OPMODE_STANDBY;
    static bool antennaSwitchTxOnPrev = true;
    bool antennaSwitchTxOn = false;

    opModePrev = SX1276LR->RegOpMode & ~RFLR_OPMODE_MASK;

    if (opMode != opModePrev)
    {
        if (opMode == RFLR_OPMODE_TRANSMITTER)
        {
            antennaSwitchTxOn = true;
        }
        else
        {
            antennaSwitchTxOn = false;
        }
        if (antennaSwitchTxOn != antennaSwitchTxOnPrev)
        {
            antennaSwitchTxOnPrev = antennaSwitchTxOn;
            RXTX(antennaSwitchTxOn); // Antenna switch control
        }
        SX1276LR->RegOpMode = (SX1276LR->RegOpMode & RFLR_OPMODE_MASK) | opMode;

        SX1276Write(REG_LR_OPMODE, SX1276LR->RegOpMode);
    }
}

uint8_t SX1276LoRaGetOpMode(void)
{
    SX1276Read(REG_LR_OPMODE, &SX1276LR->RegOpMode);

    return SX1276LR->RegOpMode & ~RFLR_OPMODE_MASK;
}

uint8_t SX1276LoRaReadRxGain(void)
{
    SX1276Read(REG_LR_LNA, &SX1276LR->RegLna);
    return (SX1276LR->RegLna >> 5) & 0x07;
}

double SX1276LoRaReadRssi(void)
{
    // Reads the RSSI value
    SX1276Read(REG_LR_RSSIVALUE, &SX1276LR->RegRssiValue);

    if (LoRaSettings.RFFrequency < 860000000) // LF
    {
        return RSSI_OFFSET_LF + (double)SX1276LR->RegRssiValue;
    }
    else
    {
        return RSSI_OFFSET_HF + (double)SX1276LR->RegRssiValue;
    }
}

uint8_t SX1276LoRaGetPacketRxGain(void)
{
    return RxGain;
}

int8_t SX1276LoRaGetPacketSnr(void)
{
    return RxPacketSnrEstimate;
}

double SX1276LoRaGetPacketRssi(void)
{
    return RxPacketRssiValue;
}

void SX1276LoRaStartRx(void)
{
    SX1276LoRaSetRFState(RFLR_STATE_RX_INIT);
}

// 获取接收到的数据包
void SX1276LoRaGetRxPacket(void *buffer, uint16_t *size)
{
    // 如果接收到的数据包大小超过缓冲区大小，则重置为0
    if (RxPacketSize > RF_BUFFER_SIZE)
    {
        RxPacketSize = 0;
    }
    // 将接收到的数据包大小赋值给输出参数
    *size = RxPacketSize;
    // 重置接收到的数据包大小
    RxPacketSize = 0;
    // 将接收到的数据从本地缓冲区复制到输出缓冲区
    memcpy((void *)buffer, (void *)RFBuffer, (size_t)*size);
}

void SX1276LoRaSetTxPacket(const void *buffer, uint16_t size)
{
    TxPacketSize = size;
    memcpy((void *)RFBuffer, buffer, (size_t)TxPacketSize); //

    RFLRState = RFLR_STATE_TX_INIT;
}

uint8_t SX1276LoRaGetRFState(void)
{
    return RFLRState;
}

void SX1276LoRaSetRFState(uint8_t state)
{
    RFLRState = state;
}

/*!
 * \brief Process the LoRa modem Rx and Tx state machines depending on the
 *        SX1276 operating mode.
 *
 * \retval rfState Current RF state [RF_IDLE, RF_BUSY,
 *                                   RF_RX_DONE, RF_RX_TIMEOUT,
 *                                   RF_TX_DONE, RF_TX_TIMEOUT]
 */
uint32_t SX1276LoRaProcess(void)
{
    uint32_t result = RF_BUSY;

    switch (RFLRState)
    {
    case RFLR_STATE_IDLE:

        SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);
        return result = RF_IDLE; // LoRa状态机进入空闲状态

        break;
    case RFLR_STATE_RX_INIT:

        SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY); // 设置LoRa为待机模式

        SX1276LR->RegIrqFlagsMask =
            // RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
            // RFLR_IRQFLAGS_RXDONE |
            // RFLR_IRQFLAGS_PAYLOADCRCERROR |
            // 以上原本是注释掉了的
            RFLR_IRQFLAGS_RXTIMEOUT |
            RFLR_IRQFLAGS_VALIDHEADER |
            RFLR_IRQFLAGS_TXDONE |
            RFLR_IRQFLAGS_CADDONE |
            RFLR_IRQFLAGS_CADDETECTED;

        SX1276Write(REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask);
        if (LoRaSettings.FreqHopOn == true)
        {
            SX1276LR->RegHopPeriod = LoRaSettings.HopPeriod;

            SX1276Read(REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel);
            SX1276LoRaSetRFFrequency(HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK]);
        }
        else
        {
            SX1276LR->RegHopPeriod = 255;
        }

        SX1276Write(REG_LR_HOPPERIOD, SX1276LR->RegHopPeriod);

        // RxDone                    RxTimeout                   FhssChangeChannel           CadDone
        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
        // PllLock              ModeReady
        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_01 | RFLR_DIOMAPPING2_DIO5_00;
        SX1276WriteBuffer(REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2);
        if (LoRaSettings.RxSingleOn == true) // Rx single mode
        {

            SX1276LoRaSetOpMode(RFLR_OPMODE_RECEIVER_SINGLE);
        }
        else // Rx continuous mode
        {
            SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxBaseAddr;
            SX1276Write(REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr);

            SX1276LoRaSetOpMode(RFLR_OPMODE_RECEIVER);
        }

        memset(RFBuffer, 0, (size_t)RF_BUFFER_SIZE);

        PacketTimeout = LoRaSettings.RxPacketTimeout;
        RxTimeoutTimer = GET_TICK_COUNT();
        RFLRState = RFLR_STATE_RX_RUNNING;
        break;
    case RFLR_STATE_RX_RUNNING:

        if (DIO0 == 1) // RxDone
        {
            RxTimeoutTimer = GET_TICK_COUNT();
            if (LoRaSettings.FreqHopOn == true)
            {
                SX1276Read(REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel);
                SX1276LoRaSetRFFrequency(HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK]);
            }
            // Clear Irq
            SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXDONE);
            RFLRState = RFLR_STATE_RX_DONE;
        }
        if (DIO2 == 1) // FHSS Changed Channel
        {
            RxTimeoutTimer = GET_TICK_COUNT();
            if (LoRaSettings.FreqHopOn == true)
            {
                SX1276Read(REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel);
                SX1276LoRaSetRFFrequency(HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK]);
            }
            // Clear Irq
            SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL);
            // Debug
            RxGain = SX1276LoRaReadRxGain();
        }

        if (LoRaSettings.RxSingleOn == true) // Rx single mode
        {
            if ((GET_TICK_COUNT() - RxTimeoutTimer) > PacketTimeout)
            {
                RFLRState = RFLR_STATE_RX_TIMEOUT; // 状态机进入超时状态
            }
        }
        break;
    case RFLR_STATE_RX_DONE:
        SX1276Read(REG_LR_IRQFLAGS, &SX1276LR->RegIrqFlags);
        if ((SX1276LR->RegIrqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR) == RFLR_IRQFLAGS_PAYLOADCRCERROR)
        {
            // Clear Irq
            SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_PAYLOADCRCERROR);

            if (LoRaSettings.RxSingleOn == true) // Rx single mode
            {
                RFLRState = RFLR_STATE_RX_INIT;
            }
            else
            {
                RFLRState = RFLR_STATE_RX_RUNNING;
            }
            break;
        }

        {
            uint8_t rxSnrEstimate;
            SX1276Read(REG_LR_PKTSNRVALUE, &rxSnrEstimate);
            if (rxSnrEstimate & 0x80) // The SNR sign bit is 1
            {
                // Invert and divide by 4
                RxPacketSnrEstimate = ((~rxSnrEstimate + 1) & 0xFF) >> 2;
                RxPacketSnrEstimate = -RxPacketSnrEstimate;
            }
            else
            {
                // Divide by 4
                RxPacketSnrEstimate = (rxSnrEstimate & 0xFF) >> 2;
            }
        }

        SX1276Read(REG_LR_PKTRSSIVALUE, &SX1276LR->RegPktRssiValue);

        if (LoRaSettings.RFFrequency < 860000000) // LF
        {
            if (RxPacketSnrEstimate < 0)
            {
                RxPacketRssiValue = RSSI_OFFSET_LF + ((double)SX1276LR->RegPktRssiValue) + RxPacketSnrEstimate;
            }
            else
            {
                RxPacketRssiValue = RSSI_OFFSET_LF + (1.0666 * ((double)SX1276LR->RegPktRssiValue));
            }
        }
        else // HF
        {
            if (RxPacketSnrEstimate < 0)
            {
                RxPacketRssiValue = RSSI_OFFSET_HF + ((double)SX1276LR->RegPktRssiValue) + RxPacketSnrEstimate;
            }
            else
            {
                RxPacketRssiValue = RSSI_OFFSET_HF + (1.0666 * ((double)SX1276LR->RegPktRssiValue));
            }
        }

        if (LoRaSettings.RxSingleOn == true) // Rx single mode
        {
            SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxBaseAddr;
            SX1276Write(REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr);

            if (LoRaSettings.ImplicitHeaderOn == true)
            {
                RxPacketSize = SX1276LR->RegPayloadLength;
                SX1276ReadFifo(RFBuffer, SX1276LR->RegPayloadLength);
            }
            else
            {
                SX1276Read(REG_LR_NBRXBYTES, &SX1276LR->RegNbRxBytes);
                RxPacketSize = SX1276LR->RegNbRxBytes;
                SX1276ReadFifo(RFBuffer, SX1276LR->RegNbRxBytes);
            }
        }
        else // Rx continuous mode
        {
            SX1276Read(REG_LR_FIFORXCURRENTADDR, &SX1276LR->RegFifoRxCurrentAddr);

            if (LoRaSettings.ImplicitHeaderOn == true)
            {
                RxPacketSize = SX1276LR->RegPayloadLength;
                SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxCurrentAddr;
                SX1276Write(REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr);
                SX1276ReadFifo(RFBuffer, SX1276LR->RegPayloadLength);
            }
            else
            {
                SX1276Read(REG_LR_NBRXBYTES, &SX1276LR->RegNbRxBytes);
                RxPacketSize = SX1276LR->RegNbRxBytes;
                SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxCurrentAddr;
                SX1276Write(REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr);
                SX1276ReadFifo(RFBuffer, SX1276LR->RegNbRxBytes);
            }
        }

        if (LoRaSettings.RxSingleOn == true) // Rx single mode
        {
            RFLRState = RFLR_STATE_RX_INIT;
        }
        else // Rx continuous mode
        {
            RFLRState = RFLR_STATE_RX_RUNNING;
        }
        result = RF_RX_DONE;
        break;
    case RFLR_STATE_RX_TIMEOUT:
        // RFLRState = RFLR_STATE_RX_INIT;
        result = RF_RX_TIMEOUT;
        break;
    case RFLR_STATE_TX_INIT:

        // 设置为待机模式
        SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

        if (LoRaSettings.FreqHopOn == true)
        {
            // 配置中断掩码，启用频率跳变相关中断
            SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT |
                                        RFLR_IRQFLAGS_RXDONE |
                                        RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                        RFLR_IRQFLAGS_VALIDHEADER |
                                        // RFLR_IRQFLAGS_TXDONE |
                                        RFLR_IRQFLAGS_CADDONE |
                                        // RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                        RFLR_IRQFLAGS_CADDETECTED;
            // 设置跳频周期
            SX1276LR->RegHopPeriod = LoRaSettings.HopPeriod;

            // 读取当前跳频信道并设置频率
            SX1276Read(REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel);
            SX1276LoRaSetRFFrequency(HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK]);
        }
        else
        {
            // 配置中断掩码，禁用频率跳变相关中断
            SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT |
                                        RFLR_IRQFLAGS_RXDONE |
                                        RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                        RFLR_IRQFLAGS_VALIDHEADER |
                                        // RFLR_IRQFLAGS_TXDONE |
                                        RFLR_IRQFLAGS_CADDONE |
                                        RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                        RFLR_IRQFLAGS_CADDETECTED;
            // 禁用跳频
            SX1276LR->RegHopPeriod = 0;
        }
        // 写入跳频周期和中断掩码
        SX1276Write(REG_LR_HOPPERIOD, SX1276LR->RegHopPeriod);
        SX1276Write(REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask);

        // 初始化发送数据包大小
        SX1276LR->RegPayloadLength = TxPacketSize;
        SX1276Write(REG_LR_PAYLOADLENGTH, SX1276LR->RegPayloadLength);

        // 设置FIFO发送缓冲区基地址
        SX1276LR->RegFifoTxBaseAddr = 0x00; // 使用整个缓冲区进行发送
        SX1276Write(REG_LR_FIFOTXBASEADDR, SX1276LR->RegFifoTxBaseAddr);

        // 设置FIFO地址指针为发送缓冲区基地址
        SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoTxBaseAddr;
        SX1276Write(REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr);

        // 将数据写入LORA调制解调器的FIFO缓冲区
        SX1276WriteFifo(RFBuffer, SX1276LR->RegPayloadLength);

        // 配置DIO引脚映射，用于发送完成、接收超时等中断
        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_01 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_01;
        // 配置DIO引脚映射，用于CAD检测和模式准备
        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
        SX1276WriteBuffer(REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2);

        // 设置为发送模式
        SX1276LoRaSetOpMode(RFLR_OPMODE_TRANSMITTER);

        // 更新状态为发送运行中
        RFLRState = RFLR_STATE_TX_RUNNING;
        break;
    case RFLR_STATE_TX_RUNNING:
        //
        if (DIO0 == 1) // TxDone
        {
            HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"TX Running\r", 12); // 发送数据包
            // Clear Irq
            SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE);
            RFLRState = RFLR_STATE_TX_DONE;
        }
        if (DIO2 == 1) // FHSS Changed Channel
        {
            if (LoRaSettings.FreqHopOn == true)
            {
                SX1276Read(REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel);
                SX1276LoRaSetRFFrequency(HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK]);
            }
            // Clear Irq
            SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL);
        }
        break;
    case RFLR_STATE_TX_DONE:
        // optimize the power consumption by switching off the transmitter as soon as the packet has been sent
        SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

        RFLRState = RFLR_STATE_IDLE;
        result = RF_TX_DONE;
        break;

    case RFLR_STATE_CAD_INIT:
        // 设置为待机模式
        SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

        // 配置中断掩码，禁用CAD完成和CAD检测到的中断
        SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT |
                                    RFLR_IRQFLAGS_RXDONE |
                                    RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                    RFLR_IRQFLAGS_VALIDHEADER |
                                    RFLR_IRQFLAGS_TXDONE |
                                    // RFLR_IRQFLAGS_CADDONE |
                                    RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL; // |
                                                                      // RFLR_IRQFLAGS_CADDETECTED;
        SX1276Write(REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask);

        // 配置DIO引脚映射，用于RxDone、CAD检测、FHSS信道切换和CAD完成中断
        // RxDone                       CAD Detected               FhssChangeChannel           CadDone
        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_10 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
        // 配置DIO引脚映射，用于PllLock和模式准备
        // PllLock               ModeReady
        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_01 | RFLR_DIOMAPPING2_DIO5_00;
        SX1276WriteBuffer(REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2);

        // 设置为CAD模式
        SX1276LoRaSetOpMode(RFLR_OPMODE_CAD);
        // 更新状态为CAD运行中
        RFLRState = RFLR_STATE_CAD_RUNNING;
        break;

    case RFLR_STATE_CAD_RUNNING:
        if (DIO3 == 1) // CAD完成中断
        {
            // 清除中断标志
            SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDONE);
            if (DIO1 == 1) // CAD检测到中断
            {
                // 清除中断标志
                SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDETECTED);
                // CAD检测到，表示存在LoRa前导码
                // RFLRState = RFLR_STATE_RX_INIT;                                        // 监测到有信号后，进入接收模式
                result = RF_CHANNEL_ACTIVITY_DETECTED;                                 // 检测到信道活动
                                                                                       // HAL_UART_Transmit(&huart1, (uint8_t *)"\r\nCAD Detected\r\n", 14, 1000); // 调试输出
                HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"CAD Detected\r", 14); // 发送数据包
            }
            else
            {
                // 设备会自动进入待机模式
                // RFLRState = RFLR_STATE_IDLE;

                // RFLRState = RFLR_STATE_CAD_INIT;

                // HAL_UART_Transmit(&huart1, (uint8_t *)"\r\nCAD NOT Detected\r\n", 14, 1000); // 调试输出
                HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"CAD NOT Detected\r", 18); // 发送数据包
                result = RF_CHANNEL_EMPTY;
                // HAL_Delay(1000); // 信道空闲
            }
        }
        break;

    default:
        break;
    }
    return result;
}

void SX1276LoRaEnterCadMode(uint8_t state)
{
    RFLRState = state;
}

void SX1276LoRaParameterChange(uint8_t syncword)
{
    // 重新设置LoRa参数
    SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

    // SX1276LoRaSetRFFrequency(LoRaSettings.RFFrequency);           // 设置射频频率
    // SX1276LoRaSetSpreadingFactor(LoRaSettings.SpreadingFactor);   // 设置扩频因子
    // SX1276LoRaSetErrorCoding(LoRaSettings.ErrorCoding);           // 设置纠错编码
    // SX1276LoRaSetPacketCrcOn(LoRaSettings.CrcOn);                 // 设置CRC校验开关
    // SX1276LoRaSetSignalBandwidth(LoRaSettings.SignalBw);          // 设置信号带宽
    SX1276LoRaSetSyncWord(syncword); // 设置同步字

    // SX1276LoRaSetImplicitHeaderOn(LoRaSettings.ImplicitHeaderOn); // 设置隐式头模式开关
    // SX1276LoRaSetSymbTimeout(0x3FF);                              // 设置符号超时时间
    // SX1276LoRaSetPayloadLength(LoRaSettings.PayloadLength);       // 设置有效载荷长度

    RFLRState = RFLR_STATE_CAD_INIT; // process()函数首先会判断RFLRState的状态，然后执行相关操作。
}

#endif // USE_SX1276_RADIO
