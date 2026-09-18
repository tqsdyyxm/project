/**
  ******************************************************************************
  * @file    bsp_buzzer.h
  * @brief   蜂鸣器板级驱动（任务一）
  *
  * 硬件：TIM4_CH3 = PD14（AF2），无源蜂鸣器，额定频率 4 kHz。
  * 用法：BSP_Buzzer_On(频率) 开始发声，BSP_Buzzer_Off() 停止。
  ******************************************************************************
  */

#ifndef __BSP_BUZZER_H
#define __BSP_BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"

#define BUZZER_RATED_FREQ_HZ 4000U   /* 额定基准频率 */

void BSP_Buzzer_Init(void);

/* 按指定频率发声（内部自动限制在 100 Hz ~ 20 kHz） */
void BSP_Buzzer_On(uint32_t freq_hz);

/* 停止发声 */
void BSP_Buzzer_Off(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_BUZZER_H */
