/*
 * BSP_SPI.c
 *
 *  Created on: 2016Äê7ÔÂ20ÈÕ
 *      Author: feitao
 */

//TODO: need to add DMA support for driver
#include "BSP_SPI.h"

//User lower driver layer
//#include "spi.h"   // including spi instance/handler
//#include "gpio.h"

//User Rtos layer
#include "FreeRTOS.h"
//#include "task.h"
#include "cmsis_os.h"

#define ICM20602_OPEN HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET)
#define ICM20602_CLOSE HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET)
#define DUMMY_BYTE   0XA5


void BSP_SPI_init(void){

}

bool BSP_SPI_read_reg(uint8_t regAddr, uint8_t* data){
	// chip select
	ICM20602_OPEN;
	//vTaskDelay(1);
	uint8_t temp_tx[2] = {(regAddr)|0x80, DUMMY_BYTE};
	uint8_t temp_rx[2];
	// get data
	if(HAL_SPI_TransmitReceive(&hspi1, temp_tx, temp_rx, 2, 200) != HAL_OK) {
		ICM20602_CLOSE;
		return false;
	}

	*data = temp_rx[1];
	ICM20602_CLOSE;
	return true;
}


bool BSP_SPI_write_reg(uint8_t regAddr, uint8_t setting){
	// chip select
	ICM20602_OPEN;

	uint8_t temp_tx[2] = {regAddr, setting};
	uint8_t temp_rx[2];

	if(HAL_SPI_TransmitReceive(&hspi1, temp_tx, temp_rx, 2, 2000) != HAL_OK) {
		ICM20602_CLOSE;
		return false;
	}

	ICM20602_CLOSE;
	return true;
}

bool BSP_SPI_read_regs(uint8_t regAddr, uint8_t length, uint8_t* data){
	// chip select
	ICM20602_OPEN;

	uint8_t temp_tx[length + 1];
	temp_tx[0] = regAddr|0x80;

	for(uint8_t i = 1 ; i < length + 1; i++){
		temp_tx[i] = DUMMY_BYTE;
	}

	uint8_t temp_rx[length + 1];
	if(HAL_SPI_TransmitReceive(&hspi1, temp_tx, temp_rx, length + 1, 2000) != HAL_OK) {
		ICM20602_CLOSE;
		return false;
	}

	for(uint8_t i = 1 ; i < length + 1; i++){
		*data = temp_rx[i];
		data++;
	}

	ICM20602_CLOSE;
	return true;
}

//ENDLINE
