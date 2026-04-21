#include "delay.h"
#include "key.h"
#include "gizwits_product.h"

#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(uint8_t  *)(0xE000ED23UL))

#define SYS_TIMER_PPMS 2
#define SYS_TICK_TCNT 72000


//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
#define SYSTICKSLOAD (72000000 / 1000)


static uint32_t systicks = 0;


//初始化延迟函数
void delay_init(void)
{
	if ((SYSTICKSLOAD - 1) > 0xFFFFFF)
	{
		return;
	}

	_SYSTICK_LOAD = SYSTICKSLOAD - 1;
	_SYSTICK_PRI = 0xFF;
	_SYSTICK_VAL = 0;
	_SYSTICK_CTRL = 0x07;

	return;
}


uint32_t systicks_get(void)
{
	return systicks;
}




void SysTick_Handler(void)
{
	static u8 key_tick = 0;
	systicks++;
	
	gizTimerMs();
	
  if (key_tick++ > 10)
    {
        key_tick = 0;

        gkey.key_scan();
    }
}



void delay_us(uint32_t us)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;

    /* 获得延时经过的 tick 数 */
    ticks = us * reload / (1000000 / 1000);
    /* 获得当前时间 */
    told = SysTick->VAL;
    while (1)
    {
        /* 循环获得当前时间，直到达到指定的时间后退出循环 */
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}


void delay_ms(u16 ms)
{
	uint32_t ams = systicks_get() + ms;

	while (systicks_get() < ams)
	{

	}
}


uint32_t millis_elapsed(uint32_t n)
{
    return systicks_get() - n;
}





















