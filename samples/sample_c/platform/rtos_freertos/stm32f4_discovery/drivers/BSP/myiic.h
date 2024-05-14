
#ifndef __MYIIC_H
#define __MYIIC_H

#include "stm32f4xx.h"
#include "uart.h"
#include "stm32f4xx_hal.h"
#include "dji_ringbuffer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "osal.h"


/******************************************************************************************/
/* Òý½Å ¶¨Òå */

#define IIC_SCL_GPIO_PORT               GPIOB
#define IIC_SCL_GPIO_PIN                GPIO_PIN_6
#define IIC_SCL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PH¿ÚÊ±ÖÓÊ¹ÄÜ */

#define IIC_SDA_GPIO_PORT               GPIOB
#define IIC_SDA_GPIO_PIN                GPIO_PIN_7
#define IIC_SDA_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PH¿ÚÊ±ÖÓÊ¹ÄÜ */

/******************************************************************************************/

/* IO²Ù×÷ */
#define IIC_SCL(x)        do{ x ? \
                              HAL_GPIO_WritePin(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                          }while(0)       /* SCL */

#define IIC_SDA(x)        do{ x ? \
                              HAL_GPIO_WritePin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                          }while(0)       /* SDA */

#define IIC_READ_SDA     HAL_GPIO_ReadPin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN) /* ¶ÁÈ¡SDA */


/* IICËùÓÐ²Ù×÷º¯Êý */
void iic_init(void);                     /* ³õÊ¼»¯IICµÄIO¿Ú */
void iic_start(void);                    /* ·¢ËÍIIC¿ªÊ¼ÐÅºÅ */
void iic_stop(void);                     /* ·¢ËÍIICÍ£Ö¹ÐÅºÅ */
void iic_ack(void);                      /* IIC·¢ËÍACKÐÅºÅ */
void iic_nack(void);                     /* IIC²»·¢ËÍACKÐÅºÅ */
uint8_t iic_wait_ack(void);              /* IICµÈ´ýACKÐÅºÅ */
void iic_send_byte(uint8_t data);        /* IIC·¢ËÍÒ»¸ö×Ö½Ú */
uint8_t iic_read_byte(unsigned char ack);/* IIC¶ÁÈ¡Ò»¸ö×Ö½Ú */
void delay_us(uint32_t nus);

#endif