/**
  ******************************************************************************
  * @file    motor.c
  * @brief   3508 电机角度闭环实现
  ******************************************************************************
  */

#include "motor.h"
#include "pid.h"
#include "bsp_can.h"
#include <math.h>

/*----------------------------- 默认 PID 参数 -----------------------------*/
/* 安全起点：输出限幅 4A、积分限幅 2A。实机按《PID 调参手册》逐步整定。 */
#define PID_DEFAULT_KP       0.5f
#define PID_DEFAULT_KI       0.1f
#define PID_DEFAULT_KD       0.02f
#define PID_DEFAULT_I_MAX_A  2.0f

/*----------------------------- 内部状态 -----------------------------*/
static Pid_t        s_pid;
static Motor_State_t s_motor;

static int32_t  s_total_counts = 0;   /* 转子累计计数（含正负） */
static int16_t  s_last_counts  = 0;
static uint8_t  s_offset_done  = 0;
static uint32_t s_last_fb_tick = 0;

static float    s_target        = 0.0f;
static float    s_settle_target = 0.0f;   /* 到位检测：当前跟踪的目标 */
static uint32_t s_settle_ms     = 0;      /* 连续在带宽内的时长（ms） */
static uint8_t  s_reached_90    = 0;
static uint8_t  s_reached_neg90 = 0;

void Control_Motor_Init(void)
{
    Pid_Init(&s_pid,
             PID_DEFAULT_KP, PID_DEFAULT_KI, PID_DEFAULT_KD,
             MOTOR_CURRENT_LIMIT_A, PID_DEFAULT_I_MAX_A);

    s_motor.current_a     = 0.0f;
    s_motor.rotor_deg     = 0.0f;
    s_motor.rotor_rpm     = 0.0f;
    s_motor.out_deg       = 0.0f;
    s_motor.out_speed_dps = 0.0f;
    s_motor.target_deg    = 0.0f;
    s_motor.feedback_ok   = 0;
    s_motor.fault_timeout = 0;
    s_motor.settled       = 0;
    s_motor.test_pass     = 0;

    s_total_counts = 0;
    s_last_counts  = 0;
    s_offset_done  = 0;
    s_last_fb_tick = 0;
    s_target       = 0.0f;
    s_settle_target = 0.0f;
    s_settle_ms     = 0;
    s_reached_90    = 0;
    s_reached_neg90 = 0;
}

void Control_Motor_SetPid(float kp, float ki, float kd)
{
    Pid_SetParam(&s_pid, kp, ki, kd);
}

void Control_Motor_GetPid(float *kp, float *ki, float *kd)
{
    if (kp != 0) { *kp = s_pid.kp; }
    if (ki != 0) { *ki = s_pid.ki; }
    if (kd != 0) { *kd = s_pid.kd; }
}

const Motor_State_t *Control_Motor_GetState(void)
{
    return &s_motor;
}

void Control_Motor_Task1ms(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsed;
    Can_Feedback_t fb;
    uint8_t fresh = 0;
    int16_t raw_cmd = 0;
    float   current_cmd_a;
    uint32_t t_cycle;

    /* 1) 判断是否有新反馈（g_feedback_tick 只在收到新帧时变化） */
    if (g_feedback_tick != s_last_fb_tick)
    {
        /* 快照拷贝期间短暂关中断，避免读到"半新半旧"的数据 */
        __disable_irq();
        fb             = g_feedback;
        s_last_fb_tick = g_feedback_tick;
        __enable_irq();
        fresh = 1;
    }

    /* 2) 目标角度序列：0°->90°->-90°，整个序列在 MOTOR_SEQ_CYCLE_MS 内完成。
     *    时间分配：0° 展示 T0 ms，0°->90° 给 T1 ms，剩余给 90°->-90°（行程更长）。
     *    序列与电机反馈无关、始终推进：即使电机尚未接入，
     *    上位机也能在"目标角度"通道看到方波，便于联调与演示。 */
    t_cycle = now % MOTOR_SEQ_CYCLE_MS;
    if (t_cycle < MOTOR_SEQ_T0_MS)
    {
        s_target = 0.0f;
    }
    else if (t_cycle < MOTOR_SEQ_T0_MS + MOTOR_SEQ_T1_MS)
    {
        s_target = 90.0f;
    }
    else
    {
        s_target = -90.0f;
    }
    s_motor.target_deg = s_target;

    /* 3) 反馈超时看门狗 */
    elapsed = now - g_feedback_tick;
    if (elapsed > MOTOR_FEEDBACK_TIMEOUT_MS)
    {
        /* 失联：电流强制为 0 + 置故障标志（反馈恢复后自动清除）；
         * 目标角度仍继续推进（仅用于显示，不驱动电机）。 */
        s_motor.fault_timeout = 1;
        Pid_Reset(&s_pid);
        BSP_Can_SendCurrent(0);
        return;
    }

    /* 4) 处理新反馈：角度累计与单位换算 */
    if (fresh)
    {
        s_motor.fault_timeout = 0;

        if (!s_offset_done)
        {
            /* 上电后第一次反馈：把当前位置记为 0°（输出轴） */
            s_offset_done = 1;
            s_total_counts = 0;
        }
        else
        {
            int32_t delta = (int32_t)fb.angle_raw - (int32_t)s_last_counts;
            /* 过零修正：两次采样差不超过半圈 */
            if (delta > 4096)
            {
                delta -= 8192;
            }
            else if (delta < -4096)
            {
                delta += 8192;
            }
            s_total_counts += delta;
        }
        s_last_counts = (int16_t)fb.angle_raw;

        /* 转子机械角上报为单圈 0~360°；输出轴角度用多圈累计换算 */
        s_motor.rotor_deg = (float)fb.angle_raw * 360.0f / MOTOR_ANGLE_COUNTS;
        s_motor.out_deg   = (float)s_total_counts * 360.0f /
                            MOTOR_ANGLE_COUNTS / MOTOR_GEAR_RATIO;
        s_motor.rotor_rpm = (float)fb.speed_rpm;
        s_motor.out_speed_dps = s_motor.rotor_rpm * 360.0f / 60.0f / MOTOR_GEAR_RATIO;
        s_motor.current_a = (float)fb.current_raw * MOTOR_CURRENT_FULL_A /
                            (float)MOTOR_CURRENT_RAW_MAX;
        s_motor.feedback_ok = 1;
    }

    /* 5) 到位检测：误差在 ±MOTOR_SETTLE_BAND_DEG 内持续 MOTOR_SETTLE_HOLD_MS 视为到位。
     *    只记录 + 上报（供 Synex 通道 7 显示），不干预控制时序。 */
    if (s_motor.feedback_ok)
    {
        if (s_target != s_settle_target)
        {
            s_settle_target = s_target;
            s_settle_ms     = 0;
            if (s_target == 0.0f)       /* 新一轮开始：清空上一轮结果 */
            {
                s_reached_90    = 0;
                s_reached_neg90 = 0;
            }
        }

        if (fabsf(s_motor.out_deg - s_target) <= MOTOR_SETTLE_BAND_DEG)
        {
            s_settle_ms++;
        }
        else
        {
            s_settle_ms = 0;
        }

        if (s_settle_ms >= MOTOR_SETTLE_HOLD_MS)
        {
            s_motor.settled = 1;
            if (s_target == 90.0f)  { s_reached_90 = 1; }
            if (s_target == -90.0f) { s_reached_neg90 = 1; }
        }
        else
        {
            s_motor.settled = 0;
        }

        s_motor.test_pass = (s_reached_90 && s_reached_neg90) ? 1 : 0;
    }
    else
    {
        s_motor.settled   = 0;
        s_motor.test_pass = 0;
    }

    /* 6) 位置式 PID（1 kHz） */
    if (s_motor.feedback_ok)
    {
        current_cmd_a = Pid_Calc(&s_pid, s_target, s_motor.out_deg);
        /* A -> 原始值，并做最终输出限幅（双重保险） */
        raw_cmd = (int16_t)(current_cmd_a * (float)MOTOR_CURRENT_RAW_MAX /
                            MOTOR_CURRENT_FULL_A);
        BSP_Can_SendCurrent(raw_cmd);
    }
    else
    {
        BSP_Can_SendCurrent(0);
    }
}
