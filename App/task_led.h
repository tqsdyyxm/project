/**
  ******************************************************************************
  * @file    task_led.h
  * @brief   RGB LED 流水灯任务（任务二）
  *
  * 功能：上电后 LED 以"蓝 -> 绿 -> 红"的顺序循环流动（渐亮-保持-渐灭），
  *       PWM 驱动（TIM5），作为独立 FreeRTOS 任务持续运行，
  *       同时可当作"FreeRTOS 是否还在运行"的指示灯。
  ******************************************************************************
  */

#ifndef __TASK_LED_H
#define __TASK_LED_H

#ifdef __cplusplus
extern "C" {
#endif

void Task_Led_Entry(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_LED_H */
