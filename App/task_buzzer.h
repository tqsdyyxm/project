/**
  ******************************************************************************
  * @file    task_buzzer.h
  * @brief   蜂鸣器任务（任务一）
  *
  * 功能：
  *   - 上电后播放启动音（每帧复位/烧录后都会响，用于确认程序已烧录）；
  *   - 两种不同的报错音：
  *       报错音 1（高音三短声）：收到无法识别的串口指令；
  *       报错音 2（低音两长声）：电机 CAN 反馈失联。
  * 任意任务/模块通过 Task_Buzzer_Play() 点播，任务内部按消息队列顺序播放。
  ******************************************************************************
  */

#ifndef __TASK_BUZZER_H
#define __TASK_BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
    BUZZER_TONE_STARTUP = 0,   /* 启动音：4kHz 单声 */
    BUZZER_TONE_ERROR_CMD,     /* 报错音 1：高音三短声 */
    BUZZER_TONE_ERROR_MOTOR,   /* 报错音 2：低音两长声 */
} Buzzer_Tone_t;

/* 点播一段音调（可在任意任务中调用，非阻塞） */
void Task_Buzzer_Play(Buzzer_Tone_t tone);

/* 任务入口（由 App 层创建） */
void Task_Buzzer_Entry(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_BUZZER_H */
