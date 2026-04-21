#include "MultiTimerFunc.h"
#include "step.h"
#include "timer.h"
#include "usart.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "sms.h"

MultiTimer timer1;



static uint8_t beepMask = 0;
uint8_t ledState = 1;
uint64_t _timer_ticks = 0;

uint64_t timer_ticks()
{
	return _timer_ticks;
}


// void Timer1Callback(MultiTimer* timer, void *userData)
// {
// 	char *p = (char *)userData;

// 	// 关闭步进电机
// 	uln2003_MotorRotation(STOP_IT);

// }


// 短信模块返回指令接收
// void sms_cmd_get_cb(MultiTimer* timer, void *userData)
// {

// 	len = queue_read(&my_uart1_rx_Q, szbuf, sizeof(szbuf));
//     my_sms_handle_rx((uint8_t *)szbuf, len);
// }




