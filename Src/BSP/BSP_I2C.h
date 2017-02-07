/*
 * BSP_I2C.h
 *
 *  Created on: 2016Äê7ÔÂ20ÈÕ
 *      Author: feitao
 */

#ifndef BSP_I2C_H_
#define BSP_I2C_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdio.h"
#include "stdbool.h"

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_i2c.h"
extern I2C_HandleTypeDef hi2c1;
		
 void BSP_I2C_init(void);
 void BSP_I2C_ID_set(uint16_t addr,int i2cnum);
 bool BSP_I2C_read_reg(uint8_t regAddr, uint8_t* data, uint32_t length);
 bool BSP_I2C_write_reg(uint8_t addr, uint8_t setting);
 bool BSP_I2C_write_r8s16(uint8_t addr, uint16_t setting);
 bool BSP_I2C_write_r16s8(uint16_t addr, uint8_t setting);
 bool BSP_I2C_write_r16s16(uint16_t addr, uint16_t setting);
 bool BSP_I2C_write_r16s32(uint16_t addr, uint32_t setting);

 bool BSP_I2C_ReadMulti(uint16_t regAddr, uint8_t* data, uint32_t length);

 //bool BSP_I2C_read_reg8(uint16_t regAddr, uint8_t* data, uint32_t length);
 //bool BSP_I2C_read_reg16(uint16_t regAddr, uint8_t* data, uint32_t length);
 //bool BSP_I2C_read_reg32(uint16_t regAddr, uint8_t* data, uint32_t length);

#ifdef __cplusplus
 extern "C" }
#endif

#endif /* BSP_I2C_H_ */
