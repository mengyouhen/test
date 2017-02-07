
#ifndef APP_SYS_TICK_H_
#define APP_SYS_TICK_H_
#include "..\..\type.h"

typedef struct 
{
	UINT32 TicksPerSec;                          /*每秒滴答数*/
	UINT32 TicksPerms;                           /*每毫秒滴答数*/	
	UINT32 TicksPerus;                           /*每微秒滴答数*/
				 
	volatile UINT32 NumTicks;	                 /*系统开始时的滴答数*/
	volatile UINT32 NumSec;                     /*从启动开始的时间*/
	volatile UINT32 NumSecRemainder;            /*从启动开始的时间，在CPU滴答内还剩余的时间*/
	FLOAT Resolution;                            /*在1S中的精度*/
	UINT32 ResolutionCpuTicks;                   /*CPU滴答精度*/
	UINT32 CpuTicksPerSec;   
}Sys_Time_Struct;

static  int systick_check_underflow(void);
void API_SysTick_config(void);
UINT32 API_SysTick_get_micros(void);
UINT32 API_SysTick_get_mills(void);
void systick_cb(void);
#endif
