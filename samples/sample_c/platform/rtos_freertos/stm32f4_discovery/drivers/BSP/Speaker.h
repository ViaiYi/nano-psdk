/**
 ********************************************************************
 * @file    Speaker.h
 * @version V2.0.0
 * @date    2024/1/15
 * @brief   This is the header file for "Speaker.c", defining the structure and
 * (exported) function prototypes.
 *
 * @copyright (c) 2024 Senen. All rights reserved.
 */

#ifndef SPEAKER_H
#define SPEAKER_H

#include "uart.h"
#include "stm32f4xx_hal.h"
#include "dji_ringbuffer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "osal.h"
#include "stm32f4xx.h"
#include <stdio.h>

#define DEBUG_USART_BAUDRATE                    9600




// #if defined ( __CC_ARM   )
// #pragma anon_unions
// #endif
// /******************************* 外部全局变量声明 ***************************/
// #define RX_BUF_MAX_LEN     1024                                     //最大接收缓存字节数
 
// extern struct  STRUCT_USARTx_Fram                                  //串口数据帧的处理结构体
// {
// 	char  Data_RX_BUF [ RX_BUF_MAX_LEN ];
	
//   union {
//     __IO u16 InfAll;
//     struct {
// 		  __IO u16 FramLength       :15;                               // 14:0 
// 		  __IO u16 FramFinishFlag   :1;                                // 15 
// 	  } InfBit;
//   }; 
	
// } strUSART_Fram_Record;

//Òý½Å¶¨Òå
/*******************************************************/
#define DEBUG_USART                             UART4
#define DEBUG_USART_CLK_ENABLE()                __UART4_CLK_ENABLE();

#define RCC_PERIPHCLK_UARTx                     RCC_PERIPHCLK_UART4
#define RCC_UARTxCLKSOURCE_SYSCLK               RCC_UART4CLKSOURCE_SYSCLK

#define DEBUG_USART_RX_GPIO_PORT                GPIOA
#define DEBUG_USART_RX_GPIO_CLK_ENABLE()        __GPIOA_CLK_ENABLE()
#define DEBUG_USART_RX_PIN                      GPIO_PIN_1
#define DEBUG_USART_RX_AF                       GPIO_AF8_UART4

#define DEBUG_USART_TX_GPIO_PORT                GPIOA
#define DEBUG_USART_TX_GPIO_CLK_ENABLE()        __GPIOA_CLK_ENABLE()
#define DEBUG_USART_TX_PIN                      GPIO_PIN_0
#define DEBUG_USART_TX_AF                       GPIO_AF8_UART4

#define DEBUG_USART_IRQHandler                  UART4_IRQHandler
#define DEBUG_USART_IRQ                 		    UART4_IRQn
/************************************************************/

void Usart_SendString(uint8_t *str,uint8_t size);
void DEBUG_USART_Config(void);
void Volinme(uint8_t Y_L);
void talker(uint8_t Y_L);
//int fputc(int ch, FILE *f);
extern UART_HandleTypeDef UartHandle;
#endif /* __USART1_H */





// void SYN6288_GPIO_Config ( void );
// void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);
// void SYN6288_Speech( USART_TypeDef * pUSARTx,char * str );
// void SYN688_USART_Config(void);
// void Usart_SYN6288_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);
// void SYN_FrameInfo(char *HZdata);
// void Usart_SSYN6288_endString( USART_TypeDef * pUSARTx, char *str);
// /* 音量控制 */
// void Volinme(uint8_t Y_L);
// /* 语调控制 */
// void Intonation(uint8_t Y_L);
// /* 语速控制 */
// void Speed_pacing(uint8_t Y_L);
// /* 人控制 */
// void speed_man(uint8_t Y_L);
