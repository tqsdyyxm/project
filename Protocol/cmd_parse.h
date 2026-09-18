/**
  ******************************************************************************
  * @file    cmd_parse.h
  * @brief   串口参数指令解析（任务四：Synex 下发调参指令）
  *
  * 指令格式（以回车/换行结尾的一行文本）：
  *   kp=0.1\r   设置位置环 Kp 为 0.1
  *   ki=0.01\r  设置位置环 Ki 为 0.01
  *   kd=0.02\r  设置位置环 Kd 为 0.02
  * 收到并成功解析后，STM32 会：
  *   1. 更新 PID 参数；
  *   2. 用 JustFloat 协议向 Synex 回发该数值（回显通道）。
  ******************************************************************************
  */

#ifndef __CMD_PARSE_H
#define __CMD_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void     Protocol_CmdParse_Init(void);
void     Protocol_CmdParse_Poll(void);               /* 每 10ms 调用一次 */
uint32_t Protocol_CmdParse_GetErrorCount(void);      /* 解析失败累计次数 */

#ifdef __cplusplus
}
#endif

#endif /* __CMD_PARSE_H */
