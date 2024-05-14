#include "myiic.h"

#define PCA_Addr 0x80
#define PCA_Model 0x00
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09
#define PCA_Pre 0xFE

void PCA9685_Init(float hz,uint8_t angle);

void PCA9685_Write(uint8_t addr,uint8_t data);

uint8_t PCA9685_Read(uint8_t addr);

void PCA9685_setPWM(uint8_t num,uint32_t on,uint32_t off);

void PCA9685_setFreq(float freq);

void setAngle(uint8_t num,uint8_t angle);
