/**
  ******************************************************************************
  * @file    FreeRTOSConfig.h
  * @brief   FreeRTOS V10.3.1 + CMSIS-RTOS V2 配置（STM32F407 / 168MHz）
  *          本文件按 ST 官方模板整理，配合 Middlewares/Third_Party/FreeRTOS 使用。
  ******************************************************************************
  */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>
extern uint32_t SystemCoreClock;

/*----------------------------- 基本调度配置 --------------------------------*/
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    56
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)
#define configTOTAL_HEAP_SIZE                   ((size_t)15360)
#define configMAX_TASK_NAME_LEN                 (16)
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

/*----------------------------- 内存分配 --------------------------------*/
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         1
#define configUSE_MUTEXES                       1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/*----------------------------- Hook 回调 --------------------------------*/
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0

/*----------------------------- 定时器与队列 ------------------------------*/
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               2
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256

/*----------------------------- 通知 --------------------------------*/
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/*----------------------------- 协程（本工程不用，关闭）--------------------*/
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         2

/*----------------------------- 中断优先级 --------------------------------*/
/*
 * Cortex-M4 使用 4 位优先级。configMAX_SYSCALL_INTERRUPT_PRIORITY 的值必须
 * 是"左移后"的形式，因此 5 表示 0x50。
 * 外设中断（串口/CAN/定时器）必须配置为数值 >= 5 的优先级（即优先级更低），
 * 才能在中断服务函数中调用 FreeRTOS 的 FromISR API。
 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY                 ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY            ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS            __NVIC_PRIO_BITS
#else
#define configPRIO_BITS            4
#endif

/*----------------------------- 断言 --------------------------------*/
#define configASSERT( x ) if ((x) == 0) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/*----------------------------- FreeRTOS 与 CMSIS 中断映射 -----------------*/
/*
 * SysTick_Handler 由 cmsis_os2.c 提供（内部调用 xPortSysTickHandler），
 * 因此这里不再映射 SysTick，避免重复定义。
 */
#define xPortPendSVHandler     PendSV_Handler
#define vPortSVCHandler        SVC_Handler

/*----------------------------- 可选 API 裁剪 -----------------------------*/
#define INCLUDE_vTaskPrioritySet             1
#define INCLUDE_uxTaskPriorityGet            1
#define INCLUDE_vTaskDelete                  1
#define INCLUDE_vTaskSuspend                 1
#define INCLUDE_xResumeFromISR               1
#define INCLUDE_vTaskDelayUntil              1
#define INCLUDE_vTaskDelay                   1
#define INCLUDE_xTaskGetSchedulerState       1
#define INCLUDE_xTaskGetCurrentTaskHandle    1
#define INCLUDE_uxTaskGetStackHighWaterMark  1
#define INCLUDE_xTaskGetIdleTaskHandle       1
#define INCLUDE_eTaskGetState                1
#define INCLUDE_xEventGroupSetBitFromISR     1
#define INCLUDE_xTimerPendFunctionCall       1
#define INCLUDE_xTaskAbortDelay              1
#define INCLUDE_xTaskGetHandle               1
#define INCLUDE_xTaskResumeFromISR           1
#define INCLUDE_xSemaphoreGetMutexHolder     1

#endif /* FREERTOS_CONFIG_H */
