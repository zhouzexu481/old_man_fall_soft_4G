#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sys.h"  

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

uint32_t systicks_get(void);

uint32_t millis_elapsed(uint32_t n);

#endif





























