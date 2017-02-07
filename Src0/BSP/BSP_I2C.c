/*
 * BSP_I2C.c
 *
 *  Created on: 2016Äê7ÔÂ20ÈÕ
 *      Author: feitao
 */

//TODO: need to add DMA support for driver
#include "BSP_I2C.h"
#include "main.h"
//User lower driver layer
//#include "i2c.h"   // including spi instance/handler
//#include "gpio.h"

//#define SLAVE_ADDRESS     0x50//(0x8C)/*7737*///(0x1F<<1)/*8870*///(0x0c<<1)/*AKM*///(0x0F<<1)/*8870 demo*///(0x1B<<1)//(0x3E<<1)/*65132*///(0x68<<1)/*20602*/
uint16_t SLAVE_ADDRESS = 0x00;
I2C_HandleTypeDef *ghi2c;
void BSP_I2C_init(void){

}
void BSP_I2C_ID_set(uint16_t addr,int i2cnum)
{
	SLAVE_ADDRESS = (addr<<1);

	switch (i2cnum){
	case 1:
		ghi2c = &hi2c1;
		break;
	/*
	case 2:
		ghi2c = &hi2c2;
		break;
	case 3:
		ghi2c = &hi2c3;
		break;*/
	default:
	        break;
	}
	printf("\n\rI2CID_SET: SLAVE_ADDRESS = 0x%x i2cnum=%d\n\r", SLAVE_ADDRESS,i2cnum);
}
bool BSP_I2C_read_reg(uint8_t regAddr, uint8_t* data, uint32_t length){
	// get data
	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&regAddr, 1, 1000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	if(HAL_I2C_Master_Receive(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t *)data, length, 1000) != HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}

	return true;
}


bool BSP_I2C_write_reg(uint8_t regAddr, uint8_t setting){
	uint8_t temp_tx[2] = {regAddr, setting};

	/* The board sends the message and expects to receive it back */
	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 2, 1000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	return true;
}

bool BSP_I2C_write_r16s8(uint16_t addr, uint8_t setting){
	uint8_t temp_tx[3] = {0x00, 0x00, 0x00};

	temp_tx[0] = (addr & 0xFF00)>>8;
	temp_tx[1] = addr & 0x00FF;
	temp_tx[2] = setting;

	/* The board sends the message and expects to receive it back */
	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 3, 10000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	return true;
}
bool BSP_I2C_write_r8s16(uint8_t addr, uint16_t setting){
	uint8_t temp_tx[3] = {0x00, 0x00, 0x00};
	temp_tx[0] = addr;
	//temp_tx[1] = setting & 0x00FF;
	//temp_tx[2] = (setting & 0xFF00)>>8;
	temp_tx[1] = (setting & 0xFF00)>>8;
	temp_tx[2] = setting & 0x00FF;
	/* The board sends the message and expects to receive it back */
	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 3, 10000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	return true;
}
bool BSP_I2C_write_r16s16(uint16_t addr, uint16_t setting)
{
	uint8_t temp_tx[4] = {0x00, 0x00, 0x00, 0x00};

	temp_tx[0] = (addr & 0xFF00)>>8;
	temp_tx[1] = addr & 0x00FF;
	temp_tx[2] = setting & 0x00FF;
	temp_tx[3] = (setting & 0xFF00)>>8;
	//temp_tx[2] = (setting & 0xFF00)>>8;
	//temp_tx[3] = setting & 0x00FF;

	/* The board sends the message and expects to receive it back */
	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 4, 10000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	return true;
}


bool BSP_I2C_write_r16s32(uint16_t addr, uint32_t setting)
{
	uint8_t temp_tx[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	temp_tx[0] = (addr & 0xFF00)>>8;
	temp_tx[1] = addr & 0x00FF;
	temp_tx[2] = setting & 0x000000FF;
	temp_tx[3] = (setting & 0xFF00)>>8;
	temp_tx[4] = (setting & 0xFF0000)>>16;
	temp_tx[5] = (setting & 0xFF000000)>>24;
	//temp_tx[2] = (setting & 0xFF000000)>>24;
	//temp_tx[3] = (setting & 0xFF0000)>>16;
	//temp_tx[4] = (setting & 0xFF00)>>8;
	//temp_tx[5] = setting & 0x000000FF;

	//for(uint8_t i = 0 ; i < 6; i++){
		//printf("\n\rBSP_I2C_write_reg32 temp_tx[%d]= 0x%x \n\r", i, temp_tx[i]);
	//}

	/* The board sends the message and expects to receive it back */
	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 6, 10000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	return true;
}
bool BSP_I2C_ReadMulti(uint16_t regAddr, uint8_t* data, uint32_t length)
{
	uint8_t temp_tx[2] = {0x00, 0x00};
	temp_tx[0] = (regAddr & 0xFF00)>>8;
	temp_tx[1] = regAddr & 0x00FF;

	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 2, 10000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	if(HAL_I2C_Master_Receive(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t *)data, length, 10000) != HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}

	return true;
}
#if 0
bool BSP_I2C_read_reg8(uint16_t regAddr, uint8_t* data, uint32_t length){
	uint8_t temp_tx[2] = {0x00, 0x00};
	//temp_tx[0] = regAddr & 0x00FF;
	//temp_tx[1] = (regAddr & 0xFF00)>>8;
	temp_tx[0] = (regAddr & 0xFF00)>>8;
	temp_tx[1] = regAddr & 0x00FF;
	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 2, 10000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	if(HAL_I2C_Master_Receive(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t *)data, length, 10000) != HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}

	return true;
}
bool BSP_I2C_read_reg16(uint16_t regAddr, uint8_t* data, uint32_t length)
{
	uint8_t temp_tx[2] = {0x00, 0x00};
	temp_tx[0] = (regAddr & 0xFF00)>>8;
	temp_tx[1] = regAddr & 0x00FF;

	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 2, 10000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	if(HAL_I2C_Master_Receive(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t *)data, length, 10000) != HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}

	return true;
}

bool BSP_I2C_read_reg32(uint16_t regAddr, uint8_t* data, uint32_t length)
{
	uint8_t temp_tx[2] = {0x00, 0x00};
	temp_tx[0] = (regAddr & 0xFF00)>>8;
	temp_tx[1] = regAddr & 0x00FF;

	if(HAL_I2C_Master_Transmit(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t*)&temp_tx, 2, 10000)!= HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}
	if(HAL_I2C_Master_Receive(ghi2c, (uint16_t)SLAVE_ADDRESS, (uint8_t *)data, length, 10000) != HAL_OK)
	{
		/* Error_Handler() function is called when Timeout error occurs.
		   When Acknowledge failure occurs (Slave don't acknowledge it's address)
		   Master restarts communication */
		  Error_Handler();
	}

	return true;
}

bool BSP_I2C_read_regs(uint8_t regAddr, uint8_t length, uint8_t* data){
	uint8_t temp_tx[length + 1];
	temp_tx[0] = regAddr|0x80;

	for(uint8_t i = 1 ; i < length + 1; i++){
		temp_tx[i] = DUMMY_BYTE;
	}

	uint8_t temp_rx[length + 1];
	if(HAL_SPI_TransmitReceive(&hspi2, temp_tx, temp_rx, length + 1, 200) != HAL_OK) {
		ICM20602_CLOSE;
		return false;
	}

	for(uint8_t i = 1 ; i < length + 1; i++){
		*data = temp_rx[i];
		data++;
	}

	return true;
}
#endif
//ENDLINE
