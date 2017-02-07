// call cmsis m4 interface ---- systick  and its handler to operate ticks of the system

#include "API_Sys_Tick.h"
#include <stdint.h>
#include "stm32f0xx_hal.h"
//#include "nrf.h"     /*board support driver*/

/*param define*/
static volatile uint32_t sysTickMillis = 0;

Sys_Time_Struct sys_time;


/*function define*/
/**
* @Brief 		a interrupt entry from the mcu
* @Author
* @Param 		the first input value
* @Return 	the return value
* @Modify       
* @Date 
* @Details 	it will be called depends on the system tick frequency  1khz frequency  
*/
void systick_cb(void)	
{
	//	unsigned int i = 0;
	__disable_irq();           /*关闭所有中断*/
	systick_check_underflow();
	sys_time.NumTicks++;
	
//	sys_time.NumSecRemainder += sys_time.ResolutionCpuTicks;

//	if (sys_time.NumSecRemainder >= sys_time.CpuTicksPerSec) 
//	{
//		sys_time.NumSecRemainder -= sys_time.CpuTicksPerSec;
//		sys_time.NumSec++;

//	}
	
	sysTickMillis++;
	__enable_irq();           /*打开所有中断*/
}

void API_SysTick_config(void)
{
	sys_time.NumSec = 0;
	sys_time.NumTicks = 0;
	sys_time.NumSecRemainder = 0;
	sys_time.TicksPerSec = SystemCoreClock;                         /* useless temporarily*/
	sys_time.TicksPerus = SystemCoreClock/1000/1000;   //64
	sys_time.Resolution = 1.0f / sys_time.TicksPerSec;   /*一个滴答的时间  */ /* useless temporarily*/
	
	//SysTick_Config(SystemCoreClock / 1000);  /*设置SysTick重装定时器值  1ms, 并且开始systick定时器中断*/	
	sys_time.CpuTicksPerSec = SystemCoreClock;      /*64M ticks*/
	sys_time.ResolutionCpuTicks = (UINT32)(sys_time.Resolution * sys_time.CpuTicksPerSec + 0.5f); 
	//NVIC_EnableIRQ(SysTick_IRQn);    // 在调用SysTick_Config的时候，中断已经开启了
}

UINT32 API_SysTick_get_micros(void)
{
	UINT32 cycle, timeMs;
    do
    {
        timeMs = sysTickMillis;
        cycle = SysTick->VAL;            //获取当前值
        __NOP();//asm volatile("nop");
        __NOP();//asm volatile("nop");
    }
    while (timeMs != sysTickMillis);     // for safty

    if (systick_check_underflow())       // check again, to insure the LOAD	do not overflow
    {
        timeMs++;
        cycle = SysTick->VAL;
    }

    return (timeMs * 1000) + (SysTick->LOAD + 1 - cycle) / sys_time.TicksPerus;
		                         //SysTick->LOAD 重装载值寄存器
}

UINT32 API_SysTick_get_mills(void)
{
	return sysTickMillis;
}

/**
* @Brief 		检查系统滴答溢出  
* @Author
* @Param
* @Return 		int 返回溢出状态   COUNTFLAG
* @Modify       
* @Date 
* @Details
*/
static int systick_check_underflow(void)
{
	  /*check countflag, when STK_VAL decreased to 0, the countflag was set 1*/
    return SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
}



