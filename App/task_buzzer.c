/**
  ******************************************************************************
  * @file    task_buzzer.c
  * @brief   蜂鸣器任务实现：非阻塞乐谱播放器
  ******************************************************************************
  */

#include "task_buzzer.h"
#include "bsp_buzzer.h"
#include "cmsis_os2.h"

/* 一个音符：频率 + 发声时长 + 休止时长（10ms 为最小粒度） */
typedef struct
{
    uint32_t freq;
    uint16_t on_ms;
    uint16_t gap_ms;
} Note_t;

static const Note_t s_melody_startup[] =
{
    { BUZZER_RATED_FREQ_HZ, 300, 0   },     /* 单声提示音，严格对应题面"响一次" */
};

static const Note_t s_melody_error_cmd[] =
{
    { 4000, 100, 100 },                     /* 高音三短声 */
    { 4000, 100, 100 },
    { 4000, 100, 0   },
};

static const Note_t s_melody_error_motor[] =
{
    { 1500, 400, 150 },                     /* 低音两长声 */
    { 1500, 400, 0   },
};

static const Note_t *s_melody_table[] =
{
    s_melody_startup,
    s_melody_error_cmd,
    s_melody_error_motor,
};

static const uint32_t s_melody_len[] =
{
    sizeof(s_melody_startup)     / sizeof(Note_t),
    sizeof(s_melody_error_cmd)   / sizeof(Note_t),
    sizeof(s_melody_error_motor) / sizeof(Note_t),
};

static osMessageQueueId_t s_queue;
static const osMessageQueueAttr_t s_queue_attr =
{
    .name = "buzzer_queue",
};

void Task_Buzzer_Play(Buzzer_Tone_t tone)
{
    uint32_t t = (uint32_t)tone;

    if (s_queue != 0)
    {
        osMessageQueuePut(s_queue, &t, 0U, 0U);
    }
}

void Task_Buzzer_Entry(void *argument)
{
    (void)argument;

    uint32_t tone;
    osStatus_t status;

    s_queue = osMessageQueueNew(8U, sizeof(uint32_t), &s_queue_attr);

    /* 上电启动音（任务四/五的报错音通过队列点播） */
    Task_Buzzer_Play(BUZZER_TONE_STARTUP);

    for (;;)
    {
        /* 等待点播，10ms 超时用于周期性检查 */
        status = osMessageQueueGet(s_queue, &tone, 0U, 10U);
        if (status == osOK)
        {
            if (tone >= (uint32_t)(sizeof(s_melody_table) / sizeof(s_melody_table[0])))
            {
                continue;
            }

            const Note_t *melody = s_melody_table[tone];
            uint32_t      len    = s_melody_len[tone];

            /* 逐音符非阻塞播放（1 tick = 1 ms，10ms 粒度） */
            for (uint32_t i = 0; i < len; i++)
            {
                uint32_t remain;

                BSP_Buzzer_On(melody[i].freq);
                remain = melody[i].on_ms;
                while (remain > 0)
                {
                    uint32_t step = (remain > 10U) ? 10U : remain;
                    osDelay(step);
                    remain -= step;
                }

                BSP_Buzzer_Off();
                remain = melody[i].gap_ms;
                while (remain > 0)
                {
                    uint32_t step = (remain > 10U) ? 10U : remain;
                    osDelay(step);
                    remain -= step;
                }
            }
        }
    }
}
