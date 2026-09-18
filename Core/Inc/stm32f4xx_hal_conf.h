/**
  ******************************************************************************
  * @file    stm32f4xx_hal_conf.h
  * @brief   HAL 模块裁剪配置（本工程只启用需要的模块）
  *          板载晶振 HSE = 12 MHz，系统主频 168 MHz。
  ******************************************************************************
  */

#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

/*--------------------------- 已启用的 HAL 模块 ----------------------------*/
#define HAL_MODULE_ENABLED
#define HAL_CAN_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_USART_MODULE_ENABLED

/* 以下模块本工程未使用，保持关闭以减小代码体积：
#define HAL_ADC_MODULE_ENABLED
#define HAL_CEC_MODULE_ENABLED
#define HAL_CRC_MODULE_ENABLED
#define HAL_CRYP_MODULE_ENABLED
#define HAL_DAC_MODULE_ENABLED
#define HAL_DCMI_MODULE_ENABLED
#define HAL_DMA2D_MODULE_ENABLED
#define HAL_ETH_MODULE_ENABLED
#define HAL_FMPI2C_MODULE_ENABLED
#define HAL_FMC_MODULE_ENABLED
#define HAL_HASH_MODULE_ENABLED
#define HAL_HCD_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_I2S_MODULE_ENABLED
#define HAL_IRDA_MODULE_ENABLED
#define HAL_IWDG_MODULE_ENABLED
#define HAL_LTDC_MODULE_ENABLED
#define HAL_NAND_MODULE_ENABLED
#define HAL_NOR_MODULE_ENABLED
#define HAL_PCCARD_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
#define HAL_RNG_MODULE_ENABLED
#define HAL_RTC_MODULE_ENABLED
#define HAL_SAI_MODULE_ENABLED
#define HAL_SD_MODULE_ENABLED
#define HAL_SDRAM_MODULE_ENABLED
#define HAL_SMARTCARD_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_SRAM_MODULE_ENABLED
#define HAL_WWDG_MODULE_ENABLED
*/

/*--------------------------- 振荡器配置 ----------------------------*/
#if !defined  (HSE_VALUE)
  #define HSE_VALUE    ((uint32_t)12000000U)   /*!< 板载外部晶振 12 MHz */
#endif /* HSE_VALUE */

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    ((uint32_t)100U)
#endif /* HSE_STARTUP_TIMEOUT */

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)16000000U)   /*!< 内部高速时钟 16 MHz */
#endif /* HSI_VALUE */

#if !defined  (LSI_VALUE)
  #define LSI_VALUE    ((uint32_t)32000U)      /*!< 内部低速时钟 32 kHz */
#endif /* LSI_VALUE */

#if !defined  (LSE_VALUE)
  #define LSE_VALUE    ((uint32_t)32768U)      /*!< 外部低速时钟 32.768 kHz */
#endif /* LSE_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    ((uint32_t)5000U)
#endif /* LSE_STARTUP_TIMEOUT */

#if !defined  (VDD_VALUE)
  #define VDD_VALUE    ((uint32_t)3300U)       /*!< 供电电压 3.3 V */
#endif /* VDD_VALUE */

#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    ((uint32_t)12288000U) /*!< 外部扩展时钟（本工程不使用） */
#endif /* EXTERNAL_CLOCK_VALUE */

/*--------------------------- 系统配置 ----------------------------*/
#define  TICK_INT_PRIORITY             ((uint32_t)0x0FU) /*!< HAL 时基(TIM6)中断优先级 = 15(最低) */
#define  PREFETCH_ENABLE               1U               /*!< 使能 Flash 预取 */
#define  INSTRUCTION_CACHE_ENABLE      0U               /*!< F4 无指令缓存 */
#define  DATA_CACHE_ENABLE             0U               /*!< F4 无数据缓存 */

/*--------------------------- 模块回调注册（保持关闭） ---------------------*/
#define  USE_HAL_ADC_REGISTER_CALLBACKS         0U
#define  USE_HAL_CAN_REGISTER_CALLBACKS         0U
#define  USE_HAL_CEC_REGISTER_CALLBACKS         0U
#define  USE_HAL_DAC_REGISTER_CALLBACKS         0U
#define  USE_HAL_DCMI_REGISTER_CALLBACKS        0U
#define  USE_HAL_DMA2D_REGISTER_CALLBACKS       0U
#define  USE_HAL_ETH_REGISTER_CALLBACKS         0U
#define  USE_HAL_HCD_REGISTER_CALLBACKS         0U
#define  USE_HAL_I2C_REGISTER_CALLBACKS         0U
#define  USE_HAL_I2S_REGISTER_CALLBACKS         0U
#define  USE_HAL_IRDA_REGISTER_CALLBACKS        0U
#define  USE_HAL_LTDC_REGISTER_CALLBACKS        0U
#define  USE_HAL_NAND_REGISTER_CALLBACKS        0U
#define  USE_HAL_NOR_REGISTER_CALLBACKS         0U
#define  USE_HAL_PCCARD_REGISTER_CALLBACKS      0U
#define  USE_HAL_PCD_REGISTER_CALLBACKS         0U
#define  USE_HAL_RNG_REGISTER_CALLBACKS         0U
#define  USE_HAL_RTC_REGISTER_CALLBACKS         0U
#define  USE_HAL_SAI_REGISTER_CALLBACKS         0U
#define  USE_HAL_SD_REGISTER_CALLBACKS          0U
#define  USE_HAL_SDRAM_REGISTER_CALLBACKS       0U
#define  USE_HAL_SMARTCARD_REGISTER_CALLBACKS   0U
#define  USE_HAL_SPI_REGISTER_CALLBACKS         0U
#define  USE_HAL_SRAM_REGISTER_CALLBACKS        0U
#define  USE_HAL_TIM_REGISTER_CALLBACKS         0U
#define  USE_HAL_UART_REGISTER_CALLBACKS        0U
#define  USE_HAL_USART_REGISTER_CALLBACKS       0U
#define  USE_HAL_WWDG_REGISTER_CALLBACKS        0U

/*--------------------------- 断言 ----------------------------*/
/*
 * 按 ST 官方模板：USE_FULL_ASSERT 保持"未定义"即关闭断言；
 * 需要完整参数检查时取消下面一行注释（并实现 assert_failed 函数）。
 */
/* #define USE_FULL_ASSERT    1U */

#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

/*--------------------------- 外设驱动头文件 ----------------------------*/
#include "stm32f4xx_hal_can.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_pwr_ex.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_rcc_ex.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_tim_ex.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_usart.h"

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_CONF_H */
