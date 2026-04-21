#include "stm32f10x.h"
#include "key.h"
#include "sys.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"
#include "hmi_common.h"



#define ENUM_TO_STR(e) (#e)

static void init(void);
static void key_scan(void);

static flex_button_t user_button[KEYMAX];


static char *enum_event_string[] = {
    ENUM_TO_STR(FLEX_BTN_PRESS_DOWN),
    ENUM_TO_STR(FLEX_BTN_PRESS_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_DOUBLE_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_REPEAT_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_SHORT_START),
    ENUM_TO_STR(FLEX_BTN_PRESS_SHORT_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_START),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_HOLD),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_HOLD_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_MAX),
    ENUM_TO_STR(FLEX_BTN_PRESS_NONE),
};



static  KEYCOFIG keyconf[KEYMAX] = {
	// 管脚组	管脚号	管脚组时钟
	{GPIOB, GPIO_Pin_12, RCC_APB2Periph_GPIOB,},
	{GPIOB, GPIO_Pin_13, RCC_APB2Periph_GPIOB,},
	{GPIOB, GPIO_Pin_14, RCC_APB2Periph_GPIOB,},
	{GPIOB, GPIO_Pin_15, RCC_APB2Periph_GPIOB,},
    {GPIOA, GPIO_Pin_8,  RCC_APB2Periph_GPIOA,},
};

KEY gkey = {.initflag = 0,
            .key_init = init,
			.key_scan = key_scan,
			};


static uint8_t common_btn_read(void *arg)
{
    uint8_t value = 0;

    flex_button_t *btn = (flex_button_t *)arg;

    value = GPIO_ReadInputDataBit(keyconf[btn->id].GPIOx, keyconf[btn->id].GPIO_Pin);

    return value;
}


static void common_btn_evt_cb(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;

    // printf("id:[%d]event:[%d-%20s]repeat:%d\r\n", 
        // btn->id,
        // btn->event, enum_event_string[btn->event],
        // btn->click_cnt);

        // 将按键信息传给ui菜单
        menu_key_scan_cb(btn->id, btn->event);

    // if ((flex_button_event_read(&user_button[USER_BUTTON_1]) == FLEX_BTN_PRESS_CLICK) &&\
    //     (flex_button_event_read(&user_button[USER_BUTTON_2]) == FLEX_BTN_PRESS_CLICK))
    // {
    //     printf("[combination]: button 1 and button 2\r\n");
    // }
}


// 按键初始化函数
static void init(void) // IO初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;

	uint8_t i = 0;

	for (i = 0; i < KEYMAX; i++)
	{
		memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitTypeDef));
		RCC_APB2PeriphClockCmd(keyconf[i].RCC_APB2Periph, ENABLE);
		GPIO_InitStructure.GPIO_Pin = keyconf[i].GPIO_Pin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	  // 上拉输入
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
		GPIO_Init(keyconf[i].GPIOx, &GPIO_InitStructure); // 根据设定参数初始化
	}

	for (i = 0; i < KEYMAX; i ++)
    {
        user_button[i].id = i;
        user_button[i].usr_button_read = common_btn_read;
        user_button[i].cb = common_btn_evt_cb;
        user_button[i].pressed_logic_level = 0;
        user_button[i].short_press_start_tick = FLEX_MS_TO_SCAN_CNT(1500);
        user_button[i].long_press_start_tick = FLEX_MS_TO_SCAN_CNT(3000);
        user_button[i].long_hold_start_tick = FLEX_MS_TO_SCAN_CNT(4500);

        flex_button_register(&user_button[i]);
    }

    gkey.initflag = 1;

}

static void key_scan(void)
{
    if (gkey.initflag == 0)
    {
        return;
    }

	flex_button_scan();
}
