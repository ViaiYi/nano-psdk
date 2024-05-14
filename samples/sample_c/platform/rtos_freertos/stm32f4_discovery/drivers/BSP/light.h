/**
 ********************************************************************
 * @file    light.h
 * @version V2.0.0
 * @date    2024/1/15
 * @brief   This is the header file for "light.c", defining the structure and
 * (exported) function prototypes.
 *
 * @copyright (c) 2024 Senen. All rights reserved.
 */

#ifndef LIGHT_H
#define LIGHT_H

#include "uart.h"
#include "stm32f4xx_hal.h"
#include "dji_ringbuffer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "osal.h"
#include "stm32f4xx.h"
#include <stdio.h>

#define LIGHT_GPIO_PIN                      GPIO_PIN_10
#define LIGHT_GPIO_PORT                     GPIOA
#define LIGHT_GPIO_CLK_ENABLE()             __HAL_RCC_GPIOA_CLK_ENABLE()

void LIGHT_Init(void);
void LIGHT_On(void);
void LIGHT_Off(void);
void LIGHT_Trigger(void);


#endif /* __USART1_H */