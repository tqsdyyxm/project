# ==============================================================================
# RoboMaster C 型开发板（STM32F407IGH6）电控大作业工程
#
# 架构分层：Core(初始化/入口) -> BSP(板级驱动) -> Protocol(通信协议)
#           -> Control(控制算法) -> App(FreeRTOS 任务)
# 依赖库：  Drivers/ (HAL + CMSIS)  Middlewares/ (FreeRTOS + CMSIS-RTOS V2)
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

project(project C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# ---------- 可执行目标与源文件 ----------
add_executable(${PROJECT_NAME}
  # ---- HAL 驱动（按需编译，减小构建时间）----
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c
  # ---- CMSIS ----
  Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
  Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f407xx.s
  # ---- FreeRTOS + CMSIS-RTOS V2 ----
  Middlewares/Third_Party/FreeRTOS/Source/tasks.c
  Middlewares/Third_Party/FreeRTOS/Source/queue.c
  Middlewares/Third_Party/FreeRTOS/Source/list.c
  Middlewares/Third_Party/FreeRTOS/Source/timers.c
  Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
  Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
  Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
  Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
  Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
  # ---- Core：程序入口与初始化 ----
  Core/Src/main.c
  Core/Src/board.c
  Core/Src/stm32f4xx_it.c
  Core/Src/syscalls.c
  # ---- BSP：板级驱动 ----
  BSP/bsp_buzzer.c
  BSP/bsp_led.c
  BSP/bsp_servo.c
  BSP/bsp_uart.c
  BSP/bsp_can.c
  # ---- Protocol：通信协议 ----
  Protocol/justfloat.c
  Protocol/cmd_parse.c
  # ---- Control：控制算法 ----
  Control/pid.c
  Control/motor.c
  # ---- App：FreeRTOS 任务 ----
  App/app_tasks.c
  App/task_buzzer.c
  App/task_led.c
  App/task_servo.c
  App/task_synex.c
  App/task_motor.c
)

# ---------- 头文件搜索路径 ----------
target_include_directories(${PROJECT_NAME} PRIVATE
  Core/Inc
  Drivers/CMSIS/Core/Include
  Drivers/CMSIS/Device/ST/STM32F4xx/Include
  Drivers/CMSIS/RTOS2/Include
  Drivers/STM32F4xx_HAL_Driver/Inc
  Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
  Middlewares/Third_Party/FreeRTOS/Source/include
  Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
  Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
  BSP
  Protocol
  Control
  App
)

# ---------- 宏与编译选项 ----------
target_compile_definitions(${PROJECT_NAME} PRIVATE
  STM32F407xx
  USE_HAL_DRIVER
  CMSIS_device_header=\"stm32f4xx.h\"
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall
  -Wno-unused-parameter
)

# ---------- 链接选项 ----------
target_link_options(${PROJECT_NAME} PRIVATE
  -T${CMAKE_CURRENT_SOURCE_DIR}/stm32f407xg_flash.ld
  --specs=nano.specs
  -Wl,--gc-sections
  -Wl,--no-warn-rwx-segments
  -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
  -u _printf_float
)

# 产物统一命名为 project.elf（烧录任务与调试配置引用此文件名）
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

# ---------- 编译后生成 bin/hex 并输出占用 ----------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
  COMMENT "生成 project.hex / project.bin"
)
