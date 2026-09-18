/**
  ******************************************************************************
  * @file    bsp_uart.h
  * @brief   串口板级驱动（任务四：Synex / JustFloat）
  *
  * 硬件：USART1，PA9(TX) / PB7(RX)，115200 8N1。
  *       TX/RX 均使用中断 + 环形缓冲，不阻塞任务执行。
  ******************************************************************************
  */

#ifndef __BSP_UART_H
#define __BSP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"

#define UART_BAUDRATE    115200U

void BSP_Uart_Init(void);

/* ---- 发送 ---- */
void BSP_Uart_Send(const uint8_t *data, uint32_t len);
void BSP_Uart_SendString(const char *str);

/* ---- 接收（供协议层逐字节取走）---- */
uint32_t BSP_Uart_RxAvailable(void);
int32_t  BSP_Uart_ReadByte(void);   /* 无数据时返回 -1 */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_UART_H */
