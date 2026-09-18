/**
  ******************************************************************************
  * @file    stm32f4xx_it.h
  * @brief   中断服务函数声明
  *
  * 注意：SysTick_Handler / PendSV_Handler / SVC_Handler 由 FreeRTOS 内核
  * （port.c）定义，本文件不声明，stm32f4xx_it.c 也不实现。
  ******************************************************************************
  */

#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

void TIM6_DAC_IRQHandler(void);   /* HAL 时基（1 kHz） */
void USART1_IRQHandler(void);     /* Synex 串口收发 */
void CAN1_RX0_IRQHandler(void);   /* 电机反馈接收 */
void CAN1_SCE_IRQHandler(void);   /* CAN 状态变化/错误 */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_IT_H */
