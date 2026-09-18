/**
  ******************************************************************************
  * @file    task_servo.h
  * @brief   舵机任务（任务三）
  *
  * 功能：上电后舵机以 45°/s 的速度从 0° 转到 180°，保持 1 秒，
  *       再以同样速度转回 0°，循环。PWM 驱动（TIM1），独立 FreeRTOS 任务。
  ******************************************************************************
  */

#ifndef __TASK_SERVO_H
#define __TASK_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

void Task_Servo_Entry(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_SERVO_H */
