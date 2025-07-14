#ifndef UART_DMA_QUEUE_H
#define UART_DMA_QUEUE_H

#include <stdint.h>

int UART_TxQueue_Init(void);
void UART_TxQueue_DeInit(void);
int UART_TxQueue_Enqueue(const uint8_t *pData, uint16_t length);
void UART_TxQueue_Process(void);
void UART_TxCpltCallback(void);

#endif
