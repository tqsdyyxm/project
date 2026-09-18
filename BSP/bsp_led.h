/**
  ******************************************************************************
  * @file    bsp_led.h
  * @brief   RGB LED 板级驱动（任务二）
  *
  * 硬件：TIM5_CH1=PH10(蓝)  TIM5_CH2=PH11(绿)  TIM5_CH3=PH12(红)
  *       高电平点亮，PWM 占空比越高越亮。
  * 参数：PWM 频率 1 kHz，占空比 0~999。
  ******************************************************************************
  */

#ifndef __BSP_LED_H
#define __BSP_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"

/* 亮度档位：0 = 灭，LED_DUTY_MAX = 最亮 */
#define LED_DUTY_MAX   999U

typedef enum
{
    LED_BLUE  = 0,
    LED_GREEN = 1,
    LED_RED   = 2,
    LED_COUNT = 3
} Led_Color_t;

void BSP_Led_Init(void);

/* 设置单个颜色亮度，duty 范围 0~LED_DUTY_MAX（超范围自动截断） */
void BSP_Led_SetDuty(Led_Color_t color, uint32_t duty);

/* 一次设置 RGB 三色亮度（r/g/b 各 0~LED_DUTY_MAX） */
void BSP_Led_SetRgb(uint32_t r, uint32_t g, uint32_t b);

/* 全灭 */
void BSP_Led_Off(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LED_H */
