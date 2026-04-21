#ifndef _MULTI_TIMER_FUNC_H_
#define _MULTI_TIMER_FUNC_H_




#include "sys.h"
#include "MultiTimer.h"


uint64_t timer_ticks();




void Timer1Callback(MultiTimer* timer, void *userData);


extern uint64_t _timer_ticks;
extern MultiTimer timer1;

#endif

