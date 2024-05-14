#include "pca9685.h"
//#include "myiic.h"
//#include "delay.h"
#include <math.h>

void PCA9685_Init(float hz,uint8_t angle)
{
	uint32_t off = 0;
	iic_init();
	PCA9685_Write(PCA_Model,0x00);
	PCA9685_setFreq(hz);
	off = (uint32_t)(103+angle*2.28);
	PCA9685_setPWM(0,0,off);
	PCA9685_setPWM(1,0,off);
	PCA9685_setPWM(2,0,off);
	PCA9685_setPWM(3,0,off);
	PCA9685_setPWM(4,0,off);
	PCA9685_setPWM(5,0,off);
	PCA9685_setPWM(6,0,off);
	PCA9685_setPWM(7,0,off);
	PCA9685_setPWM(8,0,off);
	PCA9685_setPWM(9,0,off);
	PCA9685_setPWM(10,0,off);
	PCA9685_setPWM(11,0,off);
	PCA9685_setPWM(12,0,off);
	PCA9685_setPWM(13,0,off);
	PCA9685_setPWM(14,0,off);
	PCA9685_setPWM(15,0,off);

	vTaskDelay(100);
	
}

void PCA9685_Write(uint8_t addr,uint8_t data)
{
	iic_start();
	
	iic_send_byte(PCA_Addr);
	iic_nack();
	
	iic_send_byte(addr);
	iic_nack();
	
	iic_send_byte(data);
	iic_nack();
	
	iic_stop();
	
	
}

uint8_t PCA9685_Read(uint8_t addr)
{
	uint8_t data;
	
	iic_start();
	
	iic_send_byte(PCA_Addr);
	iic_nack();
	
	iic_send_byte(addr);
	iic_nack();
	
	iic_stop();
	
    delay_us(10);
	//delay_us(10);

	
	iic_start();

	iic_send_byte(PCA_Addr|0x01);
	iic_nack();
	
	data = iic_read_byte(0);
	
	iic_stop();
	
	return data;
	
}

void PCA9685_setPWM(uint8_t num,uint32_t on,uint32_t off)
{
	iic_start();
	
	iic_send_byte(PCA_Addr);
	iic_wait_ack();
	
	iic_send_byte(LED0_ON_L+4*num);
	iic_wait_ack();
	
	iic_send_byte(on&0xFF);
	iic_wait_ack();
	
	iic_send_byte(on>>8);
	iic_wait_ack();
	
	iic_send_byte(off&0xFF);
	iic_wait_ack();
	
	iic_send_byte(off>>8);
	iic_wait_ack();
	
	iic_stop();
	
}

void PCA9685_setFreq(float freq)
{
	uint8_t prescale,oldmode,newmode;
	
	double prescaleval;
	
	freq *= 0.98;
	prescaleval = 25000000;
	prescaleval /= 4096;
	prescaleval /= freq;
	prescaleval -= 1;
	prescale = floor(prescaleval+0.5f);
	oldmode = PCA9685_Read(PCA_Model);
	
	newmode = (oldmode&0x7F)|0x10;
	PCA9685_Write(PCA_Model,newmode);
	PCA9685_Write(PCA_Pre,prescale);
	PCA9685_Write(PCA_Model,oldmode);
	vTaskDelay(5);
	PCA9685_Write(PCA_Model,oldmode|0xa1);
	
	
}

void setAngle(uint8_t num,uint8_t angle)
{
	uint32_t off = 0;
	off = (uint32_t)(103+angle*2.28);
	PCA9685_setPWM(num,0,off);
}



