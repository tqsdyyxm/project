/**
  ******************************************************************************
  * @file    pid.c
  * @brief   位置式 PID 实现
  ******************************************************************************
  */

#include "pid.h"

#define PID_DT   0.001f    /* 固定控制周期 1 ms（调用频率 1 kHz） */

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
    p->p_max   = out_max;   /* 各项默认与总输出同限幅 */
    p->d_max   = out_max;

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
