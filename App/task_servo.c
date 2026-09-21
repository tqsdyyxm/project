/**
  ******************************************************************************
  * @file    task_servo.c
  * @brief   舵机任务实现：45°/s 匀速扫过 0~180°
  *
  * 时间线（毫秒）：
  *   0 ~ 4000     0° -> 180°（1 秒转 45°）
  *   4000 ~ 5000  停在 180°
  *   5000 ~ 9000  180° -> 0°
  *   9000 ~ 10000 停在 0°，然后循环
  ******************************************************************************
  */

#include "task_servo.h"
#include "bsp_servo.h"
#include "cmsis_os2.h"

#define SERVO_SPEED_DPS  45.0f        /* 角速度 45°/s */
#define TRAVEL_TIME_MS   4000U        /* 0->180 用时 */
#define HOLD_TIME_MS     1000U        /* 端点保持 */
#define CYCLE_TIME_MS    (2U * TRAVEL_TIME_MS + 2U * HOLD_TIME_MS)

void Task_Servo_Entry(void *argument)
{
    (void)argument;

    for (;;)
    {
        uint32_t pos = osKernelGetTickCount() % CYCLE_TIME_MS;
        float    angle;
        float    manual_deg;

        /* 手动角度模式（串口 ang= 触发，10 秒后自动恢复扫描） */
        if (BSP_Servo_GetManual(&manual_deg))
        {
            BSP_Servo_SetAngle(manual_deg);
            osDelay(20U);
            continue;
        }

        if (pos < TRAVEL_TIME_MS)
        {
            /* 0 -> 180 */
            angle = (float)pos * SERVO_SPEED_DPS / 1000.0f;
        }
        else if (pos < TRAVEL_TIME_MS + HOLD_TIME_MS)
        {
            angle = 180.0f;
        }
        else if (pos < 2U * TRAVEL_TIME_MS + HOLD_TIME_MS)
        {
            /* 180 -> 0 */
            angle = 180.0f - (float)(pos - TRAVEL_TIME_MS - HOLD_TIME_MS) *
                              SERVO_SPEED_DPS / 1000.0f;
        }
        else
        {
            angle = 0.0f;
        }

        BSP_Servo_SetAngle(angle);

        osDelay(20U);
    }
}
