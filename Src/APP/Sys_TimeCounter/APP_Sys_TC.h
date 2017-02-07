#ifndef APP_SYS_TC_H_
#define APP_SYS_TC_H_
#include <stdint.h>
//#include "nrf.h"

void APP_Sys_TC_init(void);
uint32_t APP_Sys_get_micros(void);
uint32_t APP_Sys_get_mills(void);

#endif


