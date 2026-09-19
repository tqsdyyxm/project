/**
  ******************************************************************************
  * @file    motor.h
  * @brief   3508 电机控制模块（任务五：角度闭环）
  *
  * 职责：
  *   - 解析 CAN 反馈，累计转子角度并换算到输出轴角度（含减速比）；
  *   - 运行目标序列 0° -> 90° -> -90°（整个序列 2 秒内完成，循环演示）；
  *   - 位置式 PID 计算电流指令并限幅发送；
  *   - 反馈超时看门狗：失联立即停车。
  *
  * 关键换算（验收必问）：
  *   机械角计数 0~8191 <-> 转子 0~360°
  *   输出轴角度 = 转子角度 / 减速比，减速比 = 3591/187 ≈ 19.2
  *   电流 ±16384 <-> ±20 A
  ******************************************************************************
  */

#ifndef __MOTOR_H
#define __MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*----------------------------- 可调参数（宏）-----------------------------*/
#define MOTOR_GEAR_RATIO          (3591.0f / 187.0f)  /* 3508 减速比 ≈ 19.2 */
#define MOTOR_ANGLE_COUNTS        8192.0f             /* 一圈的编码器计数 */
#define MOTOR_CURRENT_FULL_A      20.0f               /* C620 满量程电流 */
#define MOTOR_CURRENT_RAW_MAX     16384               /* C620 满量程原始值 */
#define MOTOR_CURRENT_LIMIT_A     4.0f                /* 输出电流限幅（安全值） */
#define MOTOR_FEEDBACK_TIMEOUT_MS 100U                /* 反馈超时时间 */
/* 任务五验收时序：整个序列 0° -> 90° -> -90° 在 2 秒内完成，循环演示。
 * 时间分配（非均匀，长行程给更多时间）：
 *   0~100ms      目标 0°（起点展示）
 *   100~800ms    目标 90°（0°->90°，行程 90°）
 *   800~2000ms   目标 -90°（90°->-90°，行程 180°，给 1200ms） */
#define MOTOR_SEQ_CYCLE_MS       2000U   /* 一个完整序列的时限（2 秒） */
#define MOTOR_SEQ_T0_MS          100U    /* 0° 起点展示时间 */
#define MOTOR_SEQ_T1_MS          700U    /* 0°->90° 用时 */

/* 到位检测（只记录与上报，不干预控制时序） */
#define MOTOR_SETTLE_BAND_DEG    5.0f    /* 输出轴误差进入 ±5° 视为到位 */
#define MOTOR_SETTLE_HOLD_MS     50U     /* 连续保持 50ms 才算到位 */

/*----------------------------- 状态结构体 -----------------------------*/
typedef struct
{
    float    current_a;      /* 实际转矩电流，A */
    float    rotor_deg;      /* 转子机械角（单圈 0~360°） */
    float    rotor_rpm;      /* 转子转速，RPM */
    float    out_deg;        /* 输出轴累计角度（相对上电位置），° */
    float    out_speed_dps;  /* 输出轴角速度，°/s */
    float    target_deg;     /* 当前目标角度，° */
    uint8_t  feedback_ok;    /* 是否收到过反馈 */
    uint8_t  fault_timeout;  /* 反馈超时故障标志 */
    uint8_t  settled;        /* 当前是否在目标带宽内持续到位 */
    uint8_t  test_pass;      /* 本轮 0->90->-90 是否都到位（0/1） */
} Motor_State_t;

/*----------------------------- 接口 -----------------------------*/
void     Control_Motor_Init(void);
void     Control_Motor_SetPid(float kp, float ki, float kd);
void     Control_Motor_GetPid(float *kp, float *ki, float *kd);
const Motor_State_t *Control_Motor_GetState(void);

/* 电机任务以 1 kHz 调用本函数 */
void     Control_Motor_Task1ms(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H */
