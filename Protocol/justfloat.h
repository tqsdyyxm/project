/**
  ******************************************************************************
  * @file    justfloat.h
  * @brief   JustFloat 协议组帧（任务四：发给 Synex 上位机）
  *
  * 协议格式（Synex 官方文档）：
  *   若干个小端 float32 按通道顺序排列，最后追加 4 字节帧尾 0x7F800000。
  *   例：发送电流、角度、转速三个通道 =
  *       [float 电流][float 角度][float 转速][0x7F800000]。
  ******************************************************************************
  */

#ifndef __JUSTFLOAT_H
#define __JUSTFLOAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define JUSTFLOAT_TAIL      0x7F800000U   /* 帧尾标记（固定值） */

/* 本工程通道定义（与《Synex 调试手册》中的通道对应表一致） */
#define JF_CH_CURRENT_A     1U   /* 电机实际转矩电流，单位 A */
#define JF_CH_ROTOR_DEG     2U   /* 转子机械角度 0~360° */
#define JF_CH_ROTOR_RPM     3U   /* 转子转速，单位 RPM */
#define JF_CH_OUT_DEG       4U   /* 输出轴累计角度（相对上电位置），单位 ° */
#define JF_CH_TARGET_DEG    5U   /* PID 目标角度，单位 ° */
#define JF_CH_ECHO          6U   /* 参数回显通道（收到 kp=0.1 后回发 0.1） */
#define JF_CH_SETTLED       7U   /* 测试结果：0=测试中，1=通过，-1=失败 */
#define JF_CH_T90_MS        8U   /* 本轮到达 +90° 耗时（ms），0=未到达 */
#define JF_CH_TNEG90_MS     9U   /* 本轮到达 -90° 耗时（ms），0=未到达 */

/* 发送一帧：count 个 float 通道 + 帧尾 */
void JustFloat_SendFrame(const float *channels, uint32_t count);

/* 参数回显：设置/读取回显值（由 Synex 任务放进通道 6 随帧发出）。
 * 说明：JustFloat 的通道是按帧内顺序编号的，为保持通道不串位，
 *       所有遥测帧统一为固定 9 通道布局。 */
void  JustFloat_SetEcho(float value);
float JustFloat_GetEcho(void);

#ifdef __cplusplus
}
#endif

#endif /* __JUSTFLOAT_H */
