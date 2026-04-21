#ifndef __HARDIIC_H__
#define __HARDIIC_H__

#include "sys.h"

void MYDMA_Enable(DMA_Channel_TypeDef *DMA_CHx);
void Hard_I2C_WriteByte(I2C_TypeDef *I2Cx, uint8_t deviceAddress, uint8_t addr, uint8_t data);
void iic1_init(void);
void iic1_dma_config(void);

#endif
