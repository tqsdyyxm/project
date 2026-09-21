/**
  ******************************************************************************
  * @file    justfloat.c
  * @brief   JustFloat 组帧与发送实现
  ******************************************************************************
  */

#include "justfloat.h"
#include "bsp_uart.h"
#include <string.h>

#define JF_MAX_CHANNELS 12U

static float s_echo = 0.0f;

void JustFloat_SendFrame(const float *channels, uint32_t count)
{
    uint8_t frame[JF_MAX_CHANNELS * 4U + 4U];
    uint32_t tail = JUSTFLOAT_TAIL;

    if (channels == 0 || count == 0 || count > JF_MAX_CHANNELS)
    {
        return;
    }

    /* Cortex-M4 为小端存储，float 直接按字节拷贝即为小端 float32 */
    memcpy(frame, channels, count * 4U);
    memcpy(&frame[count * 4U], &tail, 4U);

    BSP_Uart_Send(frame, count * 4U + 4U);
}

void JustFloat_SetEcho(float value)
{
    s_echo = value;
}

float JustFloat_GetEcho(void)
{
    return s_echo;
}
