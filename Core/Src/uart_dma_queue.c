#include "uart_dma_queue.h"
#include <stdlib.h>
#include <string.h>
#include "stm32f0xx_hal.h"  // 根据你的芯片型号替换

typedef struct {
    uint8_t *data;
    uint16_t length;
} UART_TxItem_t;

typedef struct {
    UART_TxItem_t *items;
    uint16_t capacity;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} UART_TxQueue_t;

#define UART_TX_QUEUE_INIT_CAPACITY 8

static UART_TxQueue_t uartTxQueue = {0};
extern UART_HandleTypeDef huart1;  // 你的UART句柄，确保在main.c或其他文件定义并初始化

static volatile uint8_t uartTxBusy = 0;
static uint8_t *currentTxData = NULL;

// 初始化队列
int UART_TxQueue_Init(void)
{
    uartTxQueue.items = (UART_TxItem_t *)malloc(sizeof(UART_TxItem_t) * UART_TX_QUEUE_INIT_CAPACITY);
    if (uartTxQueue.items == NULL) return -1;
    uartTxQueue.capacity = UART_TX_QUEUE_INIT_CAPACITY;
    uartTxQueue.head = 0;
    uartTxQueue.tail = 0;
    uartTxQueue.count = 0;
    uartTxBusy = 0;
    currentTxData = NULL;
    return 0;
}

// 释放队列内存
void UART_TxQueue_DeInit(void)
{
    // 释放队列中未发送的数据内存
    for (uint16_t i = 0; i < uartTxQueue.count; i++) {
        uint16_t idx = (uartTxQueue.head + i) % uartTxQueue.capacity;
        free(uartTxQueue.items[idx].data);
    }
    free(uartTxQueue.items);
    uartTxQueue.items = NULL;
    uartTxQueue.capacity = 0;
    uartTxQueue.head = 0;
    uartTxQueue.tail = 0;
    uartTxQueue.count = 0;

    // 释放当前发送数据内存（如果有）
    if (currentTxData != NULL) {
        free(currentTxData);
        currentTxData = NULL;
    }
}

// 扩容队列
static int UART_TxQueue_Expand(void)
{
    uint16_t new_capacity = uartTxQueue.capacity * 2;
    UART_TxItem_t *new_items = (UART_TxItem_t *)malloc(sizeof(UART_TxItem_t) * new_capacity);
    if (new_items == NULL) return -1;

    // 重新排列数据，保证head在0位置
    for (uint16_t i = 0; i < uartTxQueue.count; i++) {
        uint16_t idx = (uartTxQueue.head + i) % uartTxQueue.capacity;
        new_items[i] = uartTxQueue.items[idx];
    }

    free(uartTxQueue.items);
    uartTxQueue.items = new_items;
    uartTxQueue.capacity = new_capacity;
    uartTxQueue.head = 0;
    uartTxQueue.tail = uartTxQueue.count;

    return 0;
}

// 入队，拷贝数据到堆内存
int UART_TxQueue_Enqueue(const uint8_t *pData, uint16_t length)
{
    if (length == 0 || pData == NULL) return -2;

    __disable_irq();

    if (uartTxQueue.count == uartTxQueue.capacity) {
        __enable_irq();
        if (UART_TxQueue_Expand() != 0) {
            return -1; // 扩容失败
        }
        __disable_irq();
    }

    uint8_t *pCopy = (uint8_t *)malloc(length);
    if (pCopy == NULL) {
        __enable_irq();
        return -3; // 内存分配失败
    }
    memcpy(pCopy, pData, length);

    uartTxQueue.items[uartTxQueue.tail].data = pCopy;
    uartTxQueue.items[uartTxQueue.tail].length = length;
    uartTxQueue.tail = (uartTxQueue.tail + 1) % uartTxQueue.capacity;
    uartTxQueue.count++;

    __enable_irq();

    UART_TxQueue_Process();

    return 0;
}

// 出队
static int UART_TxQueue_Dequeue(UART_TxItem_t *pItem)
{
    if (uartTxQueue.count == 0) {
        return -1;
    }

    *pItem = uartTxQueue.items[uartTxQueue.head];
    uartTxQueue.head = (uartTxQueue.head + 1) % uartTxQueue.capacity;
    uartTxQueue.count--;

    return 0;
}

// 启动发送
void UART_TxQueue_Process(void)
{
    if (uartTxBusy) return;

    UART_TxItem_t item;
    if (UART_TxQueue_Dequeue(&item) == 0) {
        uartTxBusy = 1;
        currentTxData = item.data;
        HAL_UART_Transmit_DMA(&huart1, currentTxData, item.length);
    }
}

// 发送完成回调，需在HAL_UART_TxCpltCallback中调用
void UART_TxCpltCallback(void)
{
    uartTxBusy = 0;

    if (currentTxData != NULL) {
        free(currentTxData);
        currentTxData = NULL;
    }

    UART_TxQueue_Process();
}
