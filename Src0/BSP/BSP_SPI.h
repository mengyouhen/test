/*
 * BSP_SPI.h
 *
 *  Created on: 2016Äê7ÔÂ20ÈÕ
 *      Author: feitao
 */

#ifndef BSP_SPI_H_
#define BSP_SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdio.h"
#include "stdbool.h"

#include "stm32f0xx_hal.h"


 void BSP_SPI_init(void);

 bool BSP_SPI_read_reg(uint8_t regAddr, uint8_t* data);

 bool BSP_SPI_write_reg(uint8_t addr, uint8_t setting);

 bool BSP_SPI_read_regs(uint8_t addr, uint8_t length, uint8_t* data);
#ifdef __cplusplus
 extern "C" }
#endif

#endif /* BSP_SPI_H_ */
