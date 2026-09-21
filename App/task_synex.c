/**
  ******************************************************************************
  * @file    task_synex.c
  * @brief   Synex 通信任务实现
  ******************************************************************************
  */

#include "task_synex.h"
#include "task_buzzer.h"
#include "cmd_parse.h"
#include "justfloat.h"
#include "motor.h"
#include "cmsis_os2.h"

#define SEND_INTERVAL_TICKS 2U    /* 20ms 发送一帧（10ms 任务周期的 2 倍） */

void Task_Synex_Entry(void *argument)
{
    (void)argument;

    uint32_t tick      = 0;
    uint32_t last_err  = 0;

    for (;;)
    {
        /* 1) 解析串口指令（kp= / ki= / kd=） */
        Protocol_CmdParse_Poll();

        /* 2) 每 20ms 发一帧 JustFloat 数据（50 Hz，固定 9 通道布局） */
        if ((tick % SEND_INTERVAL_TICKS) == 0U)
        {
            const Motor_State_t *m = Control_Motor_GetState();
            float channels[9];

            channels[0] = m->current_a;      /* 通道1：电流 A */
            channels[1] = m->rotor_deg;      /* 通道2：转子机械角(单圈 0~360) ° */
            channels[2] = m->rotor_rpm;      /* 通道3：转子转速 RPM */
            channels[3] = m->out_deg;        /* 通道4：输出轴角度 ° */
            channels[4] = m->target_deg;     /* 通道5：目标角度 ° */
            channels[5] = JustFloat_GetEcho(); /* 通道6：参数回显 */
            channels[6] = (float)m->test_pass;   /* 通道7：测试结果 */
            channels[7] = (float)m->t90_ms;      /* 通道8：到达 +90° 耗时(ms) */
            channels[8] = (float)m->tneg90_ms;   /* 通道9：到达 -90° 耗时(ms) */

            JustFloat_SendFrame(channels, 9U);
        }

        /* 3) 解析失败 -> 报错音 1 */
        uint32_t err = Protocol_CmdParse_GetErrorCount();
        if (err != last_err)
        {
            last_err = err;
            Task_Buzzer_Play(BUZZER_TONE_ERROR_CMD);
        }

        tick++;
        osDelay(10U);
    }
}
