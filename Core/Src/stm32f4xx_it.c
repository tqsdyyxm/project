/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   中断服务函数：转发给对应外设的 HAL 中断处理
  *
  * SysTick_Handler / PendSV_Handler / SVC_Handler 由 FreeRTOS port.c 实现，
  * 本文件不重复定义。
  ******************************************************************************
  */

#include "main.h"
#include "board.h"

/**
  * @brief  TIM6 中断：HAL 时基（1 kHz）
  */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim_timebase);
}

/**
  * @brief  USART1 中断：Synex 串口收发
  */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart_synex);
}

/**
  * @brief  CAN1 RX FIFO0 中断：3508 电机反馈
  */
void CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan_motor);
}

/**
  * @brief  CAN1 状态/错误中断（用于统计总线错误）
  */
void CAN1_SCE_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan_motor);
}
