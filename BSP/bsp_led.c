/**
  ******************************************************************************
  * @file    bsp_led.c
  * @brief   RGB LED 驱动实现：TIM5 三通道 PWM（1 kHz）
  ******************************************************************************
  */

#include "bsp_led.h"

/* TIM5 三通道对应的 CCR 寄存器（与 Led_Color_t 顺序一致） */
static const uint32_t s_channel[LED_COUNT] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3 };

void BSP_Led_Init(void)
{
    GPIO_InitTypeDef  gpio = {0};
    TIM_OC_InitTypeDef oc  = {0};

    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_TIM5_CLK_ENABLE();

    /* PH10/PH11/PH12 -> TIM5 AF2 */
    gpio.Pin       = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOH, &gpio);

    /* 84 MHz -> 1 MHz 计数，周期 1000 -> 1 kHz PWM */
    htim_led.Instance               = TIM5;
    htim_led.Init.Prescaler         = (uint32_t)(APB1_TIM_CLK_HZ / 1000000U) - 1U;
    htim_led.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim_led.Init.Period            = 1000U - 1U;
    htim_led.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim_led.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim_led) != HAL_OK)
    {
        Error_Handler();
    }

    /* 三个通道：PWM1 模式，高电平为有效电平（高电平点亮） */
    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = 0;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;

    for (uint32_t i = 0; i < LED_COUNT; i++)
    {
        if (HAL_TIM_PWM_ConfigChannel(&htim_led, &oc, s_channel[i]) != HAL_OK)
        {
            Error_Handler();
        }
        HAL_TIM_PWM_Start(&htim_led, s_channel[i]);
    }

    BSP_Led_Off();
}

void BSP_Led_SetDuty(Led_Color_t color, uint32_t duty)
{
    if ((uint32_t)color >= LED_COUNT)
    {
        return;
    }
    if (duty > LED_DUTY_MAX)
    {
        duty = LED_DUTY_MAX;
    }
    __HAL_TIM_SET_COMPARE(&htim_led, s_channel[color], duty);
}

void BSP_Led_SetRgb(uint32_t r, uint32_t g, uint32_t b)
{
    BSP_Led_SetDuty(LED_RED,   r);
    BSP_Led_SetDuty(LED_GREEN, g);
    BSP_Led_SetDuty(LED_BLUE,  b);
}

void BSP_Led_Off(void)
{
    for (uint32_t i = 0; i < LED_COUNT; i++)
    {
        __HAL_TIM_SET_COMPARE(&htim_led, s_channel[i], 0);
    }
}
