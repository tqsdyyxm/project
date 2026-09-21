/**
  ******************************************************************************
  * @file    pid.c
  * @brief   位置式 PID 实现
  ******************************************************************************
  */

#include "pid.h"

#define PID_DT   0.001f    /* 固定控制周期 1 ms（调用频率 1 kHz） */

/* 比例/微分项的单独限幅系数（相对总输出限幅）。
 * 必须 > 1：否则运动过程中"比例项 +4A、微分项 -4A"直接相抵为 0，
 * 总输出被夹在 0 附近，电机失去制动能力 -> 严重超调。
 * 取 5 倍后，两项相抵仍能剩下净刹车电流（总输出仍严格限制在 out_max）。 */
#define PID_TERM_LIMIT_FACTOR   5.0f

static float clamp_sym(float v, float limit)
{
    if (v > limit)
    {
        return limit;
    }
    if (v < -limit)
    {
        return -limit;
    }
    return v;
}

void Pid_Init(Pid_t *p, float kp, float ki, float kd, float out_max, float i_max)
{
    p->kp = kp;
    p->ki = ki;
    p->kd = kd;
    p->out_max = out_max;
    p->i_max   = i_max;
    p->p_max   = out_max * PID_TERM_LIMIT_FACTOR;   /* 各项限幅放宽（见宏注释） */
    p->d_max   = out_max * PID_TERM_LIMIT_FACTOR;

    p->err   = 0.0f;
    p->i_out = 0.0f;
    p->out   = 0.0f;
    p->fdb_last = 0.0f;
    p->first = 1;
}

void Pid_SetParam(Pid_t *p, float kp, float ki, float kd)
{
    p->kp = kp;
    p->ki = ki;
    p->kd = kd;
}

void Pid_Reset(Pid_t *p)
{
    p->i_out    = 0.0f;
    p->out      = 0.0f;
    p->fdb_last = 0.0f;
    p->first    = 1;
}

float Pid_Calc(Pid_t *p, float set, float fdb)
{
    float p_out;
    float d_out;

    if (p->first)
    {
        p->first    = 0;
        p->fdb_last = fdb;
    }

    p->err = set - fdb;

    /* 比例（带限幅） */
    p_out = p->kp * p->err;
    p_out = clamp_sym(p_out, p->p_max);

    /* 积分（带限幅，防积分饱和） */
    p->i_out += p->ki * p->err * PID_DT;
    p->i_out  = clamp_sym(p->i_out, p->i_max);

    /* 微分作用于反馈：fdb_last - fdb 即角速度增量（带限幅，防噪声尖峰） */
    d_out = p->kd * (p->fdb_last - fdb) / PID_DT;
    d_out = clamp_sym(d_out, p->d_max);
    p->fdb_last = fdb;

    /* 总输出限幅 */
    p->out = clamp_sym(p_out + p->i_out + d_out, p->out_max);

    return p->out;
}
