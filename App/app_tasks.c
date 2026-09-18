/**
  ******************************************************************************
  * @file    app_tasks.c
  * @brief   任务注册表：创建全部 FreeRTOS 任务（CMSIS-RTOS V2）
  ******************************************************************************
  */

#include "app_tasks.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"       /* 必须先于 task.h 包含 */
#include "task.h"            /* FreeRTOS 底层类型（钩子函数签名） */

#include "task_buzzer.h"
#include "task_led.h"
#include "task_servo.h"
#include "task_synex.h"
#include "task_motor.h"

/* 任务属性：stack_size 单位为字节
 * 优先级说明：FreeRTOS 合法优先级为 0~55（configMAX_PRIORITIES=56）；
 *   CMSIS 定义 osPriorityRealtime=48、osPriorityISR=56（ISR 值不能用于任务）。
 *   电机任务用 osPriorityHigh(40)，已满足 1 kHz 实时性要求。 */
static const osThreadAttr_t motor_attr =
{
    .name       = "task_motor",
    .stack_size = 256U * 4U,
    .priority   = osPriorityHigh,
};

static const osThreadAttr_t synex_attr =
{
    .name       = "task_synex",
    .stack_size = 512U * 4U,
    .priority   = osPriorityAboveNormal,
};

static const osThreadAttr_t buzzer_attr =
{
    .name       = "task_buzzer",
    .stack_size = 256U * 4U,
    .priority   = osPriorityNormal,
};

static const osThreadAttr_t led_attr =
{
    .name       = "task_led",
    .stack_size = 128U * 4U,
    .priority   = osPriorityNormal,
};

static const osThreadAttr_t servo_attr =
{
    .name       = "task_servo",
    .stack_size = 128U * 4U,
    .priority   = osPriorityNormal,
};

void App_Tasks_Init(void)
{
    osThreadNew(Task_Motor_Entry,  NULL, &motor_attr);
    osThreadNew(Task_Synex_Entry,  NULL, &synex_attr);
    osThreadNew(Task_Buzzer_Entry, NULL, &buzzer_attr);
    osThreadNew(Task_Led_Entry,    NULL, &led_attr);
    osThreadNew(Task_Servo_Entry,  NULL, &servo_attr);
}

/* ===================== FreeRTOS 钩子（配置要求提供） ===================== */

/**
  * @brief  任务栈溢出（configCHECK_FOR_STACK_OVERFLOW = 2 时要求实现）
  *          溢出属于严重错误：停在这里便于调试器定位。
  */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}

/**
  * @brief  堆内存申请失败（configUSE_MALLOC_FAILED_HOOK = 1 时要求实现）
  */
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}
