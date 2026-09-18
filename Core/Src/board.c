/**
  ******************************************************************************
  * @file    board.c
  * @brief   系统时钟配置（168 MHz）与 HAL 时基（TIM6）
  *
  * 时钟树（12 MHz HSE）：
  *   PLLM=12 -> VCO 输入 1 MHz；PLLN=336 -> VCO 336 MHz；
  *   PLLP=2  -> SYSCLK 168 MHz；PLLQ=7 -> USB 48 MHz；
  *   AHB /1 = 168 MHz；APB1 /4 = 42 MHz（定时器 x2 = 84 MHz）；
  *   APB2 /2 = 84 MHz（定时器 x2 = 168 MHz）。
  ******************************************************************************
  */

#include "board.h"

TIM_HandleTypeDef htim_timebase;
TIM_HandleTypeDef htim_buzzer;
TIM_HandleTypeDef htim_led;
TIM_HandleTypeDef htim_servo;
UART_HandleTypeDef huart_synex;
CAN_HandleTypeDef  hcan_motor;

/**
  * @brief  配置系统时钟，并把 HAL 时基从 SysTick 切换为 TIM6。
  *         （SysTick 之后由 FreeRTOS 调度器使用）
  */
void Board_ClockInit(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* ---- 振荡器与 PLL ---- */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM       = 12;
    osc.PLL.PLLN       = 336;
    osc.PLL.PLLP       = RCC_PLLP_DIV2;
    osc.PLL.PLLQ       = 7;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    /* ---- 总线分频 ---- */
    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV4;
    clk.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }

    /* ---- HAL 时基：TIM6 输出 1 kHz 中断 ---- */
    __HAL_RCC_TIM6_CLK_ENABLE();

    htim_timebase.Instance               = TIM6;
    htim_timebase.Init.Prescaler         = (uint32_t)(APB1_TIM_CLK_HZ / 1000000U) - 1U;
    htim_timebase.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim_timebase.Init.Period            = 1000U - 1U;
    htim_timebase.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim_timebase) != HAL_OK)
    {
        Error_Handler();
    }

    /* 优先级 15（最低），避免影响 FreeRTOS 可屏蔽的中断 */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    HAL_TIM_Base_Start_IT(&htim_timebase);
}

/**
  * @brief  HAL 定时器周期回调：TIM6 每 1 ms 调用一次 HAL_IncTick，
  *          为 HAL_Delay/HAL_GetTick 提供时间基准。
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        HAL_IncTick();
    }
}
