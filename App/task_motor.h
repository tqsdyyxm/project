/**
  ******************************************************************************
  * @file    task_motor.h
  * @brief   电机控制任务（任务五）
  *
  * 功能：以 1 kHz 调用电机控制模块（CAN 反馈解析 + 角度累计 +
  *       位置式 PID + 限幅发送），并监测反馈失联故障：
  *       一旦失联自动停车并触发报错音 2。
  ******************************************************************************
  */

#ifndef __TASK_MOTOR_H
#define __TASK_MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

void Task_Motor_Entry(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_MOTOR_H */
