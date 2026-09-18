/**
  ******************************************************************************
  * @file    app_tasks.h
  * @brief   App 层任务声明与任务注册表（架构总览）
  *
  * 任务一览（验收讲解时可对照）：
  *   名称      周期      优先级          职责
  *   task_motor  1 ms   osPriorityHigh    任务五：CAN 反馈 + PID 角度闭环
  *   task_synex 10 ms   osPriorityAboveNormal 任务四：指令解析 + JustFloat 发送
  *   task_buzzer 10 ms  osPriorityNormal  任务一：启动音/报错音
  *   task_led    10 ms  osPriorityNormal  任务二：RGB 流水灯
  *   task_servo  20 ms  osPriorityNormal  任务三：舵机 1 秒转 45°
  ******************************************************************************
  */

#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

void App_Tasks_Init(void);   /* 创建全部任务（main 中调用一次） */

#ifdef __cplusplus
}
#endif

#endif /* __APP_TASKS_H */
