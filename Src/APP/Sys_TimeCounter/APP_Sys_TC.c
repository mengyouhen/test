
//  system time counter

//#include "nrf.h"
#include <stdint.h>
#include "APP_Sys_TC.h"
#include "API_Sys_Tick.h"

void APP_Sys_TC_init(void)
{
	API_SysTick_config();  /*API interface, to config systick*/
	
}


uint32_t APP_Sys_get_micros(void)
{
	return API_SysTick_get_micros();
}

uint32_t APP_Sys_get_mills(void)
{
	return API_SysTick_get_mills();
}	

void APP_Sys_boot_time(void)
{
	
}

void APP_Sys_get_delta_time(void)
{
	
}
