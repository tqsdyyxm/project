/**
  ******************************************************************************
  * @file    bsp_servo.c
  * @brief   舵机驱动实现：TIM1_CH1 50 Hz PWM，脉宽换算角度
  ******************************************************************************
  */

#include "bsp_servo.h"

void BSP_Servo_Init(void)
{
    GPIO_InitTypeDef  gpio = {0};
    TIM_OC_InitTypeDef oc   = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();

    /* PE9 -> TIM1_CH1, AF1 */
    gpio.Pin       = GPIO_PIN_9;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOE, &gpio);

    /* 168 MHz -> 1 MHz 计数，周期 20000 -> 50 Hz */
    htim_servo.Instance               = TIM1;
    htim_servo.Init.Prescaler         = (uint32_t)(APB2_TIM_CLK_HZ / 1000000U) - 1U;
    htim_servo.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim_servo.Init.Period            = (uint32_t)(1000000U / SERVO_PWM_FREQ_HZ) - 1U;
    htim_servo.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim_servo.Init.RepetitionCounter = 0;
    htim_servo.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim_servo) != HAL_OK)
    {
        Error_Handler();
    }

    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = SERVO_PULSE_0DEG_US;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim_servo, &oc, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_TIM_PWM_Start(&htim_servo, TIM_CHANNEL_1);

    /* TIM1 是高级定时器：必须使能主输出，PWM 才会出现在引脚上 */
    __HAL_TIM_MOE_ENABLE(&htim_servo);
}

void BSP_Servo_SetAngle(float deg)
{
    float    pulse_us;
    uint32_t ccr;

    if (deg < 0.0f)
    {
        deg = 0.0f;
    }
    if (deg > 180.0f)
    {
        deg = 180.0f;
    }

    /* 线性换算：0° -> 500us，180° -> 2500us；计数频率 1 MHz，1 计数值 = 1 us */
    pulse_us = (float)SERVO_PULSE_0DEG_US +
               deg * (float)(SERVO_PULSE_180DEG_US - SERVO_PULSE_0DEG_US) / 180.0f;
    ccr = (uint32_t)(pulse_us + 0.5f);

    __HAL_TIM_SET_COMPARE(&htim_servo, TIM_CHANNEL_1, ccr);
}
