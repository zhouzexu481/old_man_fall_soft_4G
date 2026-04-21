#ifndef __ADC_H_
#define __ADC_H_

#include "sys.h"


typedef enum
{
    ADC_CH_6 = 0,       // MQ-2
    ADC_CH_USE_NUMS
} ae_adc_usech_t;

typedef struct ADCCOFIG
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    uint32_t RCC_APB2Periph;
    ae_adc_usech_t adc_ch;
} ADCCOFIG;

void Adc_Init(void);
void adc1_Start(void);
u16 Get_Adc_Average(u8 ch,u8 times);


extern uint16_t g_adc_value[ADC_CH_USE_NUMS];

#endif


