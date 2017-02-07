// system init
#include "stdio.h"
#include "APP_Init.h"
#include "APP_Sys_TC.h"
#include "API_IMU_ICM20602.h"

void APP_Task_Init(void)
{
	//APP_IMU_init();
	
	//APP_AHRS_init();
	
}

void APP_System_Init(void)
{
	// system module tick init
	APP_Sys_TC_init();
	// system module com uart init
	//com_uart_port_init();
	
}

bool APP_Device_Init(void)
{
	// mpu6050 init combined with i2c init
	// must first init mpu6050 ,then init hmc5883


	//API_ICM20602_init();
	//API_MPU6050_init();
	//API_65132_init(1);
	//API_5616_init();
	//API_akm9916_init();
	//API_7737_Check();
	//API_8870_init();
 API_PNI_Check();
 API_LTR559_Check();
	return true;
}

