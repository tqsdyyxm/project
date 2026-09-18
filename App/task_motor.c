/**
  ******************************************************************************
  * @file    task_motor.c
  * @brief   电机控制任务实现（1 kHz）
  ******************************************************************************
  */

#include "task_motor.h"
#include "task_buzzer.h"
#include "motor.h"
#include "cmsis_os2.h"

void Task_Motor_Entry(void *argument)
{
    (void)argument;

    uint8_t last_fault = 0;

    for (;;)
    {
        Control_Motor_Task1ms();

        /* 失联故障上升沿 -> 报错音 2（要求曾经收到过反馈才报，避免上电误报） */
        const Motor_State_t *m = Control_Motor_GetState();
        if (m->fault_timeout != 0 && last_fault == 0 && m->feedback_ok != 0)
        {
            Task_Buzzer_Play(BUZZER_TONE_ERROR_MOTOR);
        }
        last_fault = m->fault_timeout;

        osDelay(1U);
    }
}
