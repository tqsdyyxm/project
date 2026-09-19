/**
  ******************************************************************************
  * @file    task_synex.h
  * @brief   Synex 通信任务（任务四）
  *
  * 功能：
  *   - 每 10ms 解析串口指令（kp=0.1\r 等）；
  *   - 每 20ms（50Hz）用 JustFloat 协议发送固定 7 通道帧：
  *       通道1 电流(A)  通道2 转子机械角(°)  通道3 转子转速(RPM)
  *       通道4 输出轴角度(°)  通道5 目标角度(°)  通道6 参数回显  通道7 到位标志；
  *   - 收到无法识别的指令时触发报错音 1。
  ******************************************************************************
  */

#ifndef __TASK_SYNEX_H
#define __TASK_SYNEX_H

#ifdef __cplusplus
extern "C" {
#endif

void Task_Synex_Entry(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_SYNEX_H */
