/**
 ********************************************************************
 * @file    PWM.h
 * @version V2.0.0
 * @date    2024/1/15
 * @brief   This is the header file for "PWM.c", defining the structure and
 * (exported) function prototypes.
 *
 * @copyright (c) 2024 Senen. All rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PWM_H
#define PWM_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "uart.h"
#include "stm32f4xx_hal.h"
#include "dji_ringbuffer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "osal.h"
/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
//#include "sys.h"


/****************************************************************************************************/
/* 通用定时器 定义 */

/* TIMX 中断定义 
 * 默认是针对TIM2~TIM5
 * 注意: 通过修改这4个宏定义,可以支持TIM1~TIM8任意一个定时器.
 */
 
// #define GTIM_TIMX_INT                       TIM3
// #define GTIM_TIMX_INT_IRQn                  TIM3_IRQn
// #define GTIM_TIMX_INT_IRQHandler            TIM3_IRQHandler
// #define GTIM_TIMX_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM3_CLK_ENABLE(); }while(0)   /* TIM3 时钟使能 */

/*********************************以下是通用定时器PWM输出实验相关宏定义*************************************/

/* TIMX PWM输出定义 
 * 这里输出的PWM控制LED0(RED)的亮度
 * 默认是针对TIM2~TIM5
 * 注意: 通过修改这几个宏定义,可以支持TIM1~TIM8任意一个定时器,任意一个IO口输出PWM
 */
// #define GTIM_TIMX_PWM1_CHY_GPIO_PORT         GPIOA
// #define GTIM_TIMX_PWM1_CHY_GPIO_PIN          GPIO_PIN_7
// #define GTIM_TIMX_PWM1_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)  /* PA口时钟使能 */
// #define GTIM_TIMX_PWM1_CHY_GPIO_AF           GPIO_AF9_TIM14                               /* 端口复用到TIM14 */

// #define GTIM_TIMX_PWM2_CHY_GPIO_PORT         GPIOA
// #define GTIM_TIMX_PWM2_CHY_GPIO_PIN          GPIO_PIN_6
// #define GTIM_TIMX_PWM2_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)  /* PA口时钟使能 */
// #define GTIM_TIMX_PWM2_CHY_GPIO_AF           GPIO_AF9_TIM13                               /* 端口复用到TIM14 */

// #define GTIM_TIMX_PWM3_CHY_GPIO_PORT         GPIOC
// #define GTIM_TIMX_PWM3_CHY_GPIO_PIN          GPIO_PIN_6
// #define GTIM_TIMX_PWM3_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)  /* PA口时钟使能 */
// #define GTIM_TIMX_PWM3_CHY_GPIO_AF           GPIO_AF3_TIM8                               /* 端口复用到TIM8 */

/*TIMX PWM定义输出*/
// #define GTIM_TIMX_PWM1_CHY_GPIO_PORT         GPIOA
// #define GTIM_TIMX_PWM1_CHY_GPIO_PIN          GPIO_PIN_6
// #define GTIM_TIMX_PWM1_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)  /* PA口时钟使能 */
// #define GTIM_TIMX_PWM1_CHY_GPIO_AF           GPIO_AF9_TIM13                               /* 端口复用到TIM13 */


// /* TIMX REMAP设置
//  * 因为我们LED0接在PF9上, 必须通过开启TIM14的部分重映射功能, 才能将TIM14_CH1输出到PF9上
//  */

// // #define GTIM_TIMX_PWM1                       TIM14                                        /* TIM14 */
// // #define GTIM_TIMX_PWM1_CHY                   TIM_CHANNEL_1                                /* 通道Y,  1<= Y <=4 */
// // #define GTIM_TIMX_PWM1_CHY_CCRX              TIM14->CCR1                                  /* 通道Y的输出比较寄存器 */
// // #define GTIM_TIMX_PWM1_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM14_CLK_ENABLE(); }while(0)  /* TIM14 时钟使能 */

// // #define GTIM_TIMX_PWM2                       TIM13                                        /* TIM14 */
// // #define GTIM_TIMX_PWM2_CHY                   TIM_CHANNEL_1                                /* 通道Y,  1<= Y <=4 */
// // #define GTIM_TIMX_PWM2_CHY_CCRX              TIM13->CCR1                                  /* 通道Y的输出比较寄存器 */
// // #define GTIM_TIMX_PWM2_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM13_CLK_ENABLE(); }while(0)  /* TIM14 时钟使能 */

// // #define GTIM_TIMX_PWM3                       TIM8                                        /* TIM14 */
// // #define GTIM_TIMX_PWM3_CHY                   TIM_CHANNEL_1                                /* 通道Y,  1<= Y <=4 */
// // #define GTIM_TIMX_PWM3_CHY_CCRX              TIM8->CCR1                                  /* 通道Y的输出比较寄存器 */
// // #define GTIM_TIMX_PWM3_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM8_CLK_ENABLE(); }while(0)  /* TIM14 时钟使能 */

// /*TIM设置*/
// #define GTIM_TIMX_PWM1                       TIM13                                        /* TIM13 */
// #define GTIM_TIMX_PWM1_CHY                   TIM_CHANNEL_1                                /* 通道Y,  1<= Y <=4 */
// #define GTIM_TIMX_PWM1_CHY_CCRX              TIM13->CCR1                                  /* 通道Y的输出比较寄存器 */
// #define GTIM_TIMX_PWM1_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM13_CLK_ENABLE(); }while(0)  /* TIM13 时钟使能 */


// // #define GTIM_TIMX_PWM_CHY_GPIO_PORT         GPIOA
// // #define GTIM_TIMX_PWM_CHY_GPIO_PIN          GPIO_PIN_6
// // #define GTIM_TIMX_PWM_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)  /* PF????? */
// // #define GTIM_TIMX_PWM_CHY_GPIO_AF           GPIO_AF9_TIM13                               /* ?????TIM10,??? */

// // #define GTIM_TIMX_PWM                       TIM13                                        /* TIMx */
// // #define GTIM_TIMX_PWM_CHY                   TIM_CHANNEL_1                                /* ??Y,  1<= Y <=4 */
// // #define GTIM_TIMX_PWM_CHY_CCRX              TIM13->CCR1                                  /* ??Y???????? */
// // #define GTIM_TIMX_PWM_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM13_CLK_ENABLE(); }while(0)  /* TIMx ???? */

// void gtim_timx_pwm1_chy_init(uint16_t arr, uint16_t psc);    /* ????? PWM????? */

#define GTIM_TIMX_PWM_CHY_GPIO_PORT         GPIOA
#define GTIM_TIMX_PWM_CHY_GPIO_PIN          GPIO_PIN_6
#define GTIM_TIMX_PWM_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)  /* PF????? */
#define GTIM_TIMX_PWM_CHY_GPIO_AF           GPIO_AF9_TIM13                               /* ?????TIM10,??? */

#define GTIM_TIMX_PWM                       TIM13                                        /* TIMx */
#define GTIM_TIMX_PWM_CHY                   TIM_CHANNEL_1                                /* ??Y,  1<= Y <=4 */
#define GTIM_TIMX_PWM_CHY_CCRX              TIM13->CCR1                                  /* ??Y???????? */
#define GTIM_TIMX_PWM_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM13_CLK_ENABLE(); }while(0)  /* TIMx ???? */

void gtim_timx_pwm_chy_init(uint16_t arr, uint16_t psc);    /* ????? PWM????? */



/****************************************************************************************************/

// void gtim_timx_motor1_chy_init(uint16_t arr, uint16_t psc);        /* 通用定时器 定时中断初始化函数 */
// void gtim_timx_motor2_chy_init(uint16_t arr, uint16_t psc);    /* 通用定时器 PWM初始化函数 */
// void gtim_timx_motor3_chy_init(uint16_t arr, uint16_t psc);




/* Exported functions --------------------------------------------------------*/
//void gtim_timx_int_init(uint16_t arr, uint16_t psc);

#endif // PWM_H
