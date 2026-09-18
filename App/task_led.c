/**
  ******************************************************************************
  * @file    task_led.c
  * @brief   流水灯实现：每个颜色 渐亮 400ms -> 保持 300ms -> 渐灭 400ms
  ******************************************************************************
  */

#include "task_led.h"
#include "bsp_led.h"
#include "cmsis_os2.h"

/* 流水顺序：蓝 -> 绿 -> 红 */
static const Led_Color_t s_order[LED_COUNT] = { LED_BLUE, LED_GREEN, LED_RED };

/* 每个颜色的节奏（单位 10ms 拍）：渐亮 40 拍、保持 30 拍、渐灭 40 拍 */
#define T_FADE_IN   40U
#define T_HOLD      30U
#define T_FADE_OUT  40U
#define T_TOTAL     (T_FADE_IN + T_HOLD + T_FADE_OUT)

void Task_Led_Entry(void *argument)
{
    (void)argument;

    uint32_t step = 0;

    for (;;)
    {
        Led_Color_t color;
        uint32_t    pos;
        uint32_t    duty = 0;

        color = s_order[(step / T_TOTAL) % LED_COUNT];
        pos   = step % T_TOTAL;

        if (pos < T_FADE_IN)
        {
            /* 渐亮 */
            duty = LED_DUTY_MAX * pos / T_FADE_IN;
        }
        else if (pos < T_FADE_IN + T_HOLD)
        {
            /* 保持最亮 */
            duty = LED_DUTY_MAX;
        }
        else
        {
            /* 渐灭 */
            duty = LED_DUTY_MAX * (T_TOTAL - 1U - pos) / T_FADE_OUT;
        }

        BSP_Led_Off();
        BSP_Led_SetDuty(color, duty);

        step++;
        osDelay(10U);
    }
}
