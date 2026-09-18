/**
  ******************************************************************************
  * @file    pid.h
  * @brief   位置式 PID 控制器（任务五）
  *
  * 特点：
  *   - 积分限幅 + 输出限幅（保护机械结构，大作业明确要求）；
  *   - D 项作用于反馈量（避免目标跳变引起微分冲击）；
  *   - 固定调用周期 1 kHz（dt 折算进参数单位，见下）。
  *
  * 参数单位（角度环）：
  *   kp : A/deg     比例输出 = kp * 误差(deg)
  *   ki : A/(deg*s) 积分输出 += ki * 误差 * dt
  *   kd : A/(deg/s) 微分输出 = kd * 角速度(deg/s)
  ******************************************************************************
  */

#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
    float kp, ki, kd;       /* 三个参数 */

    float err;              /* 当前误差（可观测） */
    float i_out;            /* 积分项输出（可观测） */
    float out;              /* 总输出（可观测） */

    float out_max;          /* 总输出限幅（对称） */
    float i_max;            /* 积分限幅（对称） */

    float fdb_last;         /* 上次反馈（微分用） */
    uint8_t first;          /* 首次计算标志 */
} Pid_t;

void  Pid_Init(Pid_t *p, float kp, float ki, float kd, float out_max, float i_max);
void  Pid_SetParam(Pid_t *p, float kp, float ki, float kd);
void  Pid_Reset(Pid_t *p);

/* 固定 1 kHz 调用；返回本次总输出（已限幅） */
float Pid_Calc(Pid_t *p, float set, float fdb);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H */
