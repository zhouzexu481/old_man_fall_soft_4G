#include "led.h"
#include "string.h"

static void Open(LEDNUM ledx);
static void Close(LEDNUM ledx);
static void led_toggle(LEDNUM ledx);
static void Init(void);

LED gled = {.led_init = Init,
			.led_open = Open,
			.led_close = Close,
			.led_toggle = led_toggle
			};


static  LEDCOFIG ledconf[LEDMAX] = {
	// 管脚组	管脚号	管脚组时钟	默认关闭电平
	{GPIOC, GPIO_Pin_13, RCC_APB2Periph_GPIOC, 1},
	{GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOB, 0},
	{GPIOA, GPIO_Pin_15, RCC_APB2Periph_GPIOB, 0},
};


static void Open(LEDNUM ledx)
{

	switch (ledx)
	{
	case LED1:
		(ledconf[ledx].ledOffState == 0) ?  GPIO_SetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin):
									  		GPIO_ResetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin);

		gled.led_state[LED1] = 1;
		break;

	case LED2:
		(ledconf[ledx].ledOffState == 0) ?  GPIO_SetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin):
									  		GPIO_ResetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin);

		gled.led_state[LED1] = 1;
	
	case LED3:
		(ledconf[ledx].ledOffState == 0) ?  GPIO_SetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin):
									  		GPIO_ResetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin);

		gled.led_state[LED3] = 1;

		break;

	default:
		break;
	}
}


static void Close(LEDNUM ledx)
{

	switch (ledx)
	{
	case LED1:
		(ledconf[ledx].ledOffState == 0) ? GPIO_ResetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin) :
									  GPIO_SetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin);

		gled.led_state[LED1] = 0;
		break;

	case LED2:
		(ledconf[ledx].ledOffState == 0) ? GPIO_ResetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin) :
									  GPIO_SetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin);

		gled.led_state[LED1] = 0;
		break;
	
	case LED3:
		(ledconf[ledx].ledOffState == 0) ? GPIO_ResetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin) :
									  GPIO_SetBits(ledconf[ledx].GPIOx, ledconf[ledx].GPIO_Pin);

		gled.led_state[LED3] = 0;
		break;

	default:
		break;
	}
}

static void led_toggle(LEDNUM ledx)
{
	if (gled.led_state[LED1] == 0)		// 表示关闭
	{
		Open(ledx);
	}
	else if (gled.led_state[LED1] == 1)
	{
		Close(ledx);
	}
}

static void Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8_t i = 0;

	for (i = 0; i < LEDMAX; i++)
	{
		memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitTypeDef));

		RCC_APB2PeriphClockCmd(ledconf[i].RCC_APB2Periph, ENABLE);
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

		GPIO_InitStructure.GPIO_Pin = ledconf[i].GPIO_Pin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
		GPIO_Init(ledconf[i].GPIOx, &GPIO_InitStructure);	  // 根据设定参数初始化

		if (ledconf[i].ledOffState == 1) // 高电平为关闭状态
		{
			GPIO_SetBits(ledconf[i].GPIOx, ledconf[i].GPIO_Pin);
		}
		else if (ledconf[i].ledOffState == 0)
		{
			GPIO_ResetBits(ledconf[i].GPIOx, ledconf[i].GPIO_Pin);
		}
		else
		{
			ledconf[i].ledOffState = 0;
			GPIO_ResetBits(ledconf[i].GPIOx, ledconf[i].GPIO_Pin);
		}

		gled.led_state[i] = 0;
	}
}
