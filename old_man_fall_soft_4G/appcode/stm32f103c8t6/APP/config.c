#include "config.h"
#include "string.h"
#include "stmflash.h"

ADCCOFIG adcconf[ADC_CH_USE_NUMS] = {
   0 // 管脚组	管脚号	管脚组时钟	默认关闭电平
//    {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB, ADC_CH_8},
};



void cfg_load_default(void)
{
    cfg_data.hr_max = HR_MAX;
    cfg_data.hr_min = HR_MIN;
    cfg_data.sqo2_min = SQO2_MIN;
    cfg_data.wendu_max = 36;

    memcpy(cfg_data.phone, PHONE, sizeof(cfg_data.phone));
}

