/**
  ******************************************************************************
  * @file    bsp_buzzer.c
  * @brief   蜂鸣器驱动实现：TIM4_CH3 PWM，通过改变频率产生不同音调
  ******************************************************************************
  */

#include "bsp_buzzer.h"

#define BUZZER_FREQ_MIN  100U
#define BUZZER_FREQ_MAX  20000U

void BSP_Buzzer_Init(void)
{
    GPIO_InitTypeDef  gpio = {0};
    TIM_OC_InitTypeDef oc   = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* PD14 -> TIM4_CH3, AF2 */
    gpio.Pin       = GPIO_PIN_14;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOD, &gpio);

    /* 84 MHz -> 1 MHz 计数，频率由 ARR 决定 */
    htim_buzzer.Instance               = TIM4;
    htim_buzzer.Init.Prescaler         = (uint32_t)(APB1_TIM_CLK_HZ / 1000000U) - 1U;
    htim_buzzer.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim_buzzer.Init.Period            = 1000U - 1U;    /* 初始 1 kHz，发声前会被重设 */
    htim_buzzer.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim_buzzer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim_buzzer) != HAL_OK)
    {
        Error_Handler();
    }

    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = 0;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim_buzzer, &oc, TIM_CHANNEL_3) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_TIM_PWM_Start(&htim_buzzer, TIM_CHANNEL_3);

    BSP_Buzzer_Off();
}

void BSP_Buzzer_On(uint32_t freq_hz)
{
    uint32_t arr;

    if (freq_hz < BUZZER_FREQ_MIN)
    {
        freq_hz = BUZZER_FREQ_MIN;
    }
    if (freq_hz > BUZZER_FREQ_MAX)
    {
        freq_hz = BUZZER_FREQ_MAX;
    }

    /* 计数频率 1 MHz，周期 = 1e6 / freq，占空比 50% */
    arr = 1000000U / freq_hz - 1U;
    __HAL_TIM_SET_AUTORELOAD(&htim_buzzer, arr);
    __HAL_TIM_SET_COMPARE(&htim_buzzer, TIM_CHANNEL_3, (arr + 1U) / 2U);
}

void BSP_Buzzer_Off(void)
{
    __HAL_TIM_SET_COMPARE(&htim_buzzer, TIM_CHANNEL_3, 0);
}
