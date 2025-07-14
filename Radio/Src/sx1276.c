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
 * \file       sx1276.c
 * \brief      SX1276 RF chip driver
 *
 * \version    2.0.0
 * \date       May 6 2013
 * \author     Gregory Cristian
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include "platform.h"
#include "radio.h"
#include "string.h"

#if defined(USE_SX1276_RADIO)

#include "sx1276.h"

#include "sx1276-Hal.h"
#include "sx1276-Fsk.h"
#include "sx1276-LoRa.h"
#include "sx1276-FskMisc.h"
#include "sx1276-LoRamisc.h"
#include "usart.h"
/*!
 * SX1276 registers variable
 */
uint8_t SX1276Regs[0x70];

static bool LoRaOn = false;
static bool LoRaOnState = false;

extern tFskSettings FskSettings;

void SX1276Init(void)
{
    // static uint8_t spi_test=0;

    // Initialize FSK and LoRa registers structure
    SX1276 = (tSX1276 *)SX1276Regs;
    SX1276LR = (tSX1276LR *)SX1276Regs;

    memset(SX1276, (int)0, sizeof(tSX1276));
    memset(SX1276LR, (int)0, sizeof(tSX1276LR));

    SX1276Reset();

    SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

    // REMARK: After radio reset the default modem is FSK

#if (LORA == 0)

    LoRaOn = false;
    SX1276SetLoRaOn(LoRaOn);
    // Initialize FSK modem
    SX1276FskInit();

#else

    LoRaOn = true;
    LoRaOnState = false;
    SX1276SetLoRaOn(LoRaOn);// 设置LoRa模式开关
    // Initialize LoRa modem
    SX1276LoRaInit();

#endif

    // SX1276LoRaSetPreambleLength(100);
}

void EnterRxTestMode()
{
    uint8_t PackgeConfigration = 0;

    SX1276FskSetOpMode(RF_OPMODE_STANDBY);

    SX1276Read(REG_PACKETCONFIG2, &PackgeConfigration);
    PackgeConfigration = PackgeConfigration & 0xBF;
    SX1276Write(REG_PACKETCONFIG2, PackgeConfigration);
    SX1276Read(REG_PACKETCONFIG2, &PackgeConfigration);
    //       SyncAddress             Dclk                      Data                    Timeout
    SX1276->RegDioMapping1 = RF_DIOMAPPING1_DIO0_00 | RF_DIOMAPPING1_DIO1_00 | RF_DIOMAPPING1_DIO2_11 | RF_DIOMAPPING1_DIO3_00;
    SX1276->RegDioMapping2 = RF_DIOMAPPING2_DIO4_11 | RF_DIOMAPPING2_DIO5_10 | RF_DIOMAPPING2_MAP_PREAMBLEDETECT;
    SX1276WriteBuffer(REG_DIOMAPPING1, &SX1276->RegDioMapping1, 2);

    SX1276FskSetOpMode(RF_OPMODE_RECEIVER);
}


void EnterTestMode()
{

    SX1276FskSetRFFrequency(FskSettings.RFFrequency);
    SX1276FskSetFdev(FskSettings.Fdev);

    SX1276FskSetPAOutput(RF_PACONFIG_PASELECT_PABOOST);
    SX1276FskSetPa20dBm(true);
    FskSettings.Power = 20;
    SX1276FskSetRFPower(FskSettings.Power);

    SX1276FskSetOpMode(RF_OPMODE_TRANSMITTER);
}

void SX1276Reset(void)
{
    uint32_t startTick;
    SX1276SetReset(RADIO_RESET_ON);

    // Wait 1ms
    // SX1276手册手动复位：RST拉低100ms以上，拉高后等待6ms
    startTick = GET_TICK_COUNT();
    while ((GET_TICK_COUNT() - startTick) < TICK_RATE_MS(200))
        ;

    SX1276SetReset(RADIO_RESET_OFF);

    // Wait 6ms
    startTick = GET_TICK_COUNT();
    while ((GET_TICK_COUNT() - startTick) < TICK_RATE_MS(20))
        ;
}

void SX1276SetLoRaOn(bool enable)
{
    // uint8_t ModeTest=0;
    // static uint8_t test=0;

    if (LoRaOnState == enable)
    {
        return;
    }
    LoRaOnState = enable;
    LoRaOn = enable;

    if (LoRaOn == true)
    {
        SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);

        SX1276LR->RegOpMode = (SX1276LR->RegOpMode & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_ON;
        SX1276Write(REG_LR_OPMODE, SX1276LR->RegOpMode);

        SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

        //        SX1276Write( REG_LR_FRFMSB, 0x86 );
        //        SX1276ReadBuffer( REG_LR_FRFMSB, &test, 1);

        // RxDone               RxTimeout                   FhssChangeChannel           CadDone
        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
        // CadDetected          ModeReady
        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
        SX1276WriteBuffer(REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2);

        SX1276ReadBuffer(REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1);
    }
    else
    {
        SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);

        SX1276LR->RegOpMode = (SX1276LR->RegOpMode & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_OFF;
        SX1276Write(REG_LR_OPMODE, SX1276LR->RegOpMode);

        SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

        SX1276ReadBuffer(REG_OPMODE, SX1276Regs + 1, 0x70 - 1);
    }
}

bool SX1276GetLoRaOn(void)
{
    return LoRaOn;
}

void SX1276SetOpMode(uint8_t opMode)
{
    if (LoRaOn == false)
    {
        SX1276FskSetOpMode(opMode);
    }
    else
    {
        SX1276LoRaSetOpMode(opMode);
    }
}

uint8_t SX1276GetOpMode(void)
{
    if (LoRaOn == false)
    {
        return SX1276FskGetOpMode();
    }
    else
    {
        return SX1276LoRaGetOpMode();
    }
}

double SX1276ReadRssi(void)
{
    if (LoRaOn == false)
    {
        return SX1276FskReadRssi();
    }
    else
    {
        return SX1276LoRaReadRssi();
    }
}

uint8_t SX1276ReadRxGain(void)
{
    if (LoRaOn == false)
    {
        return SX1276FskReadRxGain();
    }
    else
    {
        return SX1276LoRaReadRxGain();
    }
}

uint8_t SX1276GetPacketRxGain(void)
{
    if (LoRaOn == false)
    {
        return SX1276FskGetPacketRxGain();
    }
    else
    {
        return SX1276LoRaGetPacketRxGain();
    }
}

int8_t SX1276GetPacketSnr(void)
{
    if (LoRaOn == false)
    {
        while (1)
        {
            // Useless in FSK mode
            // Block program here
        }
    }
    else
    {
        return SX1276LoRaGetPacketSnr();
    }
}

double SX1276GetPacketRssi(void)
{
    if (LoRaOn == false)
    {
        return SX1276FskGetPacketRssi();
    }
    else
    {
        return SX1276LoRaGetPacketRssi();
    }
}

uint32_t SX1276GetPacketAfc(void)
{
    if (LoRaOn == false)
    {
        return SX1276FskGetPacketAfc();
    }
    else
    {
        while (1)
        {
            // Useless in LoRa mode
            // Block program here
        }
    }
}

void SX1276StartRx(void)
{
    if (LoRaOn == false)
    {
        SX1276FskSetRFState(RF_STATE_RX_INIT);
    }
    else
    {
        SX1276LoRaSetRFState(RFLR_STATE_RX_INIT);
    }
}

void SX1276StartTx(void) // Add by Andrew
{
    if (LoRaOn == false)
    {
        SX1276FskSetRFState(RF_STATE_TX_INIT);
    }
    else
    {
        SX1276LoRaSetRFState(RFLR_STATE_TX_INIT);
    }
}

void SX1276GetRxPacket(void *buffer, uint16_t *size)
{
    if (LoRaOn == false)
    {
        SX1276FskGetRxPacket(buffer, size);
    }
    else
    {
        SX1276LoRaGetRxPacket(buffer, size);
    }
}

void SX1276SetTxPacket(const void *buffer, uint16_t size)
{
    if (LoRaOn == false)
    {
        SX1276FskSetTxPacket(buffer, size);
    }
    else
    {
        SX1276LoRaSetTxPacket(buffer, size);
    }
}

uint8_t SX1276GetRFState(void)
{
    if (LoRaOn == false)
    {
        return SX1276FskGetRFState();
    }
    else
    {
        return SX1276LoRaGetRFState();
    }
}

void SX1276SetRFState(uint8_t state)
{
    if (LoRaOn == false)
    {
        SX1276FskSetRFState(state);
    }
    else
    {
        SX1276LoRaSetRFState(state);
    }
}

uint32_t SX1276Process(void)
{
    if (LoRaOn == false)
    {
        return SX1276FskProcess();
    }
    else
    {
        return SX1276LoRaProcess();
    }
}


void SX1276EnterCadMode(void)
{
    if (LoRaOn == false)
    {
        //SX1276FskEnterCadMode(void);// FSK模式下的载波侦听?
 
    }
    else
    {
        SX1276LoRaEnterCadMode(RFLR_STATE_CAD_INIT);// LoRa模式下的载波侦听
    }
}

void SX1276ParameterChange(uint8_t syncword)
{
    if (LoRaOn == false)
    {
        // FSK模式下的参数变更
       // SX1276FskParameterChange();
    }
    else
    {
        // LoRa模式下的参数变更
        SX1276LoRaParameterChange(syncword);
    }
}

#endif // USE_SX1276_RADIO
