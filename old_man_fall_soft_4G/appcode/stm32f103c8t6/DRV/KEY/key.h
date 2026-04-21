#ifndef __KEY_H
#define __KEY_H
#include "sys.h"
#include "flexible_button.h"


typedef enum KEYNUM
{
    KEY1,
    KEY2,
    KEY3,
    KEY4,
    KEY5,
    KEYMAX,
} KEYNUM;

typedef struct KEYCOFIG
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    uint32_t RCC_APB2Periph;
} KEYCOFIG;

typedef struct KEYSTRUCT
{
    u8 initflag;
    void (*key_init)(void); // 初始化
    void (*key_scan)(void);  // 回调
}KEY;


extern KEY gkey;

#endif
