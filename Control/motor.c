/**
  ******************************************************************************
  * @file    motor.c
  * @brief   3508 电机角度闭环实现
  ******************************************************************************
  */

#include "motor.h"
#include "pid.h"
#include "bsp_can.h"

/*----------------------------- 默认 PID 参数 -----------------------------*/
/* 安全起点：输出限幅 4A、积分限幅 2A。实机按《PID 调参手册》逐步整定。 */
#define PID_DEFAULT_KP       0.5f
#define PID_DEFAULT_KI       0.1f
#define PID_DEFAULT_KD       0.02f
#define PID_DEFAULT_I_MAX_A  2.0f

/* 目标角度序列：0° -> 90° -> -90°，循环；
 * 验收要求：整个序列在 2 秒（MOTOR_SEQ_CYCLE_MS）内完成 */
static const float s_target_seq[] = { 0.0f, 90.0f, -90.0f };
#define TARGET_SEQ_LEN  (sizeof(s_target_seq) / sizeof(s_target_seq[0]))

/*----------------------------- 内部状态 -----------------------------*/
static Pid_t        s_pid;
static Motor_State_t s_motor;

static int32_t  s_total_counts = 0;   /* 转子累计计数（含正负） */
static int16_t  s_last_counts  = 0;
static uint8_t  s_offset_done  = 0;
static uint32_t s_last_fb_tick = 0;

static float    s_target      = 0.0f;

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

    s_total_counts = 0;
    s_last_counts  = 0;
    s_offset_done  = 0;
    s_last_fb_tick = 0;
    s_target       = 0.0f;
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
    uint32_t seq_index;

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

    /* 2) 目标角度序列：整个 0°->90°->-90° 在 MOTOR_SEQ_CYCLE_MS 内完成。
     *    该序列与电机反馈无关、始终推进：这样即使电机尚未接入，
     *    上位机也能在"目标角度"通道看到 0/90/-90 的方波，便于联调与演示。 */
    seq_index  = (now / MOTOR_SEQ_PHASE_MS) % TARGET_SEQ_LEN;
    s_target   = s_target_seq[seq_index];
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

        s_motor.rotor_deg = (float)s_total_counts * 360.0f / MOTOR_ANGLE_COUNTS;
        s_motor.out_deg   = s_motor.rotor_deg / MOTOR_GEAR_RATIO;
        s_motor.rotor_rpm = (float)fb.speed_rpm;
        s_motor.out_speed_dps = s_motor.rotor_rpm * 360.0f / 60.0f / MOTOR_GEAR_RATIO;
        s_motor.current_a = (float)fb.current_raw * MOTOR_CURRENT_FULL_A /
                            (float)MOTOR_CURRENT_RAW_MAX;
        s_motor.feedback_ok = 1;
    }

    /* 5) 位置式 PID（1 kHz） */
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
