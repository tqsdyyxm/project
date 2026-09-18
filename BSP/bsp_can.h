/**
  ******************************************************************************
  * @file    bsp_can.h
  * @brief   CAN 板级驱动（任务五：C620 电调 / 3508 电机）
  *
  * 硬件：CAN1，PD0(RX) / PD1(TX)，1 Mbps 标准帧。
  *   - 发送：ID 0x200 控制帧，电流写在本电机 ID 对应的 2 字节，高字节在前；
  *   - 接收：ID 0x200+ID 反馈帧，见 Can_Feedback_t 字段说明；
  *   - 电流量程：±16384 对应 ±20 A。
  ******************************************************************************
  */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"

/* 电机反馈数据（在 CAN 中断里更新，任务层只读） */
typedef struct
{
    uint16_t angle_raw;    /* DATA[0:1] 转子机械角度 0~8191 对应 0~360° */
    int16_t  speed_rpm;    /* DATA[2:3] 转子转速，单位 RPM */
    int16_t  current_raw;  /* DATA[4:5] 实际转矩电流 ±16384 */
    int8_t   temp;         /* DATA[6]   电机温度 ℃ */
} Can_Feedback_t;

/* 最近一次反馈的结构体与时间戳（HAL tick） */
extern volatile Can_Feedback_t g_feedback;
extern volatile uint32_t       g_feedback_tick;
/* CAN 错误计数（总线错误、BusOff 等，用于失联诊断） */
extern volatile uint32_t       g_can_error_count;

void BSP_Can_Init(void);

/* 发送控制电流，current_raw 范围 -16384 ~ +16384（自动截断） */
void BSP_Can_SendCurrent(int16_t current_raw);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_CAN_H */
