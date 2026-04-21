#ifndef __LED_H
#define __LED_H
#include "sys.h"

// #define LED1 PCout(13)  // PC13 

void LED_Init(void);//初始化

typedef enum LEDNUM
{
    LED1,
    LED2,
		LED3,
    LEDMAX,
}LEDNUM;



typedef struct LEDCOFIG
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    uint32_t RCC_APB2Periph;
    uint8_t ledOffState;
}LEDCOFIG;



typedef struct LEDSTRUCT
{
    uint8_t led_state[LEDMAX];                                                          // 记录LED状态
    void (*led_init)(void); // 初始化
    void (*led_open)(LEDNUM ledx);                                                            // 开灯
    void (*led_close)(LEDNUM ledx);                                                           // 关灯
    void (*led_toggle)(LEDNUM ledx);                                                          // 状态反转
}LED;


extern LED gled;


#endif
