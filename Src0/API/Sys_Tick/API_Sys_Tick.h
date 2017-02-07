
#ifndef APP_SYS_TICK_H_
#define APP_SYS_TICK_H_
#include "..\..\type.h"

typedef struct 
{
	UINT32 TicksPerSec;                          /*ÿ��δ���*/
	UINT32 TicksPerms;                           /*ÿ����δ���*/	
	UINT32 TicksPerus;                           /*ÿ΢��δ���*/
				 
	volatile UINT32 NumTicks;	                 /*ϵͳ��ʼʱ�ĵδ���*/
	volatile UINT32 NumSec;                     /*��������ʼ��ʱ��*/
	volatile UINT32 NumSecRemainder;            /*��������ʼ��ʱ�䣬��CPU�δ��ڻ�ʣ���ʱ��*/
	FLOAT Resolution;                            /*��1S�еľ���*/
	UINT32 ResolutionCpuTicks;                   /*CPU�δ𾫶�*/
	UINT32 CpuTicksPerSec;   
}Sys_Time_Struct;

static  int systick_check_underflow(void);
void API_SysTick_config(void);
UINT32 API_SysTick_get_micros(void);
UINT32 API_SysTick_get_mills(void);
void systick_cb(void);
#endif
