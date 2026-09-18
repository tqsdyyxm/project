/**
  ******************************************************************************
  * @file    main.c
  * @brief   程序入口：初始化硬件 -> 初始化各层 -> 启动 FreeRTOS
  *
  * 初始化顺序：HAL -> 时钟 -> BSP 板级驱动 -> Control 控制层 -> App 任务
  * 启动后所有业务逻辑都在 FreeRTOS 任务中运行，main 不再参与。
  ******************************************************************************
  */

#include "main.h"
#include "board.h"
#include "cmsis_os2.h"

#include "bsp_buzzer.h"
#include "bsp_led.h"
#include "bsp_servo.h"
#include "bsp_uart.h"
#include "bsp_can.h"

#include "cmd_parse.h"
#include "motor.h"
#include "app_tasks.h"

int main(void)
{
    /* 1. HAL 基础初始化（Flash 预取、SysTick 等） */
    HAL_Init();

    /* 2. 系统时钟：HSE 12 MHz -> SYSCLK 168 MHz，并把 HAL 时基切到 TIM6 */
    Board_ClockInit();

    /* 3. 板级驱动初始化（BSP 层） */
    BSP_Buzzer_Init();   /* 任务一：蜂鸣器 */
    BSP_Led_Init();      /* 任务二：RGB LED */
    BSP_Servo_Init();    /* 任务三：舵机 */
    BSP_Uart_Init();     /* 任务四：串口 + Synex */
    BSP_Can_Init();      /* 任务五：CAN + 3508 电机 */

    /* 4. 控制层与协议层初始化 */
    Control_Motor_Init();
    Protocol_CmdParse_Init();

    /* 5. 初始化 FreeRTOS 内核
     *    按 CMSIS-RTOS V2 标准顺序：osKernelInitialize() -> 创建任务 -> osKernelStart() */
    osKernelInitialize();

    /* 6. 创建全部 FreeRTOS 任务（App 层） */
    App_Tasks_Init();

    /* 7. 启动调度器，此后 main 不再执行 */
    osKernelStart();

    for (;;)
    {
    }
}

/**
  * @brief  错误处理：关中断并死循环（方便调试时停在断点）
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
