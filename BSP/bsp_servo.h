/**
  ******************************************************************************
  * @file    bsp_servo.h
  * @brief   舵机板级驱动（任务三）
  *
  * 硬件：TIM1_CH1 = PE9（AF1），PWM 口第 1 路（C1 信号 / B1 5V / A1 GND）。
  * 舵机：MG996R，PWM 50 Hz，脉宽 0.5 ms ~ 2.5 ms 对应 0° ~ 180°。
  *       若实验室舵机不同，只需修改下面三个宏。
  ******************************************************************************
  */

#ifndef __BSP_SERVO_H
#define __BSP_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"

#define SERVO_PWM_FREQ_HZ     50U     /* PWM 频率 */
#define SERVO_PULSE_0DEG_US   500U    /* 0°   对应脉宽 */
#define SERVO_PULSE_180DEG_US 2500U   /* 180° 对应脉宽 */

void BSP_Servo_Init(void);

/* 设置角度，0~180°（超范围自动截断） */
void BSP_Servo_SetAngle(float deg);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SERVO_H */
