/**
  ******************************************************************************
  * @file    main.h
  * @brief   工程公共头文件：统一包含 HAL 库
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
