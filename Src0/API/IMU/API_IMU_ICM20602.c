/*******************************************************************************
@File Name:  			API_IMU_ICM20602.c
@Created By:  			Tao
@Created Date:			2016.6.27
@Description : 			api driver for icm20602 from invensense, using spi from lower driver layer
@Edition:
@Modify  By:			
@Modify  Date:		
@Modify  Description :
*********************(C) COPYRIGHT 2016- *******************************/


// Software reset is required using PWR_MGMT_1 reg, prior to initialization
// driver layer description: stm_hal -> API-> APP
#include <stdbool.h>
#include <stdint.h>

#include "API_IMU_ICM20602.h"
#include "icm20602_register_map.h"

//Borad driver layers
#include "stm32f0xx_hal.h"
#include "BSP_I2C.h"
//#include "i2c.h"   // wlr
/* if you are using spi
 * SPI process of ICM20602,
 * master            slave
 * []MSB      ->      	[]
 * []					[]
 * []					[]
 * []					[]
 * []					[]
 * []					[]
 * []		 <-			[]
 * DUMMY BYTE from ICM20602 is 0x00
*/

#define ICM20602_ADDRESS  0x68
#define ICM20602_WHO_AM_I  0x12

#define ICM20602_Gyro_250_Scale_Factor   131.0f
#define ICM20602_Gyro_500_Scale_Factor   65.5f	//
#define ICM20602_Gyro_1000_Scale_Factor  32.8f
#define ICM20602_Gyro_2000_Scale_Factor  16.4f

#define ICM20602_Accel_2_Scale_Factor    16384.0f
#define ICM20602_Accel_4_Scale_Factor    8192.0f	  //
#define ICM20602_Accel_8_Scale_Factor    4096.0f
#define ICM20602_Accel_16_Scale_Factor   2048.0f

#define ICM20602_ACCEL_SCALE_2G 0x00   // different from MPU6xxx
#define ICM20602_ACCEL_SCALE_4G 0x08
#define ICM20602_ACCEL_SCALE_8G 0x10
#define	ICM20602_ACCEL_SCALE_16G 0x18
#define ICM20602_GYRO_SCALE_250 0X00
#define ICM20602_GYRO_SCALE_500 0X08
#define ICM20602_GYRO_SCALE_1000 0X10
#define ICM20602_GYRO_SCALE_2000 0X18
//#define ICM20602_ACCEL_SAMPLE_4_AVERAGE
//#define ICM20602_ACCEL_SAMPLE_8_AVERAGE
//#define ICM20602_ACCEL_SAMPLE_16_AVERAGE
//#define ICM20602_ACCEL_SAMPLE_32_AVERAGE
#define AKM099XX_MAG_I2C_ADDR1                 0x0C
#define AKM099XX_MAG_I2C_ADDR2                 0x0E
#define AKM099XX_MAG_I2C_ADDR3                 0x0D
#define AKM099XX_MAG_I2C_ADDR4                 0x0F
/* AKM Register Addresses */
#define AKM_MAG_REG_ADDR_WIA1_099XX            0x00   /* company ID */
#define AKM_MAG_REG_ADDR_WIA2_099XX            0x01	  /* device  ID */
#define AKM_MAG_REG_ADDR_INFO1_099XX           0x02
#define AKM_MAG_REG_ADDR_INFO2_099XX           0x03
#define AKM_MAG_REG_ADDR_STATUS1_099XX         0x10
#define AKM_MAG_REG_ADDR_HXL_099XX             0x11
#define AKM_MAG_REG_ADDR_HXH_099XX             0x12
#define AKM_MAG_REG_ADDR_HYL_099XX             0x13
#define AKM_MAG_REG_ADDR_HYH_099XX             0x14
#define AKM_MAG_REG_ADDR_HZL_099XX             0x15
#define AKM_MAG_REG_ADDR_HZH_099XX             0x16
#define AKM_MAG_REG_ADDR_TEMPERATURE_099XX     0x17
#define AKM_MAG_REG_ADDR_STATUS2_099XX         0x18
#define AKM_MAG_REG_ADDR_CNTL1_099XX           0x30
#define AKM_MAG_REG_ADDR_CNTL2_099XX           0x31
#define RESOLUT_2880_1440                      1
#define RESOLUT_480_640                        0
/*
static void _confirm_write_reg(uint8_t regAddr, uint8_t setting){
	uint8_t check_reg = 0;
	do{
		BSP_SPI_write_reg(regAddr, setting);
		HAL_Delay(5);
		check_reg = 0;
		BSP_SPI_read_reg(regAddr, &check_reg);
		HAL_Delay(5);
	} while(check_reg != setting);
}

static void API_ICM20602_config(void){
#if 1
	_confirm_write_reg(ICM20602_REG_ACCEL_CONFIG, ICM20602_ACCEL_SCALE_4G); 	//set Accel Full scale range
	_confirm_write_reg(ICM20602_REG_GYRO_CONFIG, ICM20602_GYRO_SCALE_2000);    	//set Gyro Full scale range
	//set ACC DLPF
	//set Gyro DLPF
#else
	uint8_t check_reg = 0;

	BSP_SPI_write_reg(ICM20602_REG_ACCEL_CONFIG, ICM20602_ACCEL_SCALE_4G);
	HAL_Delay(5);
	BSP_SPI_read_reg(ICM20602_REG_ACCEL_CONFIG, &check_reg);
	while(check_reg != (uint8_t)ICM20602_ACCEL_SCALE_4G) {
		BSP_SPI_write_reg(ICM20602_REG_ACCEL_CONFIG, ICM20602_ACCEL_SCALE_4G);
		HAL_Delay(5);
		BSP_SPI_read_reg(ICM20602_REG_ACCEL_CONFIG, &check_reg);
		HAL_Delay(5);
		printf("\n\rAPI_ICM20602_config check_reg = %x\n\r",check_reg);
	}
#endif
}
void API_ICM20602_reset(void){
	//reset chip
	BSP_SPI_write_reg(ICM20602_REG_PWR_MGMT_1, 0x80);
	HAL_Delay(50);
}

void API_ICM20602_wakeup(void){
	//wake up
	BSP_SPI_write_reg(ICM20602_REG_PWR_MGMT_1, 0x01);
	HAL_Delay(5);
	//enable all axis including acc and gyro
	BSP_SPI_write_reg(ICM20602_REG_PWR_MGMT_2, 0x00);
	HAL_Delay(5);
}
void API_ICM20602_read_imu_data(accel_values_t* acc_data, gyro_values_t* gyro_data)
{

}
uint8_t API_ICM20602_check_id(void){
	uint8_t data;
	BSP_SPI_read_reg(MPU_REG_WHO_AM_I, &data);
	printf("\n\rAPI_ICM20602_check_id data = 0x%x\n\r", data);
	return data;
}

bool API_ICM20602_init(void)
{
	if(API_ICM20602_check_id() != 18){
		printf("\n\rAPI_ICM20602_init error\n\r");
		return false;
	}
	API_ICM20602_reset();       // reset device

	API_ICM20602_wakeup();  	// chip and sensor wake-up

	API_ICM20602_config();   	//sensor scale and low pass filter config
	HAL_Delay(80);              // time from gyro enable to gyro drive ready, written in the datasheet

	return true;
}
*/
void API_akm9916_check_ID(void)
{
	uint8_t temp_id = 0;
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_WIA1_099XX, &temp_id,1);
	printf("\r\nakm_check_ID:00 %d\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_WIA2_099XX, &temp_id,1);
	printf("akm_check_ID:01 %d\r\n", temp_id);
	BSP_I2C_write_reg(AKM_MAG_REG_ADDR_CNTL2_099XX, 0x08);
	HAL_Delay(10);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_STATUS1_099XX, &temp_id,1);
	printf("akm_check_ID:10 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_HXL_099XX, &temp_id,1);
	printf("akm_check_ID:11 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_HXH_099XX, &temp_id,1);
	printf("akm_check_ID:12 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_HYL_099XX, &temp_id,1);
	printf("akm_check_ID:13 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_HYH_099XX, &temp_id,1);
	printf("akm_check_ID:14 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_HZL_099XX, &temp_id,1);
	printf("akm_check_ID:15 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_HZH_099XX, &temp_id,1);
	printf("akm_check_ID:16 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_TEMPERATURE_099XX, &temp_id,1);
	printf("akm_check_ID:17 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_STATUS2_099XX, &temp_id,1);
	printf("akm_check_ID:18 %x\r\n", temp_id);
	BSP_I2C_read_reg(AKM_MAG_REG_ADDR_CNTL2_099XX, &temp_id,1);
	printf("akm_check_ID:31 %x\r\n", temp_id);
}
bool API_akm9916_init(void)
{
	BSP_I2C_ID_set(0x0C,1);
	API_akm9916_check_ID();
	return true;
}

void API_5616_audiopath_setting(void)
{
  I2C_DATATypeDef settings[]={
{0x00,0x0021},
{0x02,0x0808},
{0x19,0xAFAF},
{0x29,0x8080},
{0x45,0x2000},
{0x2A,0x1212},
{0x53,0x3000},
{0x03,0x2727},
{0x61,0x9800},
{0x62,0x0800},
{0x63,0xF8FF},
{0x64,0x0000},
{0x65,0x0000},
{0x66,0x0000},
{0x6A,0x003D},
{0x6C,0x0600},
{0x70,0x8000},
{0x73,0x1104},
{0x8E,0x0019},
{0x8F,0x2000},
{0xFA,0x0001}
};
for( uint8_t i=0;i<20;i++ )
{
		BSP_I2C_write_r8s16(settings[i].reg,settings[i].setting);
}
	
	/*
	BSP_I2C_write_r8s16(0x00,0x0021);
	BSP_I2C_write_r8s16(0x02,0x0808);
	BSP_I2C_write_r8s16(0x19,0xAFAF);
	BSP_I2C_write_r8s16(0x29,0x8080);
	BSP_I2C_write_r8s16(0x45,0x2000);
	BSP_I2C_write_r8s16(0x2A,0x1212);
	BSP_I2C_write_r8s16(0x53,0x3000);
	BSP_I2C_write_r8s16(0x03,0x2727);
	BSP_I2C_write_r8s16(0x61,0x9800);
	BSP_I2C_write_r8s16(0x62,0x0800);
	BSP_I2C_write_r8s16(0x63,0xF8FF);
	BSP_I2C_write_r8s16(0x64,0x0000);
	BSP_I2C_write_r8s16(0x65,0x0000);
	BSP_I2C_write_r8s16(0x66,0x0000);

	BSP_I2C_write_r8s16(0x6A,0x003D);
	BSP_I2C_write_r8s16(0x6C,0x0600);

	BSP_I2C_write_r8s16(0x70,0x8000);
	BSP_I2C_write_r8s16(0x73,0x1104);
	BSP_I2C_write_r8s16(0x8E,0x0019);
	BSP_I2C_write_r8s16(0x8F,0x2000);
	BSP_I2C_write_r8s16(0xFA,0x0001);
	//BSP_I2C_write_r8s16(0x3D,0x0600);
	*/
}
uint8_t API_5616_check_id(void)
{
    uint8_t data[2] = {0, 0};
    BSP_I2C_read_reg(0xFE, data, 2);

	printf("\n\rAPI_5616_check_id data0 = 0x%x data1 = 0x%x\n\r", data[0],data[1]);
    return 1;
}
bool API_5616_init(void)
{
	uint8_t data[2] = {0, 0};
	uint8_t regs[]={0x00,0x02,0x19,0x29,0x45,0x2A,0x53,0x03,0x61,0x62,0x63,0x64,0x65,0x66,0x6A,0x6C,0x70,0x73,0x8E,0x8F,0xFA};
	BSP_I2C_ID_set(0x1B,2);
	if(API_5616_check_id() != 0x10EC){
		//return false;
	}
	API_5616_audiopath_setting();
	printf("\n\rAPI_5616_init:\n\r");
	for( uint8_t i=0;i<20;i++ )
	{
	 	BSP_I2C_read_reg(regs[i], data, 2);
	  printf("\n\rreg: 0x02 data = 0x%2x%2x\n\r", data[0],data[1]);	
	}
/*	
	BSP_I2C_read_reg(0x02, data, 2);
	printf("\n\rreg: 0x02 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x19, data, 2);
	printf("\n\rreg: 0x19 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x29, data, 2);
	printf("\n\rreg: 0x29 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x45, data, 2);
	printf("\n\rreg: 0x45 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x2A, data, 2);
	printf("\n\rreg: 0x2A data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x53, data, 2);
	printf("\n\rreg: 0x53 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x03, data, 2);
	printf("\n\rreg: 0x03 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x61, data, 2);
	printf("\n\rreg: 0x61 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x62, data, 2);
	printf("\n\rreg: 0x62 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x63, data, 2);
	printf("\n\rreg: 0x63 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x64, data, 2);
	printf("\n\rreg: 0x64 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x65, data, 2);
	printf("\n\rreg: 0x65 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x66, data, 2);
	printf("\n\rreg: 0x66 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x70, data, 2);
	printf("\n\rreg: 0x70 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x73, data, 2);
	printf("\n\rreg: 0x73 data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x8E, data, 2);
	printf("\n\rreg: 0x8E data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x8F, data, 2);
	printf("\n\rreg: 0x8F data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0xFA, data, 2);
	printf("\n\rreg: 0xFA data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x0D, data, 2);
	printf("\n\rreg: 0x0D data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x6A, data, 2);
	printf("\n\rreg: 0x6A data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x6C, data, 2);
	printf("\n\rreg: 0x6C data = 0x%2x%2x\n\r", data[0],data[1]);
	BSP_I2C_read_reg(0x3D, data, 2);
	printf("\n\rreg: 0x3D data = 0x%2x%2x\n\r", data[0],data[1]);
	*/
	return true;
}
void API_65132_set(int panelnum)
{
    uint8_t data;
	if(panelnum == 0)
		BSP_I2C_ID_set(0x3E,2);
	else
		BSP_I2C_ID_set(0x3E,1);
	BSP_I2C_write_reg(0x00, 0x0F);
    HAL_Delay(5);
    BSP_I2C_write_reg(0x01, 0x0F);
    HAL_Delay(5);
    BSP_I2C_write_reg(0x03, 0x03);
    HAL_Delay(5);
	printf("\n\rAPI_65132_set\n\r");
	BSP_I2C_read_reg(0x00, &data, 1);
    HAL_Delay(5);
	printf("\n\rreg00 data = 0x%x\n\r", data);
	BSP_I2C_read_reg(0x01, &data, 1);
    HAL_Delay(5);
	printf("\n\rreg01 data = 0x%x\n\r", data);
	BSP_I2C_read_reg(0x03, &data, 1);
    HAL_Delay(5);
	printf("\n\rreg03 data = 0x%x\n\r", data);
}

bool API_65132_init(int panelnum)
{
	API_65132_set(panelnum);
	return true;
}

#if 0
void API_8870_resolution_init_check()
{
	//REM	Initialization to receive HDMI signal
	//REM	Software Reset
	uint8_t data[4] = {0, 0, 0, 0};
	printf("\n\rAPI_8870_resolution_init_check>>>>>>>>>>>>>>\n\r");
	BSP_I2C_ReadMulti(0x0004,&data,2);
	printf("\n\rreg: 0x0004 data = 0x%2x%2x       0x0004\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x0002,&data,2);
	printf("\n\rreg: 0x0002 data = 0x%2x%2x       0x3F01\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x0002,&data,2);
	printf("\n\rreg: 0x0002 data = 0x%2x%2x       0x0000\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x0006,&data,2);
	printf("\n\rreg: 0x0006 data = 0x%2x%2x       0x0008\n\r", data[0],data[1]);

	//REM	DSI-TX0 Transition Timing
	BSP_I2C_ReadMulti(0x0108,&data,4);
	printf("\n\rreg: 0x0108 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x010C,&data,4);
	printf("\n\rreg: 0x010C data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x02A0,&data,4);
	printf("\n\rreg: 0x02A0 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x02AC,&data,4);
	printf("\n\rreg: 0x02AC data = 0x%2x%2x%2x%2x       0x00005863\n\r", data[0],data[1],data[2],data[3]);
	HAL_Delay(1);
	BSP_I2C_ReadMulti(0x02A0,&data,4);
	printf("\n\rreg: 0x02A0 data = 0x%2x%2x%2x%2x       0x00000003\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0118,&data,4);
	printf("\n\rreg: 0x0118 data = 0x%2x%2x%2x%2x       0x00000014\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0120,&data,4);
	printf("\n\rreg: 0x0120 data = 0x%2x%2x%2x%2x       0x00001770\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0124,&data,4);
	printf("\n\rreg: 0x0124 data = 0x%2x%2x%2x%2x       0x00000000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0128,&data,4);
	printf("\n\rreg: 0x0128 data = 0x%2x%2x%2x%2x       0x00000101\n\r", data[0],data[1],data[2],data[3]);
	//BSP_I2C_write_r16s32(0x012C,0x00000000);
	BSP_I2C_ReadMulti(0x0130,&data,4);
	printf("\n\rreg: 0x0130 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0134,&data,4);
	printf("\n\rreg: 0x0134 data = 0x%2x%2x%2x%2x       0x00005000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0138,&data,4);
	printf("\n\rreg: 0x0138 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x013C,&data,4);
	printf("\n\rreg: 0x013C data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0140,&data,4);
	printf("\n\rreg: 0x0140 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0144,&data,4);
	printf("\n\rreg: 0x0144 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0148,&data,4);
	printf("\n\rreg: 0x0148 data = 0x%2x%2x%2x%2x       0x00001000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x014C,&data,4);
	printf("\n\rreg: 0x014C data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0150,&data,4);
	printf("\n\rreg: 0x0150 data = 0x%2x%2x%2x%2x       0x00000141\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0154,&data,4);
	printf("\n\rreg: 0x0154 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0158,&data,4);
	printf("\n\rreg: 0x0158 data = 0x%2x%2x%2x%2x       0x000000C8\n\r", data[0],data[1],data[2],data[3]);
	//BSP_I2C_write_r16s32(0x0164,0x00000014);
	BSP_I2C_ReadMulti(0x0168,&data,4);
	printf("\n\rreg: 0x0168 data = 0x%2x%2x%2x%2x       0x0000002A\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0170,&data,4);
	printf("\n\rreg: 0x0170 data = 0x%2x%2x%2x%2x       0x00000312\n\r", data[0],data[1],data[2],data[3]);
	//BSP_I2C_write_r16s32(0x0178,0x00000001);
	BSP_I2C_ReadMulti(0x017C,&data,4);
	printf("\n\rreg: 0x017C data = 0x%2x%2x%2x%2x       0x00000081\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x018C,&data,4);
	printf("\n\rreg: 0x018C data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0190,&data,4);
	printf("\n\rreg: 0x0190 data = 0x%2x%2x%2x%2x       0x0000023C\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x01A4,&data,4);
	printf("\n\rreg: 0x01A4 data = 0x%2x%2x%2x%2x       0x00000000\n\r", data[0],data[1],data[2],data[3]);
	//BSP_I2C_write_r16s32(0x01AC,0x00000000);
	BSP_I2C_ReadMulti(0x01C0,&data,4);
	printf("\n\rreg: 0x01C0 data = 0x%2x%2x%2x%2x       0x00000015\n\r", data[0],data[1],data[2],data[3]);
	//BSP_I2C_write_r16s32(0x020C,0x00000000);
	BSP_I2C_ReadMulti(0x0214,&data,4);
	printf("\n\rreg: 0x0214 data = 0x%2x%2x%2x%2x       0x00000000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x021C,&data,4);
	printf("\n\rreg: 0x021C data = 0x%2x%2x%2x%2x       0x00000080\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0224,&data,4);
	printf("\n\rreg: 0x0224 data = 0x%2x%2x%2x%2x       0x00000000\n\r", data[0],data[1],data[2],data[3]);
	//BSP_I2C_write_r16s32(0x0240,0x00000000);
	BSP_I2C_ReadMulti(0x0254,&data,4);
	printf("\n\rreg: 0x0254 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0258,&data,4);
	printf("\n\rreg: 0x0258 data = 0x%2x%2x%2x%2x       0x00070001\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x025C,&data,4);
	printf("\n\rreg: 0x025C data = 0x%2x%2x%2x%2x       0x00020000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0260,&data,4);
	printf("\n\rreg: 0x0260 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0264,&data,4);
	printf("\n\rreg: 0x0264 data = 0x%2x%2x%2x%2x       0x000036B0\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0268,&data,4);
	printf("\n\rreg: 0x0268 data = 0x%2x%2x%2x%2x       0x00000006\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x026C,&data,4);
	printf("\n\rreg: 0x026C data = 0x%2x%2x%2x%2x       0x00020002\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0270,&data,4);
	printf("\n\rreg: 0x0270 data = 0x%2x%2x%2x%2x       0x00000020\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0274,&data,4);
	printf("\n\rreg: 0x0274 data = 0x%2x%2x%2x%2x       0x0000001F\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0278,&data,4);
	printf("\n\rreg: 0x0278 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x027C,&data,4);
	printf("\n\rreg: 0x027C data = 0x%2x%2x%2x%2x       0x00000002\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x011C,&data,4);
	printf("\n\rreg: 0x011C data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[0],data[1],data[2],data[3]);
	//REM	Command Transmission Before Video Start
	BSP_I2C_ReadMulti(0x0500,&data,2);
	printf("\n\rreg: 0x0500 data = 0x%2x%2x       0x0000\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x0110,&data,4);
	printf("\n\rreg: 0x0110 data = 0x%2x%2x%2x%2x       0x00000016\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x0310,&data,4);
	printf("\n\rreg: 0x0310 data = 0x%2x%2x%2x%2x       0x00000016\n\r", data[0],data[1],data[2],data[3]);

	////////Panel Init
	//-----[CMD1]0x11(type = 0x05)
	BSP_I2C_ReadMulti(0x0504,&data,2);
	printf("\n\rreg: 0x0504 data = 0x%2x%2x       0x0005\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x0504,&data,2);
	printf("\n\rreg: 0x0504 data = 0x%2x%2x       0x0011\n\r", data[0],data[1]);
	////////Panel Init End

	//REM	Split Control
	BSP_I2C_ReadMulti(0x5000,&data,2);
	printf("\n\rreg: 0x5000 data = 0x%2x%2x       0x0000\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x500C,&data,2);
	printf("\n\rreg: 0x500C data = 0x%2x%2x       0x0000\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x500E,&data,2);
	printf("\n\rreg: 0x500E data = 0x%2x%2x       0x027F\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x5080,&data,2);
	printf("\n\rreg: 0x5080 data = 0x%2x%2x       0x0000\n\r", data[0],data[1]);
	//BSP_I2C_write_r16s16(0x5082,0xA010);
	BSP_I2C_ReadMulti(0x508C,&data,2);
	printf("\n\rreg: 0x508C data = 0x%2x%2x       0x0000\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x508E,&data,2);
	printf("\n\rreg: 0x508E data = 0x%2x%2x       0x027F\n\r", data[0],data[1]);
	//REM	HDMI PHY
	BSP_I2C_ReadMulti(0x8410,&data,1);
	printf("\n\rreg: 0x8410 data = 0x%x 0x03\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8413,&data,1);
	printf("\n\rreg: 0x8413 data = 0x%x 0x3F\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8420,&data,1);
	printf("\n\rreg: 0x8420 data = 0x%x 0x07\n\r", data[0]);
	BSP_I2C_ReadMulti(0x84F0,&data,1);
	printf("\n\rreg: 0x84F0 data = 0x%x 0x31\n\r", data[0]);
	BSP_I2C_ReadMulti(0x84F4,&data,1);
	printf("\n\rreg: 0x84F4 data = 0x%x 0x01\n\r", data[0]);
	//REM	HDMI Clock
	BSP_I2C_ReadMulti(0x8540,&data,2);
	printf("\n\rreg: 0x8540 data = 0x%2x%2x       0x12C0\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x8630,&data,1);
	printf("\n\rreg: 0x8630 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8631,&data,2);
	printf("\n\rreg: 0x8631 data = 0x%2x%2x       0x0753\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x8670,&data,1);
	printf("\n\rreg: 0x8670 data = 0x%x 0x02\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8A0C,&data,2);
	printf("\n\rreg: 0x8A0C data = 0x%2x%2x       0x12C0\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x850B,&data,1);
	printf("\n\rreg: 0x850B data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x851B,&data,1);
	printf("\n\rreg: 0x851B data = 0x%x 0xFD\n\r", data[0]);

	//REM	Interrupt Control (TOP level)
	BSP_I2C_ReadMulti(0x0014,&data,2);
	printf("\n\rreg: 0x0014 data = 0x%2x%2x       0x0FBF\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x0016,&data,2);
	printf("\n\rreg: 0x0016 data = 0x%2x%2x       0x0DBF\n\r", data[0],data[1]);
	//REM	EDID
	BSP_I2C_ReadMulti(0x85E0,&data,1);
	printf("\n\rreg: 0x85E0 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x85E3,&data,2);
	printf("\n\rreg: 0x85E3 data = 0x%2x%2x       0x0100\n\r", data[0],data[1]);
	//REM	EDID Data
	BSP_I2C_ReadMulti(0x8C00,&data,1);
	printf("\n\rreg: 0x8C00 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C01,&data,1);
	printf("\n\rreg: 0x8C01 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C02,&data,1);
	printf("\n\rreg: 0x8C02 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C03,&data,1);
	printf("\n\rreg: 0x8C03 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C04,&data,1);
	printf("\n\rreg: 0x8C04 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C05,&data,1);
	printf("\n\rreg: 0x8C05 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C06,&data,1);
	printf("\n\rreg: 0x8C06 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C07,&data,1);
	printf("\n\rreg: 0x8C07 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C08,&data,1);
	printf("\n\rreg: 0x8C08 data = 0x%x 0x52\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C09,&data,1);
	printf("\n\rreg: 0x8C09 data = 0x%x 0x62\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0A,&data,1);
	printf("\n\rreg: 0x8C0A data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0B,&data,1);
	printf("\n\rreg: 0x8C0B data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0C,&data,1);
	printf("\n\rreg: 0x8C0C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0D,&data,1);
	printf("\n\rreg: 0x8C0D data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0E,&data,1);
	printf("\n\rreg: 0x8C0E data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0F,&data,1);
	printf("\n\rreg: 0x8C0F data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C10,&data,1);
	printf("\n\rreg: 0x8C10 data = 0x%x 0x1C\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C11,&data,1);
	printf("\n\rreg: 0x8C11 data = 0x%x 0x15\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C12,&data,1);
	printf("\n\rreg: 0x8C12 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C13,&data,1);
	printf("\n\rreg: 0x8C13 data = 0x%x 0x03\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C14,&data,1);
	printf("\n\rreg: 0x8C14 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C15,&data,1);
	printf("\n\rreg: 0x8C15 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C16,&data,1);
	printf("\n\rreg: 0x8C16 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C17,&data,1);
	printf("\n\rreg: 0x8C17 data = 0x%x 0x78\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C18,&data,1);
	printf("\n\rreg: 0x8C18 data = 0x%x 0x0A\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C19,&data,1);
	printf("\n\rreg: 0x8C19 data = 0x%x 0x0D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1A,&data,1);
	printf("\n\rreg: 0x8C1A data = 0x%x 0xC9\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1B,&data,1);
	printf("\n\rreg: 0x8C1B data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1C,&data,1);
	printf("\n\rreg: 0x8C1C data = 0x%x 0x57\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1D,&data,1);
	printf("\n\rreg: 0x8C1D data = 0x%x 0x47\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1E,&data,1);
	printf("\n\rreg: 0x8C1E data = 0x%x 0x98\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1F,&data,1);
	printf("\n\rreg: 0x8C1F data = 0x%x 0x27\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C20,&data,1);
	printf("\n\rreg: 0x8C20 data = 0x%x 0x12\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C21,&data,1);
	printf("\n\rreg: 0x8C21 data = 0x%x 0x48\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C22,&data,1);
	printf("\n\rreg: 0x8C22 data = 0x%x 0x4C\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C23,&data,1);
	printf("\n\rreg: 0x8C23 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C24,&data,1);
	printf("\n\rreg: 0x8C24 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C25,&data,1);
	printf("\n\rreg: 0x8C25 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C26,&data,1);
	printf("\n\rreg: 0x8C26 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C27,&data,1);
	printf("\n\rreg: 0x8C27 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C28,&data,1);
	printf("\n\rreg: 0x8C28 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C29,&data,1);
	printf("\n\rreg: 0x8C29 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2A,&data,1);
	printf("\n\rreg: 0x8C2A data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2B,&data,1);
	printf("\n\rreg: 0x8C2B data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2C,&data,1);
	printf("\n\rreg: 0x8C2C data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2D,&data,1);
	printf("\n\rreg: 0x8C2D data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2E,&data,1);
	printf("\n\rreg: 0x8C2E data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2F,&data,1);
	printf("\n\rreg: 0x8C2F data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C30,&data,1);
	printf("\n\rreg: 0x8C30 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C31,&data,1);
	printf("\n\rreg: 0x8C31 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C32,&data,1);
	printf("\n\rreg: 0x8C32 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C33,&data,1);
	printf("\n\rreg: 0x8C33 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C34,&data,1);
	printf("\n\rreg: 0x8C34 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C35,&data,1);
	printf("\n\rreg: 0x8C35 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C36,&data,1);
	printf("\n\rreg: 0x8C36 data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C37,&data,1);
	printf("\n\rreg: 0x8C37 data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C38,&data,1);
	printf("\n\rreg: 0x8C38 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C39,&data,1);
	printf("\n\rreg: 0x8C39 data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3A,&data,1);
	printf("\n\rreg: 0x8C3A data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3B,&data,1);
	printf("\n\rreg: 0x8C3B data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3C,&data,1);
	printf("\n\rreg: 0x8C3C data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3D,&data,1);
	printf("\n\rreg: 0x8C3D data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3E,&data,1);
	printf("\n\rreg: 0x8C3E data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3F,&data,1);
	printf("\n\rreg: 0x8C3F data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C40,&data,1);
	printf("\n\rreg: 0x8C40 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C41,&data,1);
	printf("\n\rreg: 0x8C41 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C42,&data,1);
	printf("\n\rreg: 0x8C42 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C43,&data,1);
	printf("\n\rreg: 0x8C43 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C44,&data,1);
	printf("\n\rreg: 0x8C44 data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C45,&data,1);
	printf("\n\rreg: 0x8C45 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C46,&data,1);
	printf("\n\rreg: 0x8C46 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C47,&data,1);
	printf("\n\rreg: 0x8C47 data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C48,&data,1);
	printf("\n\rreg: 0x8C48 data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C49,&data,1);
	printf("\n\rreg: 0x8C49 data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4A,&data,1);
	printf("\n\rreg: 0x8C4A data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4B,&data,1);
	printf("\n\rreg: 0x8C4B data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4C,&data,1);
	printf("\n\rreg: 0x8C4C data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4D,&data,1);
	printf("\n\rreg: 0x8C4D data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4E,&data,1);
	printf("\n\rreg: 0x8C4E data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4F,&data,1);
	printf("\n\rreg: 0x8C4F data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4F,&data,1);
	printf("\n\rreg: 0x8C4F data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4F,&data,1);
	printf("\n\rreg: 0x8C4F data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4F,&data,1);
	printf("\n\rreg: 0x8C4F data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4F,&data,1);
	printf("\n\rreg: 0x8C4F data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C54,&data,1);
	printf("\n\rreg: 0x8C54 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C55,&data,1);
	printf("\n\rreg: 0x8C55 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C56,&data,1);
	printf("\n\rreg: 0x8C56 data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C57,&data,1);
	printf("\n\rreg: 0x8C57 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C58,&data,1);
	printf("\n\rreg: 0x8C58 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C59,&data,1);
	printf("\n\rreg: 0x8C59 data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5A,&data,1);
	printf("\n\rreg: 0x8C5A data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5B,&data,1);
	printf("\n\rreg: 0x8C5B data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5C,&data,1);
	printf("\n\rreg: 0x8C5C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5D,&data,1);
	printf("\n\rreg: 0x8C5D data = 0x%x 0xFC\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5E,&data,1);
	printf("\n\rreg: 0x8C5E data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5F,&data,1);
	printf("\n\rreg: 0x8C5F data = 0x%x 0x54\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C60,&data,1);
	printf("\n\rreg: 0x8C60 data = 0x%x 0x6F\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C61,&data,1);
	printf("\n\rreg: 0x8C61 data = 0x%x 0x73\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C62,&data,1);
	printf("\n\rreg: 0x8C62 data = 0x%x 0x68\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C63,&data,1);
	printf("\n\rreg: 0x8C63 data = 0x%x 0x69\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C64,&data,1);
	printf("\n\rreg: 0x8C64 data = 0x%x 0x62\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C65,&data,1);
	printf("\n\rreg: 0x8C65 data = 0x%x 0x61\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C66,&data,1);
	printf("\n\rreg: 0x8C66 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C67,&data,1);
	printf("\n\rreg: 0x8C67 data = 0x%x 0x55\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C68,&data,1);
	printf("\n\rreg: 0x8C68 data = 0x%x 0x48\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C69,&data,1);
	printf("\n\rreg: 0x8C69 data = 0x%x 0x32\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6A,&data,1);
	printf("\n\rreg: 0x8C6A data = 0x%x 0x44\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6B,&data,1);
	printf("\n\rreg: 0x8C6B data = 0x%x 0x0A\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6C,&data,1);
	printf("\n\rreg: 0x8C6C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6C,&data,1);
	printf("\n\rreg: 0x8C6C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6E,&data,1);
	printf("\n\rreg: 0x8C6E data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6F,&data,1);
	printf("\n\rreg: 0x8C6F data = 0x%x 0xFD\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C70,&data,1);
	printf("\n\rreg: 0x8C70 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C71,&data,1);
	printf("\n\rreg: 0x8C71 data = 0x%x 0x14\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C72,&data,1);
	printf("\n\rreg: 0x8C72 data = 0x%x 0x78\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C73,&data,1);
	printf("\n\rreg: 0x8C73 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C74,&data,1);
	printf("\n\rreg: 0x8C74 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C75,&data,1);
	printf("\n\rreg: 0x8C75 data = 0x%x 0x1D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C76,&data,1);
	printf("\n\rreg: 0x8C76 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C77,&data,1);
	printf("\n\rreg: 0x8C77 data = 0x%x 0x0A\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C78,&data,1);
	printf("\n\rreg: 0x8C78 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C79,&data,1);
	printf("\n\rreg: 0x8C79 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7A,&data,1);
	printf("\n\rreg: 0x8C7A data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7B,&data,1);
	printf("\n\rreg: 0x8C7B data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7C,&data,1);
	printf("\n\rreg: 0x8C7C data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7D,&data,1);
	printf("\n\rreg: 0x8C7D data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7E,&data,1);
	printf("\n\rreg: 0x8C7E data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7F,&data,1);
	printf("\n\rreg: 0x8C7F data = 0x%x 0x97\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C80,&data,1);
	printf("\n\rreg: 0x8C80 data = 0x%x 0x02\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C81,&data,1);
	printf("\n\rreg: 0x8C81 data = 0x%x 0x03\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C82,&data,1);
	printf("\n\rreg: 0x8C82 data = 0x%x 0x1A\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C83,&data,1);
	printf("\n\rreg: 0x8C83 data = 0x%x 0x71\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C84,&data,1);
	printf("\n\rreg: 0x8C84 data = 0x%x 0x47\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C85,&data,1);
	printf("\n\rreg: 0x8C85 data = 0x%x 0xC6\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C86,&data,1);
	printf("\n\rreg: 0x8C86 data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C87,&data,1);
	printf("\n\rreg: 0x8C87 data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C88,&data,1);
	printf("\n\rreg: 0x8C88 data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C89,&data,1);
	printf("\n\rreg: 0x8C89 data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8A,&data,1);
	printf("\n\rreg: 0x8C8A data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8B,&data,1);
	printf("\n\rreg: 0x8C8B data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8C,&data,1);
	printf("\n\rreg: 0x8C8C data = 0x%x 0x23\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8D,&data,1);
	printf("\n\rreg: 0x8C8D data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8E,&data,1);
	printf("\n\rreg: 0x8C8E data = 0x%x 0x07\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8F,&data,1);
	printf("\n\rreg: 0x8C8F data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C90,&data,1);
	printf("\n\rreg: 0x8C90 data = 0x%x 0x83\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C91,&data,1);
	printf("\n\rreg: 0x8C91 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C92,&data,1);
	printf("\n\rreg: 0x8C92 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C93,&data,1);
	printf("\n\rreg: 0x8C93 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C94,&data,1);
	printf("\n\rreg: 0x8C94 data = 0x%x 0x65\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C95,&data,1);
	printf("\n\rreg: 0x8C95 data = 0x%x 0x03\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C96,&data,1);
	printf("\n\rreg: 0x8C96 data = 0x%x 0x0C\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C97,&data,1);
	printf("\n\rreg: 0x8C97 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C98,&data,1);
	printf("\n\rreg: 0x8C98 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C99,&data,1);
	printf("\n\rreg: 0x8C99 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9A,&data,1);
	printf("\n\rreg: 0x8C9A data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9B,&data,1);
	printf("\n\rreg: 0x8C9B data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9C,&data,1);
	printf("\n\rreg: 0x8C9C data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9D,&data,1);
	printf("\n\rreg: 0x8C9D data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9E,&data,1);
	printf("\n\rreg: 0x8C9E data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9F,&data,1);
	printf("\n\rreg: 0x8C9F data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA0,&data,1);
	printf("\n\rreg: 0x8CA0 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA1,&data,1);
	printf("\n\rreg: 0x8CA1 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA2,&data,1);
	printf("\n\rreg: 0x8CA2 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA3,&data,1);
	printf("\n\rreg: 0x8CA3 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA4,&data,1);
	printf("\n\rreg: 0x8CA4 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA5,&data,1);
	printf("\n\rreg: 0x8CA5 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA6,&data,1);
	printf("\n\rreg: 0x8CA6 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA7,&data,1);
	printf("\n\rreg: 0x8CA7 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA8,&data,1);
	printf("\n\rreg: 0x8CA8 data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA9,&data,1);
	printf("\n\rreg: 0x8CA9 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAA,&data,1);
	printf("\n\rreg: 0x8CAA data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAB,&data,1);
	printf("\n\rreg: 0x8CAB data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAC,&data,1);
	printf("\n\rreg: 0x8CAC data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAD,&data,1);
	printf("\n\rreg: 0x8CAD data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAE,&data,1);
	printf("\n\rreg: 0x8CAE data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAF,&data,1);
	printf("\n\rreg: 0x8CAF data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB0,&data,1);
	printf("\n\rreg: 0x8CB0 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB1,&data,1);
	printf("\n\rreg: 0x8CB1 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB2,&data,1);
	printf("\n\rreg: 0x8CB2 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB3,&data,1);
	printf("\n\rreg: 0x8CB3 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB4,&data,1);
	printf("\n\rreg: 0x8CB4 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB5,&data,1);
	printf("\n\rreg: 0x8CB5 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB6,&data,1);
	printf("\n\rreg: 0x8CB6 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB7,&data,1);
	printf("\n\rreg: 0x8CB7 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB8,&data,1);
	printf("\n\rreg: 0x8CB8 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB9,&data,1);
	printf("\n\rreg: 0x8CB9 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBA,&data,1);
	printf("\n\rreg: 0x8CBA data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBB,&data,1);
	printf("\n\rreg: 0x8CBB data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBC,&data,1);
	printf("\n\rreg: 0x8CBC data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBD,&data,1);
	printf("\n\rreg: 0x8CBD data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBE,&data,1);
	printf("\n\rreg: 0x8CBE data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBF,&data,1);
	printf("\n\rreg: 0x8CBF data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC0,&data,1);
	printf("\n\rreg: 0x8CC0 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC1,&data,1);
	printf("\n\rreg: 0x8CC1 data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC2,&data,1);
	printf("\n\rreg: 0x8CC2 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC3,&data,1);
	printf("\n\rreg: 0x8CC3 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC4,&data,1);
	printf("\n\rreg: 0x8CC4 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC5,&data,1);
	printf("\n\rreg: 0x8CC5 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC6,&data,1);
	printf("\n\rreg: 0x8CC6 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC7,&data,1);
	printf("\n\rreg: 0x8CC7 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC8,&data,1);
	printf("\n\rreg: 0x8CC8 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC9,&data,1);
	printf("\n\rreg: 0x8CC9 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCA,&data,1);
	printf("\n\rreg: 0x8CCA data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCB,&data,1);
	printf("\n\rreg: 0x8CCB data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCC,&data,1);
	printf("\n\rreg: 0x8CCC data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCD,&data,1);
	printf("\n\rreg: 0x8CCD data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCE,&data,1);
	printf("\n\rreg: 0x8CCE data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCF,&data,1);
	printf("\n\rreg: 0x8CCF data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD0,&data,1);
	printf("\n\rreg: 0x8CD0 data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD1,&data,1);
	printf("\n\rreg: 0x8CD1 data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD2,&data,1);
	printf("\n\rreg: 0x8CD2 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD3,&data,1);
	printf("\n\rreg: 0x8CD3 data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD4,&data,1);
	printf("\n\rreg: 0x8CD4 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD5,&data,1);
	printf("\n\rreg: 0x8CD5 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD6,&data,1);
	printf("\n\rreg: 0x8CD6 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD7,&data,1);
	printf("\n\rreg: 0x8CD7 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD8,&data,1);
	printf("\n\rreg: 0x8CD8 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD9,&data,1);
	printf("\n\rreg: 0x8CD9 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDA,&data,1);
	printf("\n\rreg: 0x8CDA data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDB,&data,1);
	printf("\n\rreg: 0x8CDB data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDC,&data,1);
	printf("\n\rreg: 0x8CDC data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDD,&data,1);
	printf("\n\rreg: 0x8CDD data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDE,&data,1);
	printf("\n\rreg: 0x8CDE data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDF,&data,1);
	printf("\n\rreg: 0x8CDF data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE0,&data,1);
	printf("\n\rreg: 0x8CE0 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE1,&data,1);
	printf("\n\rreg: 0x8CE1 data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE2,&data,1);
	printf("\n\rreg: 0x8CE2 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE3,&data,1);
	printf("\n\rreg: 0x8CE3 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE4,&data,1);
	printf("\n\rreg: 0x8CE4 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE5,&data,1);
	printf("\n\rreg: 0x8CE5 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE6,&data,1);
	printf("\n\rreg: 0x8CE6 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE7,&data,1);
	printf("\n\rreg: 0x8CE7 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE8,&data,1);
	printf("\n\rreg: 0x8CE8 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE9,&data,1);
	printf("\n\rreg: 0x8CE9 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEA,&data,1);
	printf("\n\rreg: 0x8CEA data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEB,&data,1);
	printf("\n\rreg: 0x8CEB data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEC,&data,1);
	printf("\n\rreg: 0x8CEC data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CED,&data,1);
	printf("\n\rreg: 0x8CED data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEE,&data,1);
	printf("\n\rreg: 0x8CEE data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEF,&data,1);
	printf("\n\rreg: 0x8CEF data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF0,&data,1);
	printf("\n\rreg: 0x8CF0 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF1,&data,1);
	printf("\n\rreg: 0x8CF1 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF2,&data,1);
	printf("\n\rreg: 0x8CF2 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF3,&data,1);
	printf("\n\rreg: 0x8CF3 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF4,&data,1);
	printf("\n\rreg: 0x8CF4 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF4,&data,1);
	printf("\n\rreg: 0x8CF4 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF6,&data,1);
	printf("\n\rreg: 0x8CF6 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF7,&data,1);
	printf("\n\rreg: 0x8CF7 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF8,&data,1);
	printf("\n\rreg: 0x8CF8 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF9,&data,1);
	printf("\n\rreg: 0x8CF9 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFA,&data,1);
	printf("\n\rreg: 0x8CFA data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFB,&data,1);
	printf("\n\rreg: 0x8CFB data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFC,&data,1);
	printf("\n\rreg: 0x8CFC data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFD,&data,1);
	printf("\n\rreg: 0x8CFD data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFE,&data,1);
	printf("\n\rreg: 0x8CFE data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFF,&data,1);
	printf("\n\rreg: 0x8CFF data = 0x%x 0xDF\n\r", data[0]);
	//REM	Video Color Format Se,0xng);
	BSP_I2C_ReadMulti(0x8A02,&data,1);
	printf("\n\rreg: 0x8A02 data = 0x%x 0x42\n\r", data[0]);
	//REM	HDMI SYSTEM
	BSP_I2C_ReadMulti(0x8543,&data,1);
	printf("\n\rreg: 0x8543 data = 0x%x 0x02\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8544,&data,1);
	printf("\n\rreg: 0x8544 data = 0x%x 0x10\n\r", data[0]);
	//REM	HDMI Audio Setting
	BSP_I2C_ReadMulti(0x8600,&data,1);
	printf("\n\rreg: 0x8600 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8602,&data,1);
	printf("\n\rreg: 0x8602 data = 0x%x 0xF3\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8603,&data,1);
	printf("\n\rreg: 0x8603 data = 0x%x 0x02\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8604,&data,1);
	printf("\n\rreg: 0x8604 data = 0x%x 0x0C\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8606,&data,1);
	printf("\n\rreg: 0x8606 data = 0x%x 0x05\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8607,&data,1);
	printf("\n\rreg: 0x8607 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8652,&data,1);
	printf("\n\rreg: 0x8652 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8671,&data,4);
	printf("\n\rreg: 0x8671 data = 0x%2x%2x%2x%2x       0x020C49BA\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x8675,&data,4);
	printf("\n\rreg: 0x8675 data = 0x%2x%2x%2x%2x       0x01E1B089\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_ReadMulti(0x8680,&data,1);
	printf("\n\rreg: 0x8680 data = 0x%x 0x01\n\r", data[0]);
	//REM	Let HDMI Source start access
	BSP_I2C_ReadMulti(0x854A,&data,1);
	printf("\n\rreg: 0x854A data = 0x%x 0x01\n\r", data[0]);
	//REM	HDMI Signal Detection
	//REM	Wait until HDMI sync is established
	//REM	By Interrupt   In Interrupt Service Routine.
	BSP_I2C_ReadMulti(0x8520,&data,1);
	printf("\n\rHDMI Signal Detection0 data0 = 0x%x \n\r", data[0]);
	//REM	Sequence: Check bit7 of 8x8520
	BSP_I2C_ReadMulti(0x850B,&data,1);
	printf("\n\rreg: 0x850B data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x0014,&data,2);
	printf("\n\rreg: 0x0014 data = 0x%2x%2x       0x0FBF\n\r", data[0],data[1]);
	//REM	By Polling
	BSP_I2C_ReadMulti(0x8520,&data,1);
	printf("\n\rHDMI Signal Detection1 data0 = 0x%x \n\r", data[0]);
	//REM	Sequence: Check bit7 of 8x8520
	//REM	Start Video TX
	BSP_I2C_ReadMulti(0x0004,&data,2);
	printf("\n\rreg: 0x0004 data = 0x%2x%2x       0x0C35\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x0006,&data,2);
	printf("\n\rreg: 0x0006 data = 0x%2x%2x       0x0000\n\r", data[0],data[1]);
	//REM	Command Transmission After Video Start.
	BSP_I2C_write_r16s32(0x0110,0x00000006);
	printf("\n\rreg: 0x0110 data = 0x%2x%2x%2x%2x       0x00000006\n\r", data[0],data[1],data[2],data[3]);
	BSP_I2C_write_r16s32(0x0310,0x00000006);
	printf("\n\rreg: 0x0310 data = 0x%2x%2x%2x%2x       0x00000006\n\r", data[0],data[1],data[2],data[3]);
	////////Panel Init
	//-----[CMD1]0x29(type = 0x05)
	BSP_I2C_ReadMulti(0x0504,&data,2);
	printf("\n\rreg: 0x0504 data = 0x%2x%2x       0x0005\n\r", data[0],data[1]);
	BSP_I2C_ReadMulti(0x0504,&data,2);
	printf("\n\rreg: 0x0504 data = 0x%2x%2x       0x0029\n\r", data[0],data[1]);
	printf("\n\rAPI_8870_resolution_init_check<<<<<<<<<<<<<\n\r");
}
#endif
void API_8870_resolution_init_check_reverse()
{
	//REM	Initialization to receive HDMI signal
	//REM	Software Reset
	uint8_t data[4] = {0, 0, 0, 0};
	printf("\n\rAPI_8870_resolution_init_check reverse >>>>>>>>>>>>>>\n\r");
	BSP_I2C_ReadMulti(0x0004,data,2);
	printf("\n\rreg: 0x0004 data = 0x%2x%2x       0x0004\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x0002,data,2);
	printf("\n\rreg: 0x0002 data = 0x%2x%2x       0x3F01\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x0002,data,2);
	printf("\n\rreg: 0x0002 data = 0x%2x%2x       0x0000\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x0006,data,2);
	printf("\n\rreg: 0x0006 data = 0x%2x%2x       0x0008\n\r", data[1],data[0]);

	//REM	DSI-TX0 Transition Timing
	BSP_I2C_ReadMulti(0x0108,data,4);
	printf("\n\rreg: 0x0108 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x010C,data,4);
	printf("\n\rreg: 0x010C data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x02A0,data,4);
	printf("\n\rreg: 0x02A0 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x02AC,data,4);
	printf("\n\rreg: 0x02AC data = 0x%2x%2x%2x%2x       0x00005863\n\r", data[3],data[2],data[1],data[0]);
	HAL_Delay(1);
	BSP_I2C_ReadMulti(0x02A0,data,4);
	printf("\n\rreg: 0x02A0 data = 0x%2x%2x%2x%2x       0x00000003\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0118,data,4);
	printf("\n\rreg: 0x0118 data = 0x%2x%2x%2x%2x       0x00000014\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0120,data,4);
	printf("\n\rreg: 0x0120 data = 0x%2x%2x%2x%2x       0x00001770\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0124,data,4);
	printf("\n\rreg: 0x0124 data = 0x%2x%2x%2x%2x       0x00000000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0128,data,4);
	printf("\n\rreg: 0x0128 data = 0x%2x%2x%2x%2x       0x00000101\n\r", data[3],data[2],data[1],data[0]);
	//BSP_I2C_write_r16s32(0x012C,0x00000000);
	BSP_I2C_ReadMulti(0x0130,data,4);
	printf("\n\rreg: 0x0130 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0134,data,4);
	printf("\n\rreg: 0x0134 data = 0x%2x%2x%2x%2x       0x00005000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0138,data,4);
	printf("\n\rreg: 0x0138 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x013C,data,4);
	printf("\n\rreg: 0x013C data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0140,data,4);
	printf("\n\rreg: 0x0140 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0144,data,4);
	printf("\n\rreg: 0x0144 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0148,data,4);
	printf("\n\rreg: 0x0148 data = 0x%2x%2x%2x%2x       0x00001000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x014C,data,4);
	printf("\n\rreg: 0x014C data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0150,data,4);
	printf("\n\rreg: 0x0150 data = 0x%2x%2x%2x%2x       0x00000141\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0154,data,4);
	printf("\n\rreg: 0x0154 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0158,data,4);
	printf("\n\rreg: 0x0158 data = 0x%2x%2x%2x%2x       0x000000C8\n\r", data[3],data[2],data[1],data[0]);
	//BSP_I2C_write_r16s32(0x0164,0x00000014);
	BSP_I2C_ReadMulti(0x0168,data,4);
	printf("\n\rreg: 0x0168 data = 0x%2x%2x%2x%2x       0x0000002A\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0170,data,4);
	printf("\n\rreg: 0x0170 data = 0x%2x%2x%2x%2x       0x00000312\n\r", data[3],data[2],data[1],data[0]);
	//BSP_I2C_write_r16s32(0x0178,0x00000001);
	BSP_I2C_ReadMulti(0x017C,data,4);
	printf("\n\rreg: 0x017C data = 0x%2x%2x%2x%2x       0x00000081\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x018C,data,4);
	printf("\n\rreg: 0x018C data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0190,data,4);
	printf("\n\rreg: 0x0190 data = 0x%2x%2x%2x%2x       0x0000023C\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x01A4,data,4);
	printf("\n\rreg: 0x01A4 data = 0x%2x%2x%2x%2x       0x00000000\n\r", data[3],data[2],data[1],data[0]);
	//BSP_I2C_write_r16s32(0x01AC,0x00000000);
	BSP_I2C_ReadMulti(0x01C0,data,4);
	printf("\n\rreg: 0x01C0 data = 0x%2x%2x%2x%2x       0x00000015\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0214,data,1);
	printf("\n\rreg: 0x0214 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x0215,data,1);
	printf("\n\rreg: 0x0215 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0216,data,1);
	printf("\n\rreg: 0x0216 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0217,data,1);
	printf("\n\rreg: 0x0217 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x021C,data,1);
	printf("\n\rreg: 0x021C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x021D,data,1);
	printf("\n\rreg: 0x021D data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x021E,data,1);
	printf("\n\rreg: 0x021E data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x021F,data,1);
	printf("\n\rreg: 0x021F data = 0x%x \n\r", data[0]);

	BSP_I2C_ReadMulti(0x0224,data,4);
	printf("\n\rreg: 0x0224 data = 0x%2x%2x%2x%2x       0x00000000\n\r", data[3],data[2],data[1],data[0]);
	//BSP_I2C_write_r16s32(0x0240,0x00000000);
	BSP_I2C_ReadMulti(0x0254,data,4);
	printf("\n\rreg: 0x0254 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0258,data,4);
	printf("\n\rreg: 0x0258 data = 0x%2x%2x%2x%2x       0x00070001\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x025C,data,4);
	printf("\n\rreg: 0x025C data = 0x%2x%2x%2x%2x       0x00020000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0260,data,4);
	printf("\n\rreg: 0x0260 data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0264,data,4);
	printf("\n\rreg: 0x0264 data = 0x%2x%2x%2x%2x       0x000036B0\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0268,data,4);
	printf("\n\rreg: 0x0268 data = 0x%2x%2x%2x%2x       0x00000006\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x026C,data,4);
	printf("\n\rreg: 0x026C data = 0x%2x%2x%2x%2x       0x00020002\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0270,data,4);
	printf("\n\rreg: 0x0270 data = 0x%2x%2x%2x%2x       0x00000020\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0274,data,4);
	printf("\n\rreg: 0x0274 data = 0x%2x%2x%2x%2x       0x0000001F\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0278,data,4);
	printf("\n\rreg: 0x0278 data = 0x%2x%2x%2x%2x       0x00010000\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x027C,data,4);
	printf("\n\rreg: 0x027C data = 0x%2x%2x%2x%2x       0x00000002\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x011C,data,4);
	printf("\n\rreg: 0x011C data = 0x%2x%2x%2x%2x       0x00000001\n\r", data[3],data[2],data[1],data[0]);
	//REM	Command Transmission Before Video Start
	BSP_I2C_ReadMulti(0x0500,data,2);
	printf("\n\rreg: 0x0500 data = 0x%2x%2x       0x0000\n\r", data[1],data[0]);
	//BSP_I2C_ReadMulti(0x0110,data,4);
	//printf("\n\rreg: 0x0110 data = 0x%2x%2x%2x%2x       0x00000016\n\r", data[3],data[2],data[1],data[0]);
	//BSP_I2C_ReadMulti(0x0310,data,4);
	//printf("\n\rreg: 0x0310 data = 0x%2x%2x%2x%2x       0x00000016\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x0110,data,1);
	printf("\n\rreg: 0x0110 data = 0x%x 0x16\n\r", data[0]);
	BSP_I2C_ReadMulti(0x0111,data,1);
	printf("\n\rreg: 0x0111 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0112,data,1);
	printf("\n\rreg: 0x0112 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0113,data,1);
	printf("\n\rreg: 0x0113 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0310,data,1);
	printf("\n\rreg: 0x0310 data = 0x%x 0x16\n\r", data[0]);
	BSP_I2C_ReadMulti(0x0311,data,1);
	printf("\n\rreg: 0x0311 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0312,data,1);
	printf("\n\rreg: 0x0312 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0313,data,1);
	printf("\n\rreg: 0x0313 data = 0x%x \n\r", data[0]);

	////////Panel Init
	//-----[CMD1]0x11(type = 0x05)
	BSP_I2C_ReadMulti(0x0504,data,2);
	printf("\n\rreg: 0x0504 data = 0x%2x%2x       0x0005\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x0504,data,2);
	printf("\n\rreg: 0x0504 data = 0x%2x%2x       0x0011\n\r", data[1],data[0]);
	////////Panel Init End

	//REM	Split Control
	BSP_I2C_ReadMulti(0x5000,data,2);
	printf("\n\rreg: 0x5000 data = 0x%2x%2x       0x0000\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x500C,data,2);
	printf("\n\rreg: 0x500C data = 0x%2x%2x       0x0000\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x500E,data,2);
	printf("\n\rreg: 0x500E data = 0x%2x%2x       0x027F\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x5080,data,2);
	printf("\n\rreg: 0x5080 data = 0x%2x%2x       0x0000\n\r", data[1],data[0]);
	//BSP_I2C_write_r16s16(0x5082,0xA010);
	BSP_I2C_ReadMulti(0x508C,data,2);
	printf("\n\rreg: 0x508C data = 0x%2x%2x       0x0000\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x508E,data,2);
	printf("\n\rreg: 0x508E data = 0x%2x%2x       0x027F\n\r", data[1],data[0]);
	//REM	HDMI PHY
	BSP_I2C_ReadMulti(0x8410,data,1);
	printf("\n\rreg: 0x8410 data = 0x%x 0x03\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8413,data,1);
	printf("\n\rreg: 0x8413 data = 0x%x 0x3F\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8420,data,1);
	printf("\n\rreg: 0x8420 data = 0x%x 0x07\n\r", data[0]);
	BSP_I2C_ReadMulti(0x84F0,data,1);
	printf("\n\rreg: 0x84F0 data = 0x%x 0x31\n\r", data[0]);
	BSP_I2C_ReadMulti(0x84F4,data,1);
	printf("\n\rreg: 0x84F4 data = 0x%x 0x01\n\r", data[0]);
	//REM	HDMI Clock
	BSP_I2C_ReadMulti(0x8540,data,2);
	printf("\n\rreg: 0x8540 data = 0x%2x%2x       0x12C0\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x8630,data,1);
	printf("\n\rreg: 0x8630 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8631,data,2);
	printf("\n\rreg: 0x8631 data = 0x%2x%2x       0x0753\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x8670,data,1);
	printf("\n\rreg: 0x8670 data = 0x%x 0x02\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8A0C,data,2);
	printf("\n\rreg: 0x8A0C data = 0x%2x%2x       0x12C0\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x850B,data,1);
	printf("\n\rreg: 0x850B data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x851B,data,1);
	printf("\n\rreg: 0x851B data = 0x%x 0xFD\n\r", data[0]);

	//REM	Interrupt Control (TOP level)
	BSP_I2C_ReadMulti(0x0014,data,2);
	printf("\n\rreg: 0x0014 data = 0x%2x%2x       0x0FBF\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x0016,data,2);
	printf("\n\rreg: 0x0016 data = 0x%2x%2x       0x0DBF\n\r", data[1],data[0]);
	//REM	EDID
	BSP_I2C_ReadMulti(0x85E0,data,1);
	printf("\n\rreg: 0x85E0 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x85E3,data,2);
	printf("\n\rreg: 0x85E3 data = 0x%2x%2x       0x0100\n\r", data[1],data[0]);
	//REM	EDID Data
	BSP_I2C_ReadMulti(0x8C00,data,1);
	printf("\n\rreg: 0x8C00 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C01,data,1);
	printf("\n\rreg: 0x8C01 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C02,data,1);
	printf("\n\rreg: 0x8C02 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C03,data,1);
	printf("\n\rreg: 0x8C03 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C04,data,1);
	printf("\n\rreg: 0x8C04 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C05,data,1);
	printf("\n\rreg: 0x8C05 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C06,data,1);
	printf("\n\rreg: 0x8C06 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C07,data,1);
	printf("\n\rreg: 0x8C07 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C08,data,1);
	printf("\n\rreg: 0x8C08 data = 0x%x 0x52\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C09,data,1);
	printf("\n\rreg: 0x8C09 data = 0x%x 0x62\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0A,data,1);
	printf("\n\rreg: 0x8C0A data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0B,data,1);
	printf("\n\rreg: 0x8C0B data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0C,data,1);
	printf("\n\rreg: 0x8C0C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0D,data,1);
	printf("\n\rreg: 0x8C0D data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0E,data,1);
	printf("\n\rreg: 0x8C0E data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C0F,data,1);
	printf("\n\rreg: 0x8C0F data = 0x%x 0x88\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C10,data,1);
	printf("\n\rreg: 0x8C10 data = 0x%x 0x1C\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C11,data,1);
	printf("\n\rreg: 0x8C11 data = 0x%x 0x15\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C12,data,1);
	printf("\n\rreg: 0x8C12 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C13,data,1);
	printf("\n\rreg: 0x8C13 data = 0x%x 0x03\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C14,data,1);
	printf("\n\rreg: 0x8C14 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C15,data,1);
	printf("\n\rreg: 0x8C15 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C16,data,1);
	printf("\n\rreg: 0x8C16 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C17,data,1);
	printf("\n\rreg: 0x8C17 data = 0x%x 0x78\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C18,data,1);
	printf("\n\rreg: 0x8C18 data = 0x%x 0x0A\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C19,data,1);
	printf("\n\rreg: 0x8C19 data = 0x%x 0x0D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1A,data,1);
	printf("\n\rreg: 0x8C1A data = 0x%x 0xC9\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1B,data,1);
	printf("\n\rreg: 0x8C1B data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1C,data,1);
	printf("\n\rreg: 0x8C1C data = 0x%x 0x57\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1D,data,1);
	printf("\n\rreg: 0x8C1D data = 0x%x 0x47\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1E,data,1);
	printf("\n\rreg: 0x8C1E data = 0x%x 0x98\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C1F,data,1);
	printf("\n\rreg: 0x8C1F data = 0x%x 0x27\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C20,data,1);
	printf("\n\rreg: 0x8C20 data = 0x%x 0x12\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C21,data,1);
	printf("\n\rreg: 0x8C21 data = 0x%x 0x48\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C22,data,1);
	printf("\n\rreg: 0x8C22 data = 0x%x 0x4C\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C23,data,1);
	printf("\n\rreg: 0x8C23 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C24,data,1);
	printf("\n\rreg: 0x8C24 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C25,data,1);
	printf("\n\rreg: 0x8C25 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C26,data,1);
	printf("\n\rreg: 0x8C26 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C27,data,1);
	printf("\n\rreg: 0x8C27 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C28,data,1);
	printf("\n\rreg: 0x8C28 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C29,data,1);
	printf("\n\rreg: 0x8C29 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2A,data,1);
	printf("\n\rreg: 0x8C2A data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2B,data,1);
	printf("\n\rreg: 0x8C2B data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2C,data,1);
	printf("\n\rreg: 0x8C2C data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2D,data,1);
	printf("\n\rreg: 0x8C2D data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2E,data,1);
	printf("\n\rreg: 0x8C2E data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C2F,data,1);
	printf("\n\rreg: 0x8C2F data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C30,data,1);
	printf("\n\rreg: 0x8C30 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C31,data,1);
	printf("\n\rreg: 0x8C31 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C32,data,1);
	printf("\n\rreg: 0x8C32 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C33,data,1);
	printf("\n\rreg: 0x8C33 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C34,data,1);
	printf("\n\rreg: 0x8C34 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C35,data,1);
	printf("\n\rreg: 0x8C35 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C36,data,1);
	printf("\n\rreg: 0x8C36 data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C37,data,1);
	printf("\n\rreg: 0x8C37 data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C38,data,1);
	printf("\n\rreg: 0x8C38 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C39,data,1);
	printf("\n\rreg: 0x8C39 data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3A,data,1);
	printf("\n\rreg: 0x8C3A data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3B,data,1);
	printf("\n\rreg: 0x8C3B data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3C,data,1);
	printf("\n\rreg: 0x8C3C data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3D,data,1);
	printf("\n\rreg: 0x8C3D data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3E,data,1);
	printf("\n\rreg: 0x8C3E data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C3F,data,1);
	printf("\n\rreg: 0x8C3F data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C40,data,1);
	printf("\n\rreg: 0x8C40 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C41,data,1);
	printf("\n\rreg: 0x8C41 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C42,data,1);
	printf("\n\rreg: 0x8C42 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C43,data,1);
	printf("\n\rreg: 0x8C43 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C44,data,1);
	printf("\n\rreg: 0x8C44 data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C45,data,1);
	printf("\n\rreg: 0x8C45 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C46,data,1);
	printf("\n\rreg: 0x8C46 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C47,data,1);
	printf("\n\rreg: 0x8C47 data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C48,data,1);
	printf("\n\rreg: 0x8C48 data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C49,data,1);
	printf("\n\rreg: 0x8C49 data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4A,data,1);
	printf("\n\rreg: 0x8C4A data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4B,data,1);
	printf("\n\rreg: 0x8C4B data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4C,data,1);
	printf("\n\rreg: 0x8C4C data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4D,data,1);
	printf("\n\rreg: 0x8C4D data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4E,data,1);
	printf("\n\rreg: 0x8C4E data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C4F,data,1);
	printf("\n\rreg: 0x8C4F data = 0x%x 0x10\n\r", data[0]);


	BSP_I2C_ReadMulti(0x8C50,data,1);
	printf("\n\rreg: 0x8C50 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C51,data,1);
	printf("\n\rreg: 0x8C51 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C52,data,1);
	printf("\n\rreg: 0x8C52 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C53,data,1);
	printf("\n\rreg: 0x8C53 data = 0x%x 0x00\n\r", data[0]);


	BSP_I2C_ReadMulti(0x8C54,data,1);
	printf("\n\rreg: 0x8C54 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C55,data,1);
	printf("\n\rreg: 0x8C55 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C56,data,1);
	printf("\n\rreg: 0x8C56 data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C57,data,1);
	printf("\n\rreg: 0x8C57 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C58,data,1);
	printf("\n\rreg: 0x8C58 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C59,data,1);
	printf("\n\rreg: 0x8C59 data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5A,data,1);
	printf("\n\rreg: 0x8C5A data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5B,data,1);
	printf("\n\rreg: 0x8C5B data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5C,data,1);
	printf("\n\rreg: 0x8C5C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5D,data,1);
	printf("\n\rreg: 0x8C5D data = 0x%x 0xFC\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5E,data,1);
	printf("\n\rreg: 0x8C5E data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C5F,data,1);
	printf("\n\rreg: 0x8C5F data = 0x%x 0x54\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C60,data,1);
	printf("\n\rreg: 0x8C60 data = 0x%x 0x6F\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C61,data,1);
	printf("\n\rreg: 0x8C61 data = 0x%x 0x73\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C62,data,1);
	printf("\n\rreg: 0x8C62 data = 0x%x 0x68\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C63,data,1);
	printf("\n\rreg: 0x8C63 data = 0x%x 0x69\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C64,data,1);
	printf("\n\rreg: 0x8C64 data = 0x%x 0x62\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C65,data,1);
	printf("\n\rreg: 0x8C65 data = 0x%x 0x61\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C66,data,1);
	printf("\n\rreg: 0x8C66 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C67,data,1);
	printf("\n\rreg: 0x8C67 data = 0x%x 0x55\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C68,data,1);
	printf("\n\rreg: 0x8C68 data = 0x%x 0x48\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C69,data,1);
	printf("\n\rreg: 0x8C69 data = 0x%x 0x32\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6A,data,1);
	printf("\n\rreg: 0x8C6A data = 0x%x 0x44\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6B,data,1);
	printf("\n\rreg: 0x8C6B data = 0x%x 0x0A\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6C,data,1);
	printf("\n\rreg: 0x8C6C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6C,data,1);
	printf("\n\rreg: 0x8C6C data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6E,data,1);
	printf("\n\rreg: 0x8C6E data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C6F,data,1);
	printf("\n\rreg: 0x8C6F data = 0x%x 0xFD\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C70,data,1);
	printf("\n\rreg: 0x8C70 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C71,data,1);
	printf("\n\rreg: 0x8C71 data = 0x%x 0x14\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C72,data,1);
	printf("\n\rreg: 0x8C72 data = 0x%x 0x78\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C73,data,1);
	printf("\n\rreg: 0x8C73 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C74,data,1);
	printf("\n\rreg: 0x8C74 data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C75,data,1);
	printf("\n\rreg: 0x8C75 data = 0x%x 0x1D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C76,data,1);
	printf("\n\rreg: 0x8C76 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C77,data,1);
	printf("\n\rreg: 0x8C77 data = 0x%x 0x0A\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C78,data,1);
	printf("\n\rreg: 0x8C78 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C79,data,1);
	printf("\n\rreg: 0x8C79 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7A,data,1);
	printf("\n\rreg: 0x8C7A data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7B,data,1);
	printf("\n\rreg: 0x8C7B data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7C,data,1);
	printf("\n\rreg: 0x8C7C data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7D,data,1);
	printf("\n\rreg: 0x8C7D data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7E,data,1);
	printf("\n\rreg: 0x8C7E data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C7F,data,1);
	printf("\n\rreg: 0x8C7F data = 0x%x 0x97\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C80,data,1);
	printf("\n\rreg: 0x8C80 data = 0x%x 0x02\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C81,data,1);
	printf("\n\rreg: 0x8C81 data = 0x%x 0x03\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C82,data,1);
	printf("\n\rreg: 0x8C82 data = 0x%x 0x1A\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C83,data,1);
	printf("\n\rreg: 0x8C83 data = 0x%x 0x71\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C84,data,1);
	printf("\n\rreg: 0x8C84 data = 0x%x 0x47\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C85,data,1);
	printf("\n\rreg: 0x8C85 data = 0x%x 0xC6\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C86,data,1);
	printf("\n\rreg: 0x8C86 data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C87,data,1);
	printf("\n\rreg: 0x8C87 data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C88,data,1);
	printf("\n\rreg: 0x8C88 data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C89,data,1);
	printf("\n\rreg: 0x8C89 data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8A,data,1);
	printf("\n\rreg: 0x8C8A data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8B,data,1);
	printf("\n\rreg: 0x8C8B data = 0x%x 0x46\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8C,data,1);
	printf("\n\rreg: 0x8C8C data = 0x%x 0x23\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8D,data,1);
	printf("\n\rreg: 0x8C8D data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8E,data,1);
	printf("\n\rreg: 0x8C8E data = 0x%x 0x07\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C8F,data,1);
	printf("\n\rreg: 0x8C8F data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C90,data,1);
	printf("\n\rreg: 0x8C90 data = 0x%x 0x83\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C91,data,1);
	printf("\n\rreg: 0x8C91 data = 0x%x 0x01\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C92,data,1);
	printf("\n\rreg: 0x8C92 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C93,data,1);
	printf("\n\rreg: 0x8C93 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C94,data,1);
	printf("\n\rreg: 0x8C94 data = 0x%x 0x65\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C95,data,1);
	printf("\n\rreg: 0x8C95 data = 0x%x 0x03\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C96,data,1);
	printf("\n\rreg: 0x8C96 data = 0x%x 0x0C\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C97,data,1);
	printf("\n\rreg: 0x8C97 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C98,data,1);
	printf("\n\rreg: 0x8C98 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C99,data,1);
	printf("\n\rreg: 0x8C99 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9A,data,1);
	printf("\n\rreg: 0x8C9A data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9B,data,1);
	printf("\n\rreg: 0x8C9B data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9C,data,1);
	printf("\n\rreg: 0x8C9C data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9D,data,1);
	printf("\n\rreg: 0x8C9D data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9E,data,1);
	printf("\n\rreg: 0x8C9E data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8C9F,data,1);
	printf("\n\rreg: 0x8C9F data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA0,data,1);
	printf("\n\rreg: 0x8CA0 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA1,data,1);
	printf("\n\rreg: 0x8CA1 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA2,data,1);
	printf("\n\rreg: 0x8CA2 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA3,data,1);
	printf("\n\rreg: 0x8CA3 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA4,data,1);
	printf("\n\rreg: 0x8CA4 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA5,data,1);
	printf("\n\rreg: 0x8CA5 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA6,data,1);
	printf("\n\rreg: 0x8CA6 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA7,data,1);
	printf("\n\rreg: 0x8CA7 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA8,data,1);
	printf("\n\rreg: 0x8CA8 data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CA9,data,1);
	printf("\n\rreg: 0x8CA9 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAA,data,1);
	printf("\n\rreg: 0x8CAA data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAB,data,1);
	printf("\n\rreg: 0x8CAB data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAC,data,1);
	printf("\n\rreg: 0x8CAC data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAD,data,1);
	printf("\n\rreg: 0x8CAD data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAE,data,1);
	printf("\n\rreg: 0x8CAE data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CAF,data,1);
	printf("\n\rreg: 0x8CAF data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB0,data,1);
	printf("\n\rreg: 0x8CB0 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB1,data,1);
	printf("\n\rreg: 0x8CB1 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB2,data,1);
	printf("\n\rreg: 0x8CB2 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB3,data,1);
	printf("\n\rreg: 0x8CB3 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB4,data,1);
	printf("\n\rreg: 0x8CB4 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB5,data,1);
	printf("\n\rreg: 0x8CB5 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB6,data,1);
	printf("\n\rreg: 0x8CB6 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB7,data,1);
	printf("\n\rreg: 0x8CB7 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB8,data,1);
	printf("\n\rreg: 0x8CB8 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CB9,data,1);
	printf("\n\rreg: 0x8CB9 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBA,data,1);
	printf("\n\rreg: 0x8CBA data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBB,data,1);
	printf("\n\rreg: 0x8CBB data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBC,data,1);
	printf("\n\rreg: 0x8CBC data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBD,data,1);
	printf("\n\rreg: 0x8CBD data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBE,data,1);
	printf("\n\rreg: 0x8CBE data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CBF,data,1);
	printf("\n\rreg: 0x8CBF data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC0,data,1);
	printf("\n\rreg: 0x8CC0 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC1,data,1);
	printf("\n\rreg: 0x8CC1 data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC2,data,1);
	printf("\n\rreg: 0x8CC2 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC3,data,1);
	printf("\n\rreg: 0x8CC3 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC4,data,1);
	printf("\n\rreg: 0x8CC4 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC5,data,1);
	printf("\n\rreg: 0x8CC5 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC6,data,1);
	printf("\n\rreg: 0x8CC6 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC7,data,1);
	printf("\n\rreg: 0x8CC7 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC8,data,1);
	printf("\n\rreg: 0x8CC8 data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CC9,data,1);
	printf("\n\rreg: 0x8CC9 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCA,data,1);
	printf("\n\rreg: 0x8CCA data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCB,data,1);
	printf("\n\rreg: 0x8CCB data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCC,data,1);
	printf("\n\rreg: 0x8CCC data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCD,data,1);
	printf("\n\rreg: 0x8CCD data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCE,data,1);
	printf("\n\rreg: 0x8CCE data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CCF,data,1);
	printf("\n\rreg: 0x8CCF data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD0,data,1);
	printf("\n\rreg: 0x8CD0 data = 0x%x 0xD8\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD1,data,1);
	printf("\n\rreg: 0x8CD1 data = 0x%x 0x09\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD2,data,1);
	printf("\n\rreg: 0x8CD2 data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD3,data,1);
	printf("\n\rreg: 0x8CD3 data = 0x%x 0xA0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD4,data,1);
	printf("\n\rreg: 0x8CD4 data = 0x%x 0x20\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD5,data,1);
	printf("\n\rreg: 0x8CD5 data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD6,data,1);
	printf("\n\rreg: 0x8CD6 data = 0x%x 0x2D\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD7,data,1);
	printf("\n\rreg: 0x8CD7 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD8,data,1);
	printf("\n\rreg: 0x8CD8 data = 0x%x 0x10\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CD9,data,1);
	printf("\n\rreg: 0x8CD9 data = 0x%x 0x60\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDA,data,1);
	printf("\n\rreg: 0x8CDA data = 0x%x 0xA2\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDB,data,1);
	printf("\n\rreg: 0x8CDB data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDC,data,1);
	printf("\n\rreg: 0x8CDC data = 0x%x 0x80\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDD,data,1);
	printf("\n\rreg: 0x8CDD data = 0x%x 0xE0\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDE,data,1);
	printf("\n\rreg: 0x8CDE data = 0x%x 0x21\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CDF,data,1);
	printf("\n\rreg: 0x8CDF data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE0,data,1);
	printf("\n\rreg: 0x8CE0 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE1,data,1);
	printf("\n\rreg: 0x8CE1 data = 0x%x 0x18\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE2,data,1);
	printf("\n\rreg: 0x8CE2 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE3,data,1);
	printf("\n\rreg: 0x8CE3 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE4,data,1);
	printf("\n\rreg: 0x8CE4 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE5,data,1);
	printf("\n\rreg: 0x8CE5 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE6,data,1);
	printf("\n\rreg: 0x8CE6 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE7,data,1);
	printf("\n\rreg: 0x8CE7 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE8,data,1);
	printf("\n\rreg: 0x8CE8 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CE9,data,1);
	printf("\n\rreg: 0x8CE9 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEA,data,1);
	printf("\n\rreg: 0x8CEA data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEB,data,1);
	printf("\n\rreg: 0x8CEB data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEC,data,1);
	printf("\n\rreg: 0x8CEC data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CED,data,1);
	printf("\n\rreg: 0x8CED data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEE,data,1);
	printf("\n\rreg: 0x8CEE data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CEF,data,1);
	printf("\n\rreg: 0x8CEF data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF0,data,1);
	printf("\n\rreg: 0x8CF0 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF1,data,1);
	printf("\n\rreg: 0x8CF1 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF2,data,1);
	printf("\n\rreg: 0x8CF2 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF3,data,1);
	printf("\n\rreg: 0x8CF3 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF4,data,1);
	printf("\n\rreg: 0x8CF4 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF4,data,1);
	printf("\n\rreg: 0x8CF4 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF6,data,1);
	printf("\n\rreg: 0x8CF6 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF7,data,1);
	printf("\n\rreg: 0x8CF7 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF8,data,1);
	printf("\n\rreg: 0x8CF8 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CF9,data,1);
	printf("\n\rreg: 0x8CF9 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFA,data,1);
	printf("\n\rreg: 0x8CFA data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFB,data,1);
	printf("\n\rreg: 0x8CFB data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFC,data,1);
	printf("\n\rreg: 0x8CFC data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFD,data,1);
	printf("\n\rreg: 0x8CFD data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFE,data,1);
	printf("\n\rreg: 0x8CFE data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8CFF,data,1);
	printf("\n\rreg: 0x8CFF data = 0x%x 0xDF\n\r", data[0]);
	//REM	Video Color Format Se,0xng);
	BSP_I2C_ReadMulti(0x8A02,data,1);
	printf("\n\rreg: 0x8A02 data = 0x%x 0x42\n\r", data[0]);
	//REM	HDMI SYSTEM
	BSP_I2C_ReadMulti(0x8543,data,1);
	printf("\n\rreg: 0x8543 data = 0x%x 0x02\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8544,data,1);
	printf("\n\rreg: 0x8544 data = 0x%x 0x10\n\r", data[0]);
	//REM	HDMI Audio Setting
	BSP_I2C_ReadMulti(0x8600,data,1);
	printf("\n\rreg: 0x8600 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8602,data,1);
	printf("\n\rreg: 0x8602 data = 0x%x 0xF3\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8603,data,1);
	printf("\n\rreg: 0x8603 data = 0x%x 0x02\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8604,data,1);
	printf("\n\rreg: 0x8604 data = 0x%x 0x0C\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8606,data,1);
	printf("\n\rreg: 0x8606 data = 0x%x 0x05\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8607,data,1);
	printf("\n\rreg: 0x8607 data = 0x%x 0x00\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8652,data,1);
	printf("\n\rreg: 0x8652 data = 0x%x 0x01\n\r", data[0]);

	BSP_I2C_ReadMulti(0x8671,data,1);
	printf("\n\rreg: 0x8671 data = 0x%x 0x020C49BA\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8672,data,1);
	printf("\n\rreg: 0x8672 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x8673,data,1);
	printf("\n\rreg: 0x8673 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x8674,data,1);
	printf("\n\rreg: 0x8674 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x8675,data,1);
	printf("\n\rreg: 0x8675 data = 0x%x 0x01E1B089\n\r", data[0]);
	BSP_I2C_ReadMulti(0x8676,data,1);
	printf("\n\rreg: 0x8676 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x8677,data,1);
	printf("\n\rreg: 0x8677 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x8678,data,1);
	printf("\n\rreg: 0x8678 data = 0x%x \n\r", data[0]);
	//BSP_I2C_ReadMulti(0x8671,data,4);
	//printf("\n\rreg: 0x8671 data = 0x%2x%2x%2x%2x       0x020C49BA\n\r", data[3],data[2],data[1],data[0]);
	//BSP_I2C_ReadMulti(0x8675,data,4);
	//printf("\n\rreg: 0x8675 data = 0x%2x%2x%2x%2x       0x01E1B089\n\r", data[3],data[2],data[1],data[0]);
	BSP_I2C_ReadMulti(0x8680,data,1);
	printf("\n\rreg: 0x8680 data = 0x%x 0x01\n\r", data[0]);
	//REM	Let HDMI Source start access
	BSP_I2C_ReadMulti(0x854A,data,1);
	printf("\n\rreg: 0x854A data = 0x%x 0x01\n\r", data[0]);
	//REM	HDMI Signal Detection
	//REM	Wait until HDMI sync is established
	//REM	By Interrupt   In Interrupt Service Routine.
	BSP_I2C_ReadMulti(0x8520,data,1);
	printf("\n\rHDMI Signal Detection0 data0 = 0x%x \n\r", data[0]);
	//REM	Sequence: Check bit7 of 8x8520
	BSP_I2C_ReadMulti(0x850B,data,1);
	printf("\n\rreg: 0x850B data = 0x%x 0xFF\n\r", data[0]);
	BSP_I2C_ReadMulti(0x0014,data,2);
	printf("\n\rreg: 0x0014 data = 0x%2x%2x       0x0FBF\n\r", data[1],data[0]);
	//REM	By Polling
	BSP_I2C_ReadMulti(0x8520,data,1);
	printf("\n\rHDMI Signal Detection1 data0 = 0x%x \n\r", data[0]);
	//REM	Sequence: Check bit7 of 8x8520
	//REM	Start Video TX
	BSP_I2C_ReadMulti(0x0004,data,2);
	printf("\n\rreg: 0x0004 data = 0x%2x%2x       0x0C35\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x0006,data,2);
	printf("\n\rreg: 0x0006 data = 0x%2x%2x       0x0000\n\r", data[1],data[0]);
	//REM	Command Transmission After Video Start.
	//BSP_I2C_write_r16s32(0x0110,0x00000006);
	//printf("\n\rreg: 0x0110 data = 0x%2x%2x%2x%2x       0x00000006\n\r", data[3],data[2],data[1],data[0]);
	//BSP_I2C_write_r16s32(0x0310,0x00000006);
	//printf("\n\rreg: 0x0310 data = 0x%2x%2x%2x%2x       0x00000006\n\r", data[3],data[2],data[1],data[0]);

	BSP_I2C_ReadMulti(0x0110,data,1);
	printf("\n\rreg: 0x0110 data = 0x%x 0x16\n\r", data[0]);
	BSP_I2C_ReadMulti(0x0111,data,1);
	printf("\n\rreg: 0x0111 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0112,data,1);
	printf("\n\rreg: 0x0112 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0113,data,1);
	printf("\n\rreg: 0x0113 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0310,data,1);
	printf("\n\rreg: 0x0310 data = 0x%x 0x16\n\r", data[0]);
	BSP_I2C_ReadMulti(0x0311,data,1);
	printf("\n\rreg: 0x0311 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0312,data,1);
	printf("\n\rreg: 0x0312 data = 0x%x \n\r", data[0]);
	BSP_I2C_ReadMulti(0x0313,data,1);
	printf("\n\rreg: 0x0313 data = 0x%x \n\r", data[0]);

	////////Panel Init
	//-----[CMD1]0x29(type = 0x05)
	BSP_I2C_ReadMulti(0x0504,data,2);
	printf("\n\rreg: 0x0504 data = 0x%2x%2x       0x0005\n\r", data[1],data[0]);
	BSP_I2C_ReadMulti(0x0504,data,2);
	printf("\n\rreg: 0x0504 data = 0x%2x%2x       0x0029\n\r", data[1],data[0]);
	printf("\n\rAPI_8870_resolution_init_check reverse<<<<<<<<<<<<<\n\r");
}

void API_8870_HDMI_InputSignal(void)
{
	uint8_t data = 0;
	printf("\n\rAPI_8870_HDMI_Test\n\r");
	BSP_I2C_write_r16s8(0x8540, 0xC0);
	BSP_I2C_write_r16s8(0x8541, 0x12);
	BSP_I2C_write_r16s8(0x8A0C, 0xC0);
	BSP_I2C_write_r16s8(0x8A0D, 0x12);
	BSP_I2C_write_r16s8(0x8593, 0x33);
    HAL_Delay(100);
	BSP_I2C_ReadMulti(0x8405, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8405data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8406, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8406data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x858E, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x858Edata = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x858F, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x858Fdata = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8580, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8580data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8581, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8581data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8582, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8582data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8583, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8583data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8590, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8590data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8591, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8591data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8584, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8584data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8585, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8585data = 0x%x 00\n\r", data);
	BSP_I2C_ReadMulti(0x8586, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8586data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8587, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8587data = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x858C, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x858Cdata = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x858D, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x858Ddata = 0x%x \n\r", data);
	BSP_I2C_ReadMulti(0x8526, &data, 1);
	printf("\n\rAPI_8870_HDMI_Test 0x8526data = 0x%x \n\r", data);
}

uint8_t API_8870_check_id(void)
{
	uint8_t data[2] = {0, 0};
	BSP_I2C_ReadMulti(0x0000, data, 2);
	printf("\n\rAPI_8870_check_id data0 = 0x%x data1 = 0x%x\n\r", data[0],data[1]);
    return 1;
}

bool API_8870_init(void)
{
	uint8_t data[4] = {0, 0, 0, 0};
	BSP_I2C_ID_set(0x1F,3);//1F 8870,0F 开发板
	if(API_8870_check_id() != 0x47){
		//return false;
	}
    HAL_Delay(10);

	//REM	Initialization to receive HDMI signal
	//REM	Software Reset
	BSP_I2C_write_r16s16(0x0004,0x0004);
	BSP_I2C_write_r16s16(0x0002,0x3F01);
	BSP_I2C_write_r16s16(0x0002,0x0000);
	BSP_I2C_write_r16s16(0x0006,0x0008);
	HAL_Delay(5);
	//REM	DSI-TX0 Transition Timing
	BSP_I2C_write_r16s32(0x0108,0x00000001);
	BSP_I2C_write_r16s32(0x010C,0x00000001);
	BSP_I2C_write_r16s32(0x02A0,0x00000001);
#if RESOLUT_2880_1440
	BSP_I2C_write_r16s32(0x02AC,0x0000507C);
#else
	printf("\n\rHDMI resolution 680*480 0x02AC\n\r");
	BSP_I2C_write_r16s32(0x02AC,0x00005863);
#endif
	HAL_Delay(1);
	BSP_I2C_write_r16s32(0x02A0,0x00000003);
	BSP_I2C_write_r16s32(0x0118,0x00000014);
	BSP_I2C_write_r16s32(0x0120,0x00001770);
	BSP_I2C_write_r16s32(0x0124,0x00000000);
	BSP_I2C_write_r16s32(0x0128,0x00000101);
	//BSP_I2C_write_r16s32(0x012C,0x00000000);
	BSP_I2C_write_r16s32(0x0130,0x00010000);
	BSP_I2C_write_r16s32(0x0134,0x00005000);
	BSP_I2C_write_r16s32(0x0138,0x00010000);
	BSP_I2C_write_r16s32(0x013C,0x00010000);
	BSP_I2C_write_r16s32(0x0140,0x00010000);
	BSP_I2C_write_r16s32(0x0144,0x00010000);
	BSP_I2C_write_r16s32(0x0148,0x00001000);
	BSP_I2C_write_r16s32(0x014C,0x00010000);
	BSP_I2C_write_r16s32(0x0150,0x00000141);
	BSP_I2C_write_r16s32(0x0154,0x00000001);
	BSP_I2C_write_r16s32(0x0158,0x000000C8);
	//BSP_I2C_write_r16s32(0x0164,0x00000014);
	BSP_I2C_write_r16s32(0x0168,0x0000002A);
#if RESOLUT_2880_1440
	//BSP_I2C_write_r16s32(0x0170,0x000004E8);
	BSP_I2C_write_r16s32(0x0170,0x00000553);
#else
	printf("\n\rHDMI resolution 680*480 170\n\r");
	BSP_I2C_write_r16s32(0x0170,0x00000312);
#endif
	//BSP_I2C_write_r16s32(0x0178,0x00000001);
	BSP_I2C_write_r16s32(0x017C,0x00000081);
	BSP_I2C_write_r16s32(0x018C,0x00000001);
#if RESOLUT_2880_1440
	//BSP_I2C_write_r16s32(0x0190,0x0000009B);
	BSP_I2C_write_r16s32(0x0190,0x000000C8);
#else
	printf("\n\rHDMI resolution 680*480 190\n\r");
	BSP_I2C_write_r16s32(0x0190,0x0000023C);
#endif
	BSP_I2C_write_r16s32(0x01A4,0x00000000);
	//BSP_I2C_write_r16s32(0x01AC,0x00000000);
	BSP_I2C_write_r16s32(0x01C0,0x00000015);
	//BSP_I2C_write_r16s32(0x020C,0x00000000);
	//BSP_I2C_write_r16s32(0x0214,0x00000000);
	//BSP_I2C_write_r16s32(0x021C,0x00000080);
	//reg: 0x0214 data = 0xfffffca0       0x00000000
	//reg: 0x021C data = 0xffffc0a7       0x00000080
	BSP_I2C_write_r16s8(0x0214,0x00);
	BSP_I2C_write_r16s8(0x0215,0x00);
	BSP_I2C_write_r16s8(0x0216,0x00);
	BSP_I2C_write_r16s8(0x0217,0x00);
	BSP_I2C_write_r16s8(0x021C,0x80);
	BSP_I2C_write_r16s8(0x021D,0x00);
	BSP_I2C_write_r16s8(0x021E,0x00);
	BSP_I2C_write_r16s8(0x021F,0x00);

	BSP_I2C_write_r16s32(0x0224,0x00000000);
	//BSP_I2C_write_r16s32(0x0240,0x00000000);
#if RESOLUT_2880_1440
	BSP_I2C_write_r16s32(0x0254,0x00000007);
	BSP_I2C_write_r16s32(0x0258,0x00260006);
	BSP_I2C_write_r16s32(0x025C,0x000D0008);
	BSP_I2C_write_r16s32(0x0260,0x000E0006);
	BSP_I2C_write_r16s32(0x0264,0x00004E20);
	BSP_I2C_write_r16s32(0x0268,0x0000000D);
	BSP_I2C_write_r16s32(0x026C,0x000E0008);
	BSP_I2C_write_r16s32(0x0270,0x00000020);
	BSP_I2C_write_r16s32(0x0274,0x0000001F);
	BSP_I2C_write_r16s32(0x0278,0x00070007);
	BSP_I2C_write_r16s32(0x027C,0x00000002);
	//BSP_I2C_write_r16s32(0x0288,0x000002AA);
	BSP_I2C_write_r16s32(0x011C,0x00000001);
	//REM	DSI-TX1 Transition Timing
	BSP_I2C_write_r16s32(0x0308,0x00000001);
	BSP_I2C_write_r16s32(0x030C,0x00000001);
	BSP_I2C_write_r16s32(0x04A0,0x00000001);
	BSP_I2C_write_r16s32(0x04AC,0x0000507C);
	HAL_Delay(1);
	BSP_I2C_write_r16s32(0x04A0,0x00000003);
	BSP_I2C_write_r16s32(0x0318,0x00000014);
	BSP_I2C_write_r16s32(0x0320,0x00001770);
	BSP_I2C_write_r16s32(0x0324,0x00000000);
	BSP_I2C_write_r16s32(0x0328,0x00000101);
	//BSP_I2C_write_r16s32(0x032C,0x00000004);
	BSP_I2C_write_r16s32(0x0330,0x00010000);
	BSP_I2C_write_r16s32(0x0334,0x00005000);
	BSP_I2C_write_r16s32(0x0338,0x00010000);
	BSP_I2C_write_r16s32(0x033C,0x00010000);
	BSP_I2C_write_r16s32(0x0340,0x00010000);
	BSP_I2C_write_r16s32(0x0344,0x00010000);
	BSP_I2C_write_r16s32(0x0348,0x00001000);
	BSP_I2C_write_r16s32(0x034C,0x00010000);
	BSP_I2C_write_r16s32(0x0350,0x00000141);
	BSP_I2C_write_r16s32(0x0354,0x00000001);
	BSP_I2C_write_r16s32(0x0358,0x000000C8);
	//BSP_I2C_write_r16s32(0x0364,0x00000015);
	BSP_I2C_write_r16s32(0x0368,0x0000002A);
#if RESOLUT_2880_1440
	//BSP_I2C_write_r16s32(0x0370,0x000004E8);
	BSP_I2C_write_r16s32(0x0370,0x00000553);
#else
	printf("\n\rHDMI resolution 680*480 370\n\r");
	BSP_I2C_write_r16s32(0x0370,0x00000312);
#endif
	//BSP_I2C_write_r16s32(0x0378,0x00000001);
	BSP_I2C_write_r16s32(0x037C,0x00000081);
	BSP_I2C_write_r16s32(0x038C,0x00000001);
#if RESOLUT_2880_1440
	//BSP_I2C_write_r16s32(0x0390,0x0000009B);
	BSP_I2C_write_r16s32(0x0390,0x000000C8);
#else
	printf("\n\rHDMI resolution 680*480 390\n\r");
	BSP_I2C_write_r16s32(0x0390,0x0000023C);
#endif
	BSP_I2C_write_r16s32(0x03A4,0x00000000);
	//BSP_I2C_write_r16s32(0x03AC,0x00000000);
	BSP_I2C_write_r16s32(0x03C0,0x00000015);
	//BSP_I2C_write_r16s32(0x040C,0x00000000);
	BSP_I2C_write_r16s32(0x0414,0x00000000);
	BSP_I2C_write_r16s32(0x041C,0x00000080);
	BSP_I2C_write_r16s32(0x0424,0x00000000);
	//BSP_I2C_write_r16s32(0x0440,0x00000000);
	BSP_I2C_write_r16s32(0x0454,0x00000007);
	BSP_I2C_write_r16s32(0x0458,0x00260006);
	BSP_I2C_write_r16s32(0x045C,0x000D0008);
	BSP_I2C_write_r16s32(0x0460,0x000E0006);
	BSP_I2C_write_r16s32(0x0464,0x00004E20);
	BSP_I2C_write_r16s32(0x0468,0x0000000D);
	BSP_I2C_write_r16s32(0x046C,0x000E0008);
	BSP_I2C_write_r16s32(0x0470,0x00000020);
	BSP_I2C_write_r16s32(0x0474,0x0000001F);
	BSP_I2C_write_r16s32(0x0478,0x00070007);
	BSP_I2C_write_r16s32(0x047C,0x00000002);
	//BSP_I2C_write_r16s32(0x0488,0x00000000);
	BSP_I2C_write_r16s32(0x031C,0x00000001);
#else
	printf("\n\rHDMI resolution 680*480 0x0254-0x011C\n\r");
	BSP_I2C_write_r16s32(0x0254,0x00000001);
	BSP_I2C_write_r16s32(0x0258,0x00070001);
	BSP_I2C_write_r16s32(0x025C,0x00020000);
	BSP_I2C_write_r16s32(0x0260,0x00000001);
	BSP_I2C_write_r16s32(0x0264,0x000036B0);
	BSP_I2C_write_r16s32(0x0268,0x00000006);
	BSP_I2C_write_r16s32(0x026C,0x00020002);
	BSP_I2C_write_r16s32(0x0270,0x00000020);
	BSP_I2C_write_r16s32(0x0274,0x0000001F);
	BSP_I2C_write_r16s32(0x0278,0x00010000);
	BSP_I2C_write_r16s32(0x027C,0x00000002);
	BSP_I2C_write_r16s32(0x011C,0x00000001);
#endif
	//REM	Command Transmission Before Video Start
#if RESOLUT_2880_1440
	BSP_I2C_write_r16s16(0x0500,0x0004);
#else
	printf("\n\rHDMI resolution 640*480 500=000\n\r");
	BSP_I2C_write_r16s16(0x0500,0x0000);
#endif
	//BSP_I2C_write_r16s32(0x0110,0x00000016);
	//BSP_I2C_write_r16s32(0x0310,0x00000016);
	BSP_I2C_write_r16s8(0x0110,0x16);
	BSP_I2C_write_r16s8(0x0111,0x00);
	BSP_I2C_write_r16s8(0x0112,0x00);
	BSP_I2C_write_r16s8(0x0113,0x00);
	BSP_I2C_write_r16s8(0x0310,0x16);
	BSP_I2C_write_r16s8(0x0311,0x00);
	BSP_I2C_write_r16s8(0x0312,0x00);
	BSP_I2C_write_r16s8(0x0313,0x00);
	////////Panel Init
    //sleep out
	//-----[CMD1]0x11(type = 0x05)
	BSP_I2C_write_r16s16(0x0504,0x0005);
	BSP_I2C_write_r16s16(0x0504,0x0011);
	HAL_Delay(150);
	printf("\n\rHDMI Sleep out\n\r");
	BSP_I2C_write_r16s16(0x0504,0x0005);
	BSP_I2C_write_r16s16(0x0504,0x0029);
	HAL_Delay(40);
	printf("\n\rHDMI Display on\n\r");
	////////Panel Init End
	//REM	Split Control
#if RESOLUT_2880_1440
	BSP_I2C_write_r16s16(0x5000,0x0000);
	BSP_I2C_write_r16s16(0x500C,0xC4E0);
	BSP_I2C_write_r16s16(0x5080,0x0000);
#else
	printf("\n\rHDMI resolution 640*480 0x5000-0x508E\n\r");
	BSP_I2C_write_r16s16(0x5000,0x0000);
	BSP_I2C_write_r16s16(0x500C,0x0000);
	BSP_I2C_write_r16s16(0x500E,0x027F);
	BSP_I2C_write_r16s16(0x5080,0x0000);
	BSP_I2C_write_r16s16(0x508C,0x0000);
	BSP_I2C_write_r16s16(0x508E,0x027F);
#endif
	//REM	HDMI PHY
	BSP_I2C_write_r16s8(0x8410,0x03);
	BSP_I2C_write_r16s8(0x8413,0x3F);
	BSP_I2C_write_r16s8(0x8420,0x07);
	BSP_I2C_write_r16s8(0x84F0,0x31);
	BSP_I2C_write_r16s8(0x84F4,0x01);
	//REM	HDMI Clock
	BSP_I2C_write_r16s16(0x8540,0x12C0);
	BSP_I2C_write_r16s8(0x8630,0x00);
	BSP_I2C_write_r16s16(0x8631,0x0753);
	BSP_I2C_write_r16s8(0x8670,0x02);
	BSP_I2C_write_r16s16(0x8A0C,0x12C0);

	BSP_I2C_write_r16s8(0x850B,0xFF);
	BSP_I2C_write_r16s8(0x851B,0xFD);

	//REM	Interrupt Control (TOP level)
	BSP_I2C_write_r16s16(0x0014,0x0FBF);
	BSP_I2C_write_r16s16(0x0016,0x0DBF);
	//REM	EDID
	BSP_I2C_write_r16s8(0x85E0,0x01);
	BSP_I2C_write_r16s16(0x85E3,0x0100);
	//REM	EDID Data
	BSP_I2C_write_r16s8(0x8C00,0x00);
	BSP_I2C_write_r16s8(0x8C01,0xFF);
	BSP_I2C_write_r16s8(0x8C02,0xFF);
	BSP_I2C_write_r16s8(0x8C03,0xFF);
	BSP_I2C_write_r16s8(0x8C04,0xFF);
	BSP_I2C_write_r16s8(0x8C05,0xFF);
	BSP_I2C_write_r16s8(0x8C06,0xFF);
	BSP_I2C_write_r16s8(0x8C07,0x00);
	BSP_I2C_write_r16s8(0x8C08,0x52);
	BSP_I2C_write_r16s8(0x8C09,0x62);
	BSP_I2C_write_r16s8(0x8C0A,0x88);
	BSP_I2C_write_r16s8(0x8C0B,0x88);
	BSP_I2C_write_r16s8(0x8C0C,0x00);
	BSP_I2C_write_r16s8(0x8C0D,0x88);
	BSP_I2C_write_r16s8(0x8C0E,0x88);
	BSP_I2C_write_r16s8(0x8C0F,0x88);
	BSP_I2C_write_r16s8(0x8C10,0x1C);
	BSP_I2C_write_r16s8(0x8C11,0x15);
	BSP_I2C_write_r16s8(0x8C12,0x01);
	BSP_I2C_write_r16s8(0x8C13,0x03);
	BSP_I2C_write_r16s8(0x8C14,0x80);
	BSP_I2C_write_r16s8(0x8C15,0x00);
	BSP_I2C_write_r16s8(0x8C16,0x00);
	BSP_I2C_write_r16s8(0x8C17,0x78);
	BSP_I2C_write_r16s8(0x8C18,0x0A);
	BSP_I2C_write_r16s8(0x8C19,0x0D);
	BSP_I2C_write_r16s8(0x8C1A,0xC9);
	BSP_I2C_write_r16s8(0x8C1B,0xA0);
	BSP_I2C_write_r16s8(0x8C1C,0x57);
	BSP_I2C_write_r16s8(0x8C1D,0x47);
	BSP_I2C_write_r16s8(0x8C1E,0x98);
	BSP_I2C_write_r16s8(0x8C1F,0x27);
	BSP_I2C_write_r16s8(0x8C20,0x12);
	BSP_I2C_write_r16s8(0x8C21,0x48);
	BSP_I2C_write_r16s8(0x8C22,0x4C);
	BSP_I2C_write_r16s8(0x8C23,0x00);
	BSP_I2C_write_r16s8(0x8C24,0x00);
	BSP_I2C_write_r16s8(0x8C25,0x00);
	BSP_I2C_write_r16s8(0x8C26,0x01);
	BSP_I2C_write_r16s8(0x8C27,0x01);
	BSP_I2C_write_r16s8(0x8C28,0x01);
	BSP_I2C_write_r16s8(0x8C29,0x01);
	BSP_I2C_write_r16s8(0x8C2A,0x01);
	BSP_I2C_write_r16s8(0x8C2B,0x01);
	BSP_I2C_write_r16s8(0x8C2C,0x01);
	BSP_I2C_write_r16s8(0x8C2D,0x01);
	BSP_I2C_write_r16s8(0x8C2E,0x01);
	BSP_I2C_write_r16s8(0x8C2F,0x01);
	BSP_I2C_write_r16s8(0x8C30,0x01);
	BSP_I2C_write_r16s8(0x8C31,0x01);
	BSP_I2C_write_r16s8(0x8C32,0x01);
	BSP_I2C_write_r16s8(0x8C33,0x01);
	BSP_I2C_write_r16s8(0x8C34,0x01);
	BSP_I2C_write_r16s8(0x8C35,0x01);
#if RESOLUT_2880_1440
	BSP_I2C_write_r16s8(0x8C36,0x04);
	BSP_I2C_write_r16s8(0x8C37,0x74);
	BSP_I2C_write_r16s8(0x8C38,0x40);
	BSP_I2C_write_r16s8(0x8C39,0x06);//76
	BSP_I2C_write_r16s8(0x8C3A,0xB2);//B0
	BSP_I2C_write_r16s8(0x8C3B,0xA0);
	BSP_I2C_write_r16s8(0x8C3C,0x10);//09
	BSP_I2C_write_r16s8(0x8C3D,0x50);
	BSP_I2C_write_r16s8(0x8C3E,0x3E);//1A
	BSP_I2C_write_r16s8(0x8C3F,0x64);
	BSP_I2C_write_r16s8(0x8C40,0x82);
	BSP_I2C_write_r16s8(0x8C41,0x40);
	BSP_I2C_write_r16s8(0x8C42,0x40);
	BSP_I2C_write_r16s8(0x8C43,0xA0);
	BSP_I2C_write_r16s8(0x8C44,0xB5);
	BSP_I2C_write_r16s8(0x8C45,0x00);
	BSP_I2C_write_r16s8(0x8C46,0x00);
	BSP_I2C_write_r16s8(0x8C47,0x18);
	BSP_I2C_write_r16s8(0x8C48,0x04);
	BSP_I2C_write_r16s8(0x8C49,0x74);
	BSP_I2C_write_r16s8(0x8C4A,0x40);
	BSP_I2C_write_r16s8(0x8C4B,0x06);
	BSP_I2C_write_r16s8(0x8C4C,0xB2);
	BSP_I2C_write_r16s8(0x8C4D,0xA0);
	BSP_I2C_write_r16s8(0x8C4E,0x10);
	BSP_I2C_write_r16s8(0x8C4F,0x50);
	BSP_I2C_write_r16s8(0x8C50,0x3E);
	BSP_I2C_write_r16s8(0x8C51,0x64);
	BSP_I2C_write_r16s8(0x8C52,0x82);
	BSP_I2C_write_r16s8(0x8C53,0x40);
	BSP_I2C_write_r16s8(0x8C54,0x40);
	BSP_I2C_write_r16s8(0x8C55,0xA0);
	BSP_I2C_write_r16s8(0x8C56,0xB5);
	BSP_I2C_write_r16s8(0x8C57,0x00);
	BSP_I2C_write_r16s8(0x8C58,0x00);
	BSP_I2C_write_r16s8(0x8C59,0x18);//
	BSP_I2C_write_r16s8(0x8C5A,0x00);
	BSP_I2C_write_r16s8(0x8C5B,0x00);
	BSP_I2C_write_r16s8(0x8C5C,0x00);
	BSP_I2C_write_r16s8(0x8C5D,0xFC);
	BSP_I2C_write_r16s8(0x8C5E,0x00);
	BSP_I2C_write_r16s8(0x8C5F,0x54);
	BSP_I2C_write_r16s8(0x8C60,0x6F);
	BSP_I2C_write_r16s8(0x8C61,0x73);
	BSP_I2C_write_r16s8(0x8C62,0x68);
	BSP_I2C_write_r16s8(0x8C63,0x69);
	BSP_I2C_write_r16s8(0x8C64,0x62);
	BSP_I2C_write_r16s8(0x8C65,0x61);
	BSP_I2C_write_r16s8(0x8C66,0x2D);
	BSP_I2C_write_r16s8(0x8C67,0x55);
	BSP_I2C_write_r16s8(0x8C68,0x48);
	BSP_I2C_write_r16s8(0x8C69,0x32);
	BSP_I2C_write_r16s8(0x8C6A,0x44);
	BSP_I2C_write_r16s8(0x8C6B,0x0A);
	BSP_I2C_write_r16s8(0x8C6C,0x00);
	BSP_I2C_write_r16s8(0x8C6D,0x00);
	BSP_I2C_write_r16s8(0x8C6E,0x00);
	BSP_I2C_write_r16s8(0x8C6F,0xFD);
	BSP_I2C_write_r16s8(0x8C70,0x00);
	BSP_I2C_write_r16s8(0x8C71,0x14);
	BSP_I2C_write_r16s8(0x8C72,0x78);
	BSP_I2C_write_r16s8(0x8C73,0x01);
	BSP_I2C_write_r16s8(0x8C74,0xFF);
	BSP_I2C_write_r16s8(0x8C75,0x1D);
	BSP_I2C_write_r16s8(0x8C76,0x00);
	BSP_I2C_write_r16s8(0x8C77,0x0A);
	BSP_I2C_write_r16s8(0x8C78,0x20);
	BSP_I2C_write_r16s8(0x8C79,0x20);
	BSP_I2C_write_r16s8(0x8C7A,0x20);
	BSP_I2C_write_r16s8(0x8C7B,0x20);
	BSP_I2C_write_r16s8(0x8C7C,0x20);
	BSP_I2C_write_r16s8(0x8C7D,0x20);
	BSP_I2C_write_r16s8(0x8C7E,0x01);
	BSP_I2C_write_r16s8(0x8C7F,0x67);
	BSP_I2C_write_r16s8(0x8C80,0x02);
	BSP_I2C_write_r16s8(0x8C81,0x03);
	BSP_I2C_write_r16s8(0x8C82,0x1A);
	BSP_I2C_write_r16s8(0x8C83,0x71);
	BSP_I2C_write_r16s8(0x8C84,0x47);
	BSP_I2C_write_r16s8(0x8C85,0xC6);
	BSP_I2C_write_r16s8(0x8C86,0x46);
	BSP_I2C_write_r16s8(0x8C87,0x46);
	BSP_I2C_write_r16s8(0x8C88,0x46);
	BSP_I2C_write_r16s8(0x8C89,0x46);
	BSP_I2C_write_r16s8(0x8C8A,0x46);
	BSP_I2C_write_r16s8(0x8C8B,0x46);
	BSP_I2C_write_r16s8(0x8C8C,0x23);
	BSP_I2C_write_r16s8(0x8C8D,0x09);
	BSP_I2C_write_r16s8(0x8C8E,0x07);
	BSP_I2C_write_r16s8(0x8C8F,0x01);
	BSP_I2C_write_r16s8(0x8C90,0x83);
	BSP_I2C_write_r16s8(0x8C91,0x01);
	BSP_I2C_write_r16s8(0x8C92,0x00);
	BSP_I2C_write_r16s8(0x8C93,0x00);
	BSP_I2C_write_r16s8(0x8C94,0x65);
	BSP_I2C_write_r16s8(0x8C95,0x03);
	BSP_I2C_write_r16s8(0x8C96,0x0C);
	BSP_I2C_write_r16s8(0x8C97,0x00);
	BSP_I2C_write_r16s8(0x8C98,0x10);
	BSP_I2C_write_r16s8(0x8C99,0x00);//
	BSP_I2C_write_r16s8(0x8C9A,0x04);
	BSP_I2C_write_r16s8(0x8C9B,0x74);
	BSP_I2C_write_r16s8(0x8C9C,0x40);
	BSP_I2C_write_r16s8(0x8C9D,0x06);
	BSP_I2C_write_r16s8(0x8C9E,0xB2);
	BSP_I2C_write_r16s8(0x8C9F,0xA0);
	BSP_I2C_write_r16s8(0x8CA0,0x10);
	BSP_I2C_write_r16s8(0x8CA1,0x50);
	BSP_I2C_write_r16s8(0x8CA2,0x3E);
	BSP_I2C_write_r16s8(0x8CA3,0x64);
	BSP_I2C_write_r16s8(0x8CA4,0x82);
	BSP_I2C_write_r16s8(0x8CA5,0x40);
	BSP_I2C_write_r16s8(0x8CA6,0x40);
	BSP_I2C_write_r16s8(0x8CA7,0xA0);
	BSP_I2C_write_r16s8(0x8CA8,0xB5);
	BSP_I2C_write_r16s8(0x8CA9,0x00);
	BSP_I2C_write_r16s8(0x8CAA,0x00);
	BSP_I2C_write_r16s8(0x8CAB,0x18);
	BSP_I2C_write_r16s8(0x8CAC,0x04);
	BSP_I2C_write_r16s8(0x8CAD,0x74);
	BSP_I2C_write_r16s8(0x8CAE,0x40);
	BSP_I2C_write_r16s8(0x8CAF,0x06);
	BSP_I2C_write_r16s8(0x8CB0,0xB2);
	BSP_I2C_write_r16s8(0x8CB1,0xA0);
	BSP_I2C_write_r16s8(0x8CB2,0x10);
	BSP_I2C_write_r16s8(0x8CB3,0x50);
	BSP_I2C_write_r16s8(0x8CB4,0x3E);
	BSP_I2C_write_r16s8(0x8CB5,0x64);
	BSP_I2C_write_r16s8(0x8CB6,0x82);
	BSP_I2C_write_r16s8(0x8CB7,0x40);
	BSP_I2C_write_r16s8(0x8CB8,0x40);
	BSP_I2C_write_r16s8(0x8CB9,0xA0);
	BSP_I2C_write_r16s8(0x8CBA,0xB5);
	BSP_I2C_write_r16s8(0x8CBB,0x00);
	BSP_I2C_write_r16s8(0x8CBC,0x00);
	BSP_I2C_write_r16s8(0x8CBD,0x18);
	BSP_I2C_write_r16s8(0x8CBE,0x04);
	BSP_I2C_write_r16s8(0x8CBF,0x74);
	BSP_I2C_write_r16s8(0x8CC0,0x40);
	BSP_I2C_write_r16s8(0x8CC1,0x06);
	BSP_I2C_write_r16s8(0x8CC2,0xB2);
	BSP_I2C_write_r16s8(0x8CC3,0xA0);
	BSP_I2C_write_r16s8(0x8CC4,0x10);
	BSP_I2C_write_r16s8(0x8CC5,0x50);
	BSP_I2C_write_r16s8(0x8CC6,0x3E);
	BSP_I2C_write_r16s8(0x8CC7,0x64);
	BSP_I2C_write_r16s8(0x8CC8,0x82);
	BSP_I2C_write_r16s8(0x8CC9,0x40);
	BSP_I2C_write_r16s8(0x8CCA,0x40);
	BSP_I2C_write_r16s8(0x8CCB,0xA0);
	BSP_I2C_write_r16s8(0x8CCC,0xB5);
	BSP_I2C_write_r16s8(0x8CCD,0x00);
	BSP_I2C_write_r16s8(0x8CCE,0x00);
	BSP_I2C_write_r16s8(0x8CCF,0x18);
	BSP_I2C_write_r16s8(0x8CD0,0x04);
	BSP_I2C_write_r16s8(0x8CD1,0x74);
	BSP_I2C_write_r16s8(0x8CD2,0x40);
	BSP_I2C_write_r16s8(0x8CD3,0x06);
	BSP_I2C_write_r16s8(0x8CD4,0xB2);
	BSP_I2C_write_r16s8(0x8CD5,0xA0);
	BSP_I2C_write_r16s8(0x8CD6,0x10);
	BSP_I2C_write_r16s8(0x8CD7,0x50);
	BSP_I2C_write_r16s8(0x8CD8,0x3E);
	BSP_I2C_write_r16s8(0x8CD9,0x64);
	BSP_I2C_write_r16s8(0x8CDA,0x82);
	BSP_I2C_write_r16s8(0x8CDB,0x40);
	BSP_I2C_write_r16s8(0x8CDC,0x40);
	BSP_I2C_write_r16s8(0x8CDD,0xA0);
	BSP_I2C_write_r16s8(0x8CDE,0xB5);
	BSP_I2C_write_r16s8(0x8CDF,0x00);
	BSP_I2C_write_r16s8(0x8CE0,0x00);
	BSP_I2C_write_r16s8(0x8CE1,0x18);
	BSP_I2C_write_r16s8(0x8CE2,0x00);
	BSP_I2C_write_r16s8(0x8CE3,0x00);
	BSP_I2C_write_r16s8(0x8CE4,0x00);
	BSP_I2C_write_r16s8(0x8CE5,0x00);
	BSP_I2C_write_r16s8(0x8CE6,0x00);
	BSP_I2C_write_r16s8(0x8CE7,0x00);
	BSP_I2C_write_r16s8(0x8CE8,0x00);
	BSP_I2C_write_r16s8(0x8CE9,0x00);
	BSP_I2C_write_r16s8(0x8CEA,0x00);
	BSP_I2C_write_r16s8(0x8CEB,0x00);
	BSP_I2C_write_r16s8(0x8CEC,0x00);
	BSP_I2C_write_r16s8(0x8CED,0x00);
	BSP_I2C_write_r16s8(0x8CEE,0x00);
	BSP_I2C_write_r16s8(0x8CEF,0x00);
	BSP_I2C_write_r16s8(0x8CF0,0x00);
	BSP_I2C_write_r16s8(0x8CF1,0x00);
	BSP_I2C_write_r16s8(0x8CF2,0x00);
	BSP_I2C_write_r16s8(0x8CF3,0x00);
	BSP_I2C_write_r16s8(0x8CF4,0x00);
	BSP_I2C_write_r16s8(0x8CF5,0x00);
	BSP_I2C_write_r16s8(0x8CF6,0x00);
	BSP_I2C_write_r16s8(0x8CF7,0x00);
	BSP_I2C_write_r16s8(0x8CF8,0x00);
	BSP_I2C_write_r16s8(0x8CF9,0x00);
	BSP_I2C_write_r16s8(0x8CFA,0x00);
	BSP_I2C_write_r16s8(0x8CFB,0x00);
	BSP_I2C_write_r16s8(0x8CFC,0x00);
	BSP_I2C_write_r16s8(0x8CFD,0x00);
	BSP_I2C_write_r16s8(0x8CFE,0x00);
	BSP_I2C_write_r16s8(0x8CFF,0x7F);
#else
	printf("\n\rHDMI resolution 640*480 0x8C36-0x8CFF\n\r");
	BSP_I2C_write_r16s8(0x8C36,0xD8);
	BSP_I2C_write_r16s8(0x8C37,0x09);
	BSP_I2C_write_r16s8(0x8C38,0x80);
	BSP_I2C_write_r16s8(0x8C39,0xA0);
	BSP_I2C_write_r16s8(0x8C3A,0x20);
	BSP_I2C_write_r16s8(0x8C3B,0xE0);
	BSP_I2C_write_r16s8(0x8C3C,0x2D);
	BSP_I2C_write_r16s8(0x8C3D,0x10);
	BSP_I2C_write_r16s8(0x8C3E,0x10);
	BSP_I2C_write_r16s8(0x8C3F,0x60);
	BSP_I2C_write_r16s8(0x8C40,0xA2);
	BSP_I2C_write_r16s8(0x8C41,0x00);
	BSP_I2C_write_r16s8(0x8C42,0x80);
	BSP_I2C_write_r16s8(0x8C43,0xE0);
	BSP_I2C_write_r16s8(0x8C44,0x21);
	BSP_I2C_write_r16s8(0x8C45,0x00);
	BSP_I2C_write_r16s8(0x8C46,0x00);
	BSP_I2C_write_r16s8(0x8C47,0x18);
	BSP_I2C_write_r16s8(0x8C48,0xD8);
	BSP_I2C_write_r16s8(0x8C49,0x09);
	BSP_I2C_write_r16s8(0x8C4A,0x80);
	BSP_I2C_write_r16s8(0x8C4B,0xA0);
	BSP_I2C_write_r16s8(0x8C4C,0x20);
	BSP_I2C_write_r16s8(0x8C4D,0xE0);
	BSP_I2C_write_r16s8(0x8C4E,0x2D);
	BSP_I2C_write_r16s8(0x8C4F,0x10);
	BSP_I2C_write_r16s8(0x8C50,0x10);
	BSP_I2C_write_r16s8(0x8C51,0x60);
	BSP_I2C_write_r16s8(0x8C52,0xA2);
	BSP_I2C_write_r16s8(0x8C53,0x00);
	BSP_I2C_write_r16s8(0x8C54,0x80);
	BSP_I2C_write_r16s8(0x8C55,0xE0);
	BSP_I2C_write_r16s8(0x8C56,0x21);
	BSP_I2C_write_r16s8(0x8C57,0x00);
	BSP_I2C_write_r16s8(0x8C58,0x00);
	BSP_I2C_write_r16s8(0x8C59,0x18);
	BSP_I2C_write_r16s8(0x8C5A,0x00);
	BSP_I2C_write_r16s8(0x8C5B,0x00);
	BSP_I2C_write_r16s8(0x8C5C,0x00);
	BSP_I2C_write_r16s8(0x8C5D,0xFC);
	BSP_I2C_write_r16s8(0x8C5E,0x00);
	BSP_I2C_write_r16s8(0x8C5F,0x54);
	BSP_I2C_write_r16s8(0x8C60,0x6F);
	BSP_I2C_write_r16s8(0x8C61,0x73);
	BSP_I2C_write_r16s8(0x8C62,0x68);
	BSP_I2C_write_r16s8(0x8C63,0x69);
	BSP_I2C_write_r16s8(0x8C64,0x62);
	BSP_I2C_write_r16s8(0x8C65,0x61);
	BSP_I2C_write_r16s8(0x8C66,0x2D);
	BSP_I2C_write_r16s8(0x8C67,0x55);
	BSP_I2C_write_r16s8(0x8C68,0x48);
	BSP_I2C_write_r16s8(0x8C69,0x32);
	BSP_I2C_write_r16s8(0x8C6A,0x44);
	BSP_I2C_write_r16s8(0x8C6B,0x0A);
	BSP_I2C_write_r16s8(0x8C6C,0x00);
	BSP_I2C_write_r16s8(0x8C6D,0x00);
	BSP_I2C_write_r16s8(0x8C6E,0x00);
	BSP_I2C_write_r16s8(0x8C6F,0xFD);
	BSP_I2C_write_r16s8(0x8C70,0x00);
	BSP_I2C_write_r16s8(0x8C71,0x14);
	BSP_I2C_write_r16s8(0x8C72,0x78);
	BSP_I2C_write_r16s8(0x8C73,0x01);
	BSP_I2C_write_r16s8(0x8C74,0xFF);
	BSP_I2C_write_r16s8(0x8C75,0x1D);
	BSP_I2C_write_r16s8(0x8C76,0x00);
	BSP_I2C_write_r16s8(0x8C77,0x0A);
	BSP_I2C_write_r16s8(0x8C78,0x20);
	BSP_I2C_write_r16s8(0x8C79,0x20);
	BSP_I2C_write_r16s8(0x8C7A,0x20);
	BSP_I2C_write_r16s8(0x8C7B,0x20);
	BSP_I2C_write_r16s8(0x8C7C,0x20);
	BSP_I2C_write_r16s8(0x8C7D,0x20);
	BSP_I2C_write_r16s8(0x8C7E,0x01);
	BSP_I2C_write_r16s8(0x8C7F,0x97);
	BSP_I2C_write_r16s8(0x8C80,0x02);
	BSP_I2C_write_r16s8(0x8C81,0x03);
	BSP_I2C_write_r16s8(0x8C82,0x1A);
	BSP_I2C_write_r16s8(0x8C83,0x71);
	BSP_I2C_write_r16s8(0x8C84,0x47);
	BSP_I2C_write_r16s8(0x8C85,0xC6);
	BSP_I2C_write_r16s8(0x8C86,0x46);
	BSP_I2C_write_r16s8(0x8C87,0x46);
	BSP_I2C_write_r16s8(0x8C88,0x46);
	BSP_I2C_write_r16s8(0x8C89,0x46);
	BSP_I2C_write_r16s8(0x8C8A,0x46);
	BSP_I2C_write_r16s8(0x8C8B,0x46);
	BSP_I2C_write_r16s8(0x8C8C,0x23);
	BSP_I2C_write_r16s8(0x8C8D,0x09);
	BSP_I2C_write_r16s8(0x8C8E,0x07);
	BSP_I2C_write_r16s8(0x8C8F,0x01);
	BSP_I2C_write_r16s8(0x8C90,0x83);
	BSP_I2C_write_r16s8(0x8C91,0x01);
	BSP_I2C_write_r16s8(0x8C92,0x00);
	BSP_I2C_write_r16s8(0x8C93,0x00);
	BSP_I2C_write_r16s8(0x8C94,0x65);
	BSP_I2C_write_r16s8(0x8C95,0x03);
	BSP_I2C_write_r16s8(0x8C96,0x0C);
	BSP_I2C_write_r16s8(0x8C97,0x00);
	BSP_I2C_write_r16s8(0x8C98,0x10);
	BSP_I2C_write_r16s8(0x8C99,0x00);
	BSP_I2C_write_r16s8(0x8C9A,0xD8);
	BSP_I2C_write_r16s8(0x8C9B,0x09);
	BSP_I2C_write_r16s8(0x8C9C,0x80);
	BSP_I2C_write_r16s8(0x8C9D,0xA0);
	BSP_I2C_write_r16s8(0x8C9E,0x20);
	BSP_I2C_write_r16s8(0x8C9F,0xE0);
	BSP_I2C_write_r16s8(0x8CA0,0x2D);
	BSP_I2C_write_r16s8(0x8CA1,0x10);
	BSP_I2C_write_r16s8(0x8CA2,0x10);
	BSP_I2C_write_r16s8(0x8CA3,0x60);
	BSP_I2C_write_r16s8(0x8CA4,0xA2);
	BSP_I2C_write_r16s8(0x8CA5,0x00);
	BSP_I2C_write_r16s8(0x8CA6,0x80);
	BSP_I2C_write_r16s8(0x8CA7,0xE0);
	BSP_I2C_write_r16s8(0x8CA8,0x21);
	BSP_I2C_write_r16s8(0x8CA9,0x00);
	BSP_I2C_write_r16s8(0x8CAA,0x00);
	BSP_I2C_write_r16s8(0x8CAB,0x18);
	BSP_I2C_write_r16s8(0x8CAC,0xD8);
	BSP_I2C_write_r16s8(0x8CAD,0x09);
	BSP_I2C_write_r16s8(0x8CAE,0x80);
	BSP_I2C_write_r16s8(0x8CAF,0xA0);
	BSP_I2C_write_r16s8(0x8CB0,0x20);
	BSP_I2C_write_r16s8(0x8CB1,0xE0);
	BSP_I2C_write_r16s8(0x8CB2,0x2D);
	BSP_I2C_write_r16s8(0x8CB3,0x10);
	BSP_I2C_write_r16s8(0x8CB4,0x10);
	BSP_I2C_write_r16s8(0x8CB5,0x60);
	BSP_I2C_write_r16s8(0x8CB6,0xA2);
	BSP_I2C_write_r16s8(0x8CB7,0x00);
	BSP_I2C_write_r16s8(0x8CB8,0x80);
	BSP_I2C_write_r16s8(0x8CB9,0xE0);
	BSP_I2C_write_r16s8(0x8CBA,0x21);
	BSP_I2C_write_r16s8(0x8CBB,0x00);
	BSP_I2C_write_r16s8(0x8CBC,0x00);
	BSP_I2C_write_r16s8(0x8CBD,0x18);
	BSP_I2C_write_r16s8(0x8CBE,0xD8);
	BSP_I2C_write_r16s8(0x8CBF,0x09);
	BSP_I2C_write_r16s8(0x8CC0,0x80);
	BSP_I2C_write_r16s8(0x8CC1,0xA0);
	BSP_I2C_write_r16s8(0x8CC2,0x20);
	BSP_I2C_write_r16s8(0x8CC3,0xE0);
	BSP_I2C_write_r16s8(0x8CC4,0x2D);
	BSP_I2C_write_r16s8(0x8CC5,0x10);
	BSP_I2C_write_r16s8(0x8CC6,0x10);
	BSP_I2C_write_r16s8(0x8CC7,0x60);
	BSP_I2C_write_r16s8(0x8CC8,0xA2);
	BSP_I2C_write_r16s8(0x8CC9,0x00);
	BSP_I2C_write_r16s8(0x8CCA,0x80);
	BSP_I2C_write_r16s8(0x8CCB,0xE0);
	BSP_I2C_write_r16s8(0x8CCC,0x21);
	BSP_I2C_write_r16s8(0x8CCD,0x00);
	BSP_I2C_write_r16s8(0x8CCE,0x00);
	BSP_I2C_write_r16s8(0x8CCF,0x18);
	BSP_I2C_write_r16s8(0x8CD0,0xD8);
	BSP_I2C_write_r16s8(0x8CD1,0x09);
	BSP_I2C_write_r16s8(0x8CD2,0x80);
	BSP_I2C_write_r16s8(0x8CD3,0xA0);
	BSP_I2C_write_r16s8(0x8CD4,0x20);
	BSP_I2C_write_r16s8(0x8CD5,0xE0);
	BSP_I2C_write_r16s8(0x8CD6,0x2D);
	BSP_I2C_write_r16s8(0x8CD7,0x10);
	BSP_I2C_write_r16s8(0x8CD8,0x10);
	BSP_I2C_write_r16s8(0x8CD9,0x60);
	BSP_I2C_write_r16s8(0x8CDA,0xA2);
	BSP_I2C_write_r16s8(0x8CDB,0x00);
	BSP_I2C_write_r16s8(0x8CDC,0x80);
	BSP_I2C_write_r16s8(0x8CDD,0xE0);
	BSP_I2C_write_r16s8(0x8CDE,0x21);
	BSP_I2C_write_r16s8(0x8CDF,0x00);
	BSP_I2C_write_r16s8(0x8CE0,0x00);
	BSP_I2C_write_r16s8(0x8CE1,0x18);
	BSP_I2C_write_r16s8(0x8CE2,0x00);
	BSP_I2C_write_r16s8(0x8CE3,0x00);
	BSP_I2C_write_r16s8(0x8CE4,0x00);
	BSP_I2C_write_r16s8(0x8CE5,0x00);
	BSP_I2C_write_r16s8(0x8CE6,0x00);
	BSP_I2C_write_r16s8(0x8CE7,0x00);
	BSP_I2C_write_r16s8(0x8CE8,0x00);
	BSP_I2C_write_r16s8(0x8CE9,0x00);
	BSP_I2C_write_r16s8(0x8CEA,0x00);
	BSP_I2C_write_r16s8(0x8CEB,0x00);
	BSP_I2C_write_r16s8(0x8CEC,0x00);
	BSP_I2C_write_r16s8(0x8CED,0x00);
	BSP_I2C_write_r16s8(0x8CEE,0x00);
	BSP_I2C_write_r16s8(0x8CEF,0x00);
	BSP_I2C_write_r16s8(0x8CF0,0x00);
	BSP_I2C_write_r16s8(0x8CF1,0x00);
	BSP_I2C_write_r16s8(0x8CF2,0x00);
	BSP_I2C_write_r16s8(0x8CF3,0x00);
	BSP_I2C_write_r16s8(0x8CF4,0x00);
	BSP_I2C_write_r16s8(0x8CF5,0x00);
	BSP_I2C_write_r16s8(0x8CF6,0x00);
	BSP_I2C_write_r16s8(0x8CF7,0x00);
	BSP_I2C_write_r16s8(0x8CF8,0x00);
	BSP_I2C_write_r16s8(0x8CF9,0x00);
	BSP_I2C_write_r16s8(0x8CFA,0x00);
	BSP_I2C_write_r16s8(0x8CFB,0x00);
	BSP_I2C_write_r16s8(0x8CFC,0x00);
	BSP_I2C_write_r16s8(0x8CFD,0x00);
	BSP_I2C_write_r16s8(0x8CFE,0x00);
	BSP_I2C_write_r16s8(0x8CFF,0xDF);
#endif
	//REM	Video Color Format Se,0xng);
	BSP_I2C_write_r16s8(0x8A02,0x42);
	//REM	HDMI SYSTEM
	BSP_I2C_write_r16s8(0x8543,0x02);
	BSP_I2C_write_r16s8(0x8544,0x10);
	//REM	HDMI Audio Setting
	BSP_I2C_write_r16s8(0x8600,0x00);
	BSP_I2C_write_r16s8(0x8602,0xF3);
	BSP_I2C_write_r16s8(0x8603,0x02);
	BSP_I2C_write_r16s8(0x8604,0x0C);
	BSP_I2C_write_r16s8(0x8606,0x05);
	BSP_I2C_write_r16s8(0x8607,0x00);
	BSP_I2C_write_r16s8(0x8652,0x01);
	//BSP_I2C_write_r16s32(0x8671,0x020C49BA);
	//BSP_I2C_write_r16s32(0x8675,0x01E1B089);
	BSP_I2C_write_r16s8(0x8671,0xBA);
	BSP_I2C_write_r16s8(0x8672,0x49);
	BSP_I2C_write_r16s8(0x8673,0x0C);
	BSP_I2C_write_r16s8(0x8674,0x02);
	BSP_I2C_write_r16s8(0x8675,0x89);
	BSP_I2C_write_r16s8(0x8676,0xB0);
	BSP_I2C_write_r16s8(0x8677,0xE1);
	BSP_I2C_write_r16s8(0x8678,0x01);
	BSP_I2C_write_r16s8(0x8680,0x00);
	//REM	Let HDMI Source start access
	BSP_I2C_write_r16s8(0x854A,0x01);
	//REM	HDMI Signal Detection
	//REM	Wait until HDMI sync is established
	//REM	By Interrupt   In Interrupt Service Routine.
	int hdmi_signal=0;
	for(hdmi_signal=0;hdmi_signal<5;hdmi_signal++)
	{
	do{
		BSP_I2C_ReadMulti(0x8520,data,1);
	    HAL_Delay(50);
		printf("\n\rHDMI Signal Detection0 data0 = 0x%x \n\r", data[0]);
	}while((data[0] & 0x80)!= 0x80);
	}
	//REM	Sequence: Check bit7 of 8x8520
	BSP_I2C_write_r16s8(0x850B,0xFF);
	BSP_I2C_write_r16s16(0x0014,0x0FBF);
	//REM	By Polling
	for(hdmi_signal=0;hdmi_signal<5;hdmi_signal++)
	{
	do{
		BSP_I2C_ReadMulti(0x8520,data,1);
	    HAL_Delay(50);
		printf("\n\rHDMI Signal Detection1 data0 = 0x%x \n\r", data[0]);
	}while((data[0] & 0x80)!= 0x80);
	}
	API_8870_HDMI_InputSignal();
	//REM	Sequence: Check bit7 of 8x8520
	//REM	Start Video TX
#if RESOLUT_2880_1440
	BSP_I2C_write_r16s16(0x0004,0x0C37);
#else
	printf("\n\rHDMI resolution 640*480 0x0004\n\r");
	BSP_I2C_write_r16s16(0x0004,0x0C35);
#endif
	BSP_I2C_write_r16s16(0x0006,0x0000);
	printf("\n\rHDMI Video Start\n\r");
	//REM	Command Transmission After Video Start.
	//BSP_I2C_write_r16s32(0x0110,0x00000016);
	//BSP_I2C_write_r16s32(0x0310,0x00000016);
	BSP_I2C_write_r16s8(0x0110,0x06);
	BSP_I2C_write_r16s8(0x0111,0x00);
	BSP_I2C_write_r16s8(0x0112,0x00);
	BSP_I2C_write_r16s8(0x0113,0x00);
	BSP_I2C_write_r16s8(0x0310,0x06);
	BSP_I2C_write_r16s8(0x0311,0x00);
	BSP_I2C_write_r16s8(0x0312,0x00);
	BSP_I2C_write_r16s8(0x0313,0x00);
	////////Panel Init
	//-----[CMD1]0x29, 0x05(type = 0x05) display on
	BSP_I2C_write_r16s16(0x0504,0x0005);
	BSP_I2C_write_r16s16(0x0504,0x0011);
	HAL_Delay(150);
	printf("\n\rHDMI Sleep out\n\r");
	BSP_I2C_write_r16s16(0x0504,0x0005);
	BSP_I2C_write_r16s16(0x0504,0x0029);
	//HAL_Delay(40);
	printf("\n\rHDMI Display on\n\r");
	////////Panel Init End

	//API_8870_resolution_init_check_reverse();
	return true;
}

void API_7737_Check(void)
{
	uint8_t data=0x0;
	printf("\n\rAPI_7737_Check...\n\r");
	//BSP_I2C_ID_set(0x50>>1,);
	//1. 首先帮忙确认下：读7737 0x50:0x00～0x50:0x03 能够得到7737的slave ID,正确的值是aa aa 38 77. 这个是正确的吗？
	//2. 如果上边的chip ID读正确了，然后帮忙check 0x8C:0x11, 0x8C:0x17, 0x8C:0x18, 0x8c:0x0a(new one for the link bandwidth) and 0x50:0x0b；
	BSP_I2C_read_reg(0x00,&data,1);
	printf("\n\rAPI_7737_Check 0x00 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x01,&data,1);
	printf("\n\rAPI_7737_Check 0x01 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x02,&data,1);
	printf("\n\rAPI_7737_Check 0x02 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x03,&data,1);
	printf("\n\rAPI_7737_Check 0x03 data = 0x%x \n\r", data);
	//BSP_I2C_ID_set(0x8C>>1);
	BSP_I2C_read_reg(0x011,&data,1);//0x8c:0x11 是0x07， 然后读0x8c:0x17 & 0x8c:0x18,这个值应该是0x00 &0x80
	printf("\n\rAPI_7737_Check 0x11 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x17,&data,1);
	printf("\n\rAPI_7737_Check 0x17 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x18,&data,1);
	printf("\n\rAPI_7737_Check 0x18 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x0A,&data,1);
	printf("\n\rAPI_7737_Check 0x0A data = 0x%x \n\r", data);
	//BSP_I2C_ID_set(0x50>>1);
	BSP_I2C_read_reg(0x0A,&data,1);
	printf("\n\rAPI_7737_Check 0x0A data = 0x%x \n\r", data);
}

void API_PNI_Check(void)
{
	uint8_t data=0x0;
	printf("\n\rAPI_PNI_Check...\n\r");
	BSP_I2C_ID_set(0x28,1);
	BSP_I2C_read_reg(0x90,&data,1);
	printf("\n\rAPI_PNI_Check 0x90 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x91,&data,1);
	printf("\n\rAPI_PNI_Check 0x91 data = 0x%x \n\r", data);
	
	BSP_I2C_read_reg(0x37,&data,1);
	printf("\n\rAPI_PNI_Check 0x37 data = 0x%x \n\r", data);	
	if(data & 0x01)
	{
	    BSP_I2C_write_reg(0x9B,0x01);
					HAL_Delay(1000);
			  printf("\n\rAPI_PNI_Check write 0x9B\n\r");	
	}
	BSP_I2C_read_reg(0x37,&data,1);
	printf("\n\rAPI_PNI_Check 0x37 data = 0x%x \n\r", data);	

	BSP_I2C_write_reg(0x55,0x64);//100
	BSP_I2C_write_reg(0x56,0x14);//20
	BSP_I2C_write_reg(0x57,0x14);//20
	
	BSP_I2C_write_reg(0x34,0x01);
	BSP_I2C_write_reg(0x33,0x20);
	BSP_I2C_write_reg(0x54,0x02);
	
	BSP_I2C_read_reg(0x45,&data,1);
	printf("\n\rAPI_PNI_Check 0x45 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x46,&data,1);
	printf("\n\rAPI_PNI_Check 0x46 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x47,&data,1);
	printf("\n\rAPI_PNI_Check 0x47 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x1E,&data,1);
	printf("\n\rAPI_PNI_Check 0x1E data = 0x%x \n\r", data);	
	BSP_I2C_read_reg(0x1F,&data,1);
	printf("\n\rAPI_PNI_Check 0x1F data = 0x%x \n\r", data);		
	BSP_I2C_read_reg(0x50,&data,1);
	printf("\n\rAPI_PNI_Check 0x50 data = 0x%x \n\r", data);		

	BSP_I2C_read_reg(0x97,&data,1);
	printf("\n\rAPI_PNI_Check 0x97 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x98,&data,1);
	printf("\n\rAPI_PNI_Check 0x98 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x99,&data,1);
	printf("\n\rAPI_PNI_Check 0x99 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x9A,&data,1);
	printf("\n\rAPI_PNI_Check 0x9A data = 0x%x \n\r", data);	


	BSP_I2C_write_reg(0xA0,0x01);
	BSP_I2C_ID_set(0x68,1);
	BSP_I2C_read_reg(0x00,&data,1);
	printf("\n\rBMI 0x00 data = 0x%x \n\r", data);

	BSP_I2C_ID_set(0x0C,1);
	BSP_I2C_read_reg(0x00,&data,1);
	printf("\n\rAKM 0x00 data = 0x%x \n\r", data);
	
}	

void API_LTR559_Check(void)
{
	uint8_t data=0x0;
	printf("\n\rAPI_LTR559_Check...\n\r");
	BSP_I2C_ID_set(0x23,1);
	BSP_I2C_read_reg(0x86,&data,1);
	printf("\n\rAPI_LTR559_Check 0x86 data = 0x%x \n\r", data);
	BSP_I2C_read_reg(0x87,&data,1);
	printf("\n\rAPI_LTR559_Check 0x87 data = 0x%x \n\r", data);
	
	BSP_I2C_write_reg(0x82,0x7F);
	BSP_I2C_write_reg(0x83,0x04);
	BSP_I2C_write_reg(0x84,0x00);
	BSP_I2C_write_reg(0x8F,0x00);
	BSP_I2C_write_reg(0x80,0x0D);
	BSP_I2C_write_reg(0x81,0x03);
	
	data = 0x258 &	0x00FF;
	BSP_I2C_write_reg(0x90,data);
	data = (0x258 &	0xFF00)>>8;		
	BSP_I2C_write_reg(0x91,data);
	data = 0x1F4 &	0x00FF;		
	BSP_I2C_write_reg(0x92,data);
	data = (0x1F4 &	0xFF00)>>8;	
	BSP_I2C_write_reg(0x93,data);
}	

#ifdef USE_MPU6050
bool API_MPU6050_init(void)
{
	if(API_MPU6050_check_id() != 0x68){
		return false;
	}
	// reset device
	API_MPU6050_reset();
	// wakeup
	API_MPU6050_wakeup();
	// config
	API_MPU6050_config();
	return true;
}
static void API_MPU6050_config(void){
	uint8_t check_reg = 0;

	BSP_I2C_write_reg(ICM20602_REG_ACCEL_CONFIG, ICM20602_ACCEL_SCALE_4G);
	HAL_Delay(5);
	BSP_I2C_read_reg(ICM20602_REG_ACCEL_CONFIG, &check_reg, 1);
	while(check_reg != (uint8_t)ICM20602_ACCEL_SCALE_4G) {
		BSP_I2C_write_reg(ICM20602_REG_ACCEL_CONFIG, ICM20602_ACCEL_SCALE_4G);
		HAL_Delay(5);
		BSP_I2C_read_reg(ICM20602_REG_ACCEL_CONFIG, &check_reg, 1);
		HAL_Delay(5);
	}

}
void API_MPU6050_reset(void){
	//reset chip
	BSP_I2C_write_reg(ICM20602_REG_PWR_MGMT_1, 0x80);
	HAL_Delay(5);
}

void API_MPU6050_wakeup(void){
	//wake up
	BSP_I2C_write_reg(ICM20602_REG_PWR_MGMT_1, 0x00);
	HAL_Delay(5);
	//enable all axis including acc and gyro
	BSP_I2C_write_reg(ICM20602_REG_PWR_MGMT_2, 0x00);
	HAL_Delay(5);
}

uint8_t API_MPU6050_check_id(void)
{
    uint8_t data;
    BSP_I2C_read_reg(MPU_REG_WHO_AM_I, &data, 1);
	//com_uart_port_wrtie_char(data);
    //nrf_delay_ms(10);
	printf("\n\rdata = 0x%x\n\r", data);
    return data;
}
#endif
// ENDLINE
