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

/* 目标序列状态机（回零 -> 两秒测试 -> 结果展示 -> 循环） */
typedef enum
{
    SEQ_STATE_ZEROING = 0,  /* 回零：目标 0°，等实际角度到位后开始测试 */
    SEQ_STATE_TESTING,      /* 两秒测试：0~700ms 目标 90°，700~2000ms 目标 -90° */
    SEQ_STATE_SHOW,         /* 结果展示：保持 -90° 展示本轮结果 */
} Seq_State_t;

static float    s_target        = 0.0f;
static float    s_settle_target = 0.0f;   /* 到位检测：当前跟踪的目标 */
static uint32_t s_settle_ms     = 0;      /* 连续在带宽内的时长（ms） */
static uint8_t  s_reached_90    = 0;
static uint8_t  s_reached_neg90 = 0;

static Seq_State_t s_seq_state  = SEQ_STATE_ZEROING;
static uint32_t    s_seq_start  = 0;      /* 本轮测试起点（tick） */
static uint32_t    s_show_start = 0;      /* 展示阶段起点（tick） */

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
    s_seq_state     = SEQ_STATE_ZEROING;
    s_seq_start     = 0;
    s_show_start    = 0;
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
    uint32_t t_test;

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

    /* 2) 目标序列状态机：回零 -> 两秒测试 -> 结果展示 -> 循环。
     *    只有实际角度回到 0°（误差 ±MOTOR_SETTLE_BAND_DEG 持续 MOTOR_SETTLE_HOLD_MS）
     *    后才开始两秒测试，保证每轮都真正执行 0° -> 90° -> -90°。
     *    测试阶段按固定时间切换目标，到位检测只记录、不推迟时间节点。 */
    switch (s_seq_state)
    {
        case SEQ_STATE_ZEROING:
            s_target = 0.0f;
            if (s_motor.feedback_ok && s_motor.settled)
            {
                /* 已稳定在 0°：开始本轮测试，并清除上一轮结果 */
                s_reached_90      = 0;
                s_reached_neg90   = 0;
                s_motor.test_pass = 0;
                s_seq_start = now;
                s_seq_state = SEQ_STATE_TESTING;
            }
            break;

        case SEQ_STATE_TESTING:
            t_test = now - s_seq_start;
            s_target = (t_test < MOTOR_TEST_T90_MS) ? 90.0f : -90.0f;
            if (t_test >= MOTOR_TEST_TOTAL_MS)
            {
                /* 测试结束：锁存本轮结果（1 通过 / -1 失败），进入展示阶段 */
                s_motor.test_pass = (s_reached_90 && s_reached_neg90) ? 1 : -1;
                s_show_start = now;
                s_seq_state  = SEQ_STATE_SHOW;
            }
            break;

        case SEQ_STATE_SHOW:
        default:
            s_target = -90.0f;
            if ((now - s_show_start) >= MOTOR_SHOW_MS)
            {
                s_seq_state = SEQ_STATE_ZEROING;   /* 回零，准备下一轮 */
            }
            break;
    }
    s_motor.target_deg = s_target;

    /* 3) 反馈超时看门狗 */
    elapsed = now - g_feedback_tick;
    if (elapsed > MOTOR_FEEDBACK_TIMEOUT_MS)
    {
        /* 失联：电流强制为 0；测试结果不保留旧值（曾测过 -> -1 失败，
         * 从未测过 -> 0），序列回到回零状态，反馈恢复后重新回零再测。 */
        s_motor.fault_timeout = 1;
        s_motor.settled       = 0;
        s_motor.test_pass     = (s_motor.feedback_ok) ? -1 : 0;
        s_seq_state = SEQ_STATE_ZEROING;
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
     *    只记录（settled / reached 标志），不干预状态机时序；
     *    test_pass 由状态机在测试结束时锁存，失联时由看门狗清零/置失败。 */
    if (s_motor.feedback_ok)
    {
        if (s_target != s_settle_target)
        {
            s_settle_target = s_target;
            s_settle_ms     = 0;
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
    }
    else
    {
        s_motor.settled = 0;
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
