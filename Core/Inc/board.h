/**
  ******************************************************************************
  * @file    board.h
  * @brief   开发板级定义：系统时钟、关键宏、外设句柄声明
  *
  * 硬件对应关系（以《硬件配置.md》为准）：
  *   蜂鸣器  TIM4_CH3  PD14   RGB LED  TIM5_CH1/2/3 PH10/PH11/PH12
  *   舵机    TIM1_CH1  PE9    串口     USART1  PA9(TX)/PB7(RX)
  *   电机    CAN1      PD0(RX)/PD1(TX)  1 Mbps
  ******************************************************************************
  */

#ifndef __BOARD_H
#define __BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/*----------------------------- 系统时钟 -----------------------------*/
#define SYS_CLK_HZ         168000000U  /* 系统主频 */
#define APB1_TIM_CLK_HZ    84000000U   /* APB1 定时器时钟（APB1 分频后 x2） */
#define APB2_TIM_CLK_HZ    168000000U  /* APB2 定时器时钟（APB2 分频后 x2） */

/*----------------------------- 电机 CAN 参数 -----------------------------*/
/*
 * C620 电调 ID：上电后看电调绿灯闪烁次数（闪 N 次 = ID N）。
 * 单电机出厂默认 ID = 1。若实测 ID 不同，只改 MOTOR_ID 即可。
 *   ID 1~4 的电调监听 0x200（每个 ID 占 8 字节数据里的 2 字节）；
 *   ID 5~8 的电调监听 0x1FF；
 *   反馈帧统一为 0x200 + ID。
 */
#define MOTOR_ID               1

#if (MOTOR_ID >= 1 && MOTOR_ID <= 4)
  #define CAN_CTRL_STDID       0x200U
  #define CAN_CTRL_OFFSET      ((uint32_t)(MOTOR_ID - 1) * 2U)
#elif (MOTOR_ID >= 5 && MOTOR_ID <= 8)
  #define CAN_CTRL_STDID       0x1FFU
  #define CAN_CTRL_OFFSET      ((uint32_t)(MOTOR_ID - 5) * 2U)
#else
  #error "MOTOR_ID 必须在 1~8 之间"
#endif

#define CAN_FEEDBACK_STDID     (0x200U + (uint32_t)MOTOR_ID)

/* 控制电流换算：±16384 对应电调转矩电流 ±20 A */
#define CURRENT_RAW_MAX        16384
#define CURRENT_FULL_SCALE_A   20.0f

/*----------------------------- 外设句柄（BSP 层持有）-----------------------------*/
extern TIM_HandleTypeDef  htim_timebase;  /* TIM6  HAL 时基 */
extern TIM_HandleTypeDef  htim_buzzer;    /* TIM4_CH3 PD14 */
extern TIM_HandleTypeDef  htim_led;       /* TIM5_CH1/2/3 PH10/PH11/PH12 */
extern TIM_HandleTypeDef  htim_servo;     /* TIM1_CH1 PE9 */
extern UART_HandleTypeDef huart_synex;    /* USART1 PA9/PB7 */
extern CAN_HandleTypeDef  hcan_motor;     /* CAN1 PD0/PD1 */

/*----------------------------- 函数声明 -----------------------------*/
void Board_ClockInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */
