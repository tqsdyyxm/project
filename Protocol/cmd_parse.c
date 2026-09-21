/**
  ******************************************************************************
  * @file    cmd_parse.c
  * @brief   串口指令解析实现
  *
  * 解析流程：逐字节收行 -> 按 '=' 拆分 -> 匹配参数名 -> strtof 转数值
  *           -> 写入 PID -> JustFloat 回显。
  ******************************************************************************
  */

#include "cmd_parse.h"
#include "bsp_uart.h"
#include "bsp_servo.h"
#include "justfloat.h"
#include "motor.h"
#include <string.h>
#include <stdlib.h>

#define CMD_LINE_MAX    64U
#define CMD_POLL_BYTES  16U    /* 每次轮询最多处理 16 字节 */

/* PID 参数安全范围（防止手误输入极端值损坏机械结构） */
#define PARAM_KP_MAX    50.0f
#define PARAM_KI_MAX    5.0f
#define PARAM_KD_MAX    5.0f

static char     s_line[CMD_LINE_MAX];
static uint32_t s_line_len   = 0;
static uint32_t s_error_cnt  = 0;

void Protocol_CmdParse_Init(void)
{
    s_line_len  = 0;
    s_error_cnt = 0;
}

/* 去掉首尾空白，返回有效长度 */
static uint32_t trim_line(char *buf, uint32_t len)
{
    uint32_t start = 0;
    uint32_t end   = len;

    while (start < end && (buf[start] == ' ' || buf[start] == '\t'))
    {
        start++;
    }
    while (end > start && (buf[end - 1] == ' ' || buf[end - 1] == '\t' ||
                           buf[end - 1] == '\r' || buf[end - 1] == '\n'))
    {
        end--;
    }
    if (end > start && start > 0)
    {
        memmove(buf, &buf[start], end - start);
    }
    return end - start;
}

/* 不区分大小写比较两个字符串（长度相等才继续） */
static int name_equal(const char *name, uint32_t name_len, const char *expect)
{
    if (name_len != strlen(expect))
    {
        return 0;
    }
    for (uint32_t i = 0; i < name_len; i++)
    {
        char c = name[i];
        if (c >= 'A' && c <= 'Z')
        {
            c = (char)(c - 'A' + 'a');
        }
        if (c != expect[i])
        {
            return 0;
        }
    }
    return 1;
}

/* 解析一行指令：kp=0.1 / ki=.. / kd=.. */
static void parse_line(const char *line, uint32_t len)
{
    char     name_buf[CMD_LINE_MAX];
    char     value_buf[CMD_LINE_MAX];
    uint32_t eq   = 0;
    uint32_t name_len;
    uint32_t val_len;
    float    value;
    float    kp, ki, kd;
    char    *endptr;

    /* 1) 找 '=' */
    while (eq < len && line[eq] != '=')
    {
        eq++;
    }
    if (eq == 0 || eq >= len)
    {
        s_error_cnt++;
        return;
    }

    /* 2) 参数名（去前后空格） */
    memcpy(name_buf, line, eq);
    name_buf[eq] = '\0';
    name_len = trim_line(name_buf, eq);
    name_buf[name_len] = '\0';

    /* 3) 数值部分（去前后空格） */
    val_len = len - eq - 1;
    if (val_len == 0 || val_len >= CMD_LINE_MAX)
    {
        s_error_cnt++;
        return;
    }
    memcpy(value_buf, &line[eq + 1], val_len);
    value_buf[val_len] = '\0';
    val_len = trim_line(value_buf, val_len);
    value_buf[val_len] = '\0';

    value = strtof(value_buf, &endptr);
    if (endptr == value_buf || value < 0.0f)
    {
        s_error_cnt++;
        return;
    }

    /* 4) 匹配参数名并更新 */
    Control_Motor_GetPid(&kp, &ki, &kd);

    if (name_equal(name_buf, name_len, "kp"))
    {
        if (value > PARAM_KP_MAX) { value = PARAM_KP_MAX; }
        kp = value;
    }
    else if (name_equal(name_buf, name_len, "ki"))
    {
        if (value > PARAM_KI_MAX) { value = PARAM_KI_MAX; }
        ki = value;
    }
    else if (name_equal(name_buf, name_len, "kd"))
    {
        if (value > PARAM_KD_MAX) { value = PARAM_KD_MAX; }
        kd = value;
    }
    else if (name_equal(name_buf, name_len, "smin"))
    {
        /* 舵机 0° 对应脉宽在线校准（µs） */
        BSP_Servo_SetPulseRange((uint16_t)value, BSP_Servo_GetPulseMax());
        JustFloat_SetEcho((float)BSP_Servo_GetPulseMin());   /* 回显实际生效值 */
        return;
    }
    else if (name_equal(name_buf, name_len, "smax"))
    {
        /* 舵机 180° 对应脉宽在线校准（µs） */
        BSP_Servo_SetPulseRange(BSP_Servo_GetPulseMin(), (uint16_t)value);
        JustFloat_SetEcho((float)BSP_Servo_GetPulseMax());   /* 回显实际生效值 */
        return;
    }
    else if (name_equal(name_buf, name_len, "ang"))
    {
        /* 舵机手动角度（0~180°），10 秒后自动恢复扫描，用于校准/诊断 */
        BSP_Servo_SetManual(value, 10000U);
        JustFloat_SetEcho(value);
        return;
    }
    else
    {
        s_error_cnt++;      /* 未识别的参数名 */
        return;
    }

    Control_Motor_SetPid(kp, ki, kd);

    /* 记录回显值：由 Synex 任务放进 JustFloat 通道 6 随帧回发（任务四验收点） */
    JustFloat_SetEcho(value);
}

void Protocol_CmdParse_Poll(void)
{
    uint32_t count = 0;

    while (count < CMD_POLL_BYTES && BSP_Uart_RxAvailable() > 0)
    {
        int32_t byte = BSP_Uart_ReadByte();
        count++;
        if (byte < 0)
        {
            break;
        }

        if ((uint8_t)byte == '\r' || (uint8_t)byte == '\n')
        {
            if (s_line_len > 0)
            {
                parse_line(s_line, s_line_len);
                s_line_len = 0;
            }
        }
        else if (s_line_len < CMD_LINE_MAX - 1U)
        {
            s_line[s_line_len++] = (char)byte;
        }
        else
        {
            /* 行过长：整行丢弃 */
            s_line_len = 0;
            s_error_cnt++;
        }
    }
}

uint32_t Protocol_CmdParse_GetErrorCount(void)
{
    return s_error_cnt;
}
