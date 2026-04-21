#ifndef   __CONFIG_H_
#define   __CONFIG_H_

#include "sys.h"
#include "adc.h"
#include "stmflash.h"

#define Oneline_H GPIO_SetBits(GPIOA, GPIO_Pin_7)
#define Oneline_L GPIO_ResetBits(GPIOA, GPIO_Pin_7)


extern void cfg_load_default(void);

extern ADCCOFIG adcconf[];

#endif


