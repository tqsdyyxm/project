/**
  ******************************************************************************
  * @file    bsp_servo.h
  * @brief   舵机板级驱动（任务三）
  *
  * 硬件：TIM1_CH1 = PE9（AF1），PWM 口第 1 路（C1 信号 / B1 5V / A1 GND）。
  * 舵机：TS90A / SG90（9g）与 MG996R 同为 50 Hz PWM，脉宽 0.5~2.5 ms 对应 0~180°。
  *       不同舵机实际端点存在个体偏差，可用串口指令 smin=/smax= 在线校准（无需重新烧录）。
  ******************************************************************************
  */

#ifndef __BSP_SERVO_H
#define __BSP_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"

#define SERVO_PWM_FREQ_HZ     50U     /* PWM 频率 */
#define SERVO_PULSE_0DEG_US   500U    /* 0°   对应脉宽（默认值） */
#define SERVO_PULSE_180DEG_US 2500U   /* 180° 对应脉宽（默认值） */

/* 在线校准允许的脉宽范围（防止误输入极端值） */
#define SERVO_PULSE_MIN_US    300U
#define SERVO_PULSE_MAX_US    3000U

void BSP_Servo_Init(void);

/* 设置角度，0~180°（超范围自动截断） */
void BSP_Servo_SetAngle(float deg);

/* 在线校准脉宽范围（min < max 且至少相差 200µs；非法组合会被忽略） */
void BSP_Servo_SetPulseRange(uint16_t min_us, uint16_t max_us);
uint16_t BSP_Servo_GetPulseMin(void);
uint16_t BSP_Servo_GetPulseMax(void);

/* 手动角度测试（校准/诊断用）：设置后舵机任务在 timeout_ms 内只输出该角度，
 * 超时自动恢复自动扫描，避免影响正常演示。 */
void    BSP_Servo_SetManual(float deg, uint32_t timeout_ms);
uint8_t BSP_Servo_GetManual(float *deg);   /* 返回 1 = 手动模式生效 */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SERVO_H */
