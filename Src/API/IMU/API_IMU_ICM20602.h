
#ifndef __API_IMU_ICM20602_H
#define __API_IMU_ICM20602_H

#include "IMU_common.h"
#define ICM20602_REG_PWR_MGMT_1         0x6B // Dec 107, R/W,  DEVICE_RESET SLEEP CYCLE - TEMP_DIS CLKSEL[2:0]  default is 0x01
#define ICM20602_REG_PWR_MGMT_2         0x6C // Dec 108, R/W,  LP_WAKE_CTRL[1:0] STBY_XA STBY_YA STBY_ZA STBY_XG STBY_YG STBY_ZG

#define ICM20602_REG_SELF_TEST_X_ACC        0x0D // Dec 13,  R/W,  XA_TEST[4-2] XG_TEST[4-0]
#define ICM20602_REG_SELF_TEST_Y_ACC        0x0E // Dec 14,  R/W,  YA_TEST[4-2] YG_TEST[4-0]
#define ICM20602_REG_SELF_TEST_Z_ACC        0x0F // Dec 15,  R/W,  ZA_TEST[4-2] ZG_TEST[4-0]

#define ICM20602_REG_XG_OFFS_USRH 			0x13
#define ICM20602_REG_XG_OFFS_USRL			0x14
#define ICM20602_REG_YG_OFFS_USRH 			0x15
#define ICM20602_REG_YG_OFFS_USRL			0x16
#define ICM20602_REG_ZG_OFFS_USRH 			0x17
#define ICM20602_REG_ZG_OFFS_USRL			0x18

#define ICM20602_REG_SMPLRT_DIV         0x19 // Dec 25,  R/W,  SMPLRT_DIV[7:0]
#define ICM20602_REG_CONFIG             0x1A // Dec 26,  R/W,  - -FIFO MODE[1:0] EXT_SYNC_SET[2:0] DLPF_CFG[2:0]
#define ICM20602_REG_GYRO_CONFIG        0x1B // Dec 27,  R/W,  - - - FS_SEL [1:0] - - -
#define ICM20602_REG_ACCEL_CONFIG       0x1C // Dec 28,  R/W,  XA_ST YA_ST ZA_ST AFS_SEL[1:0]
#define ICM20602_REG_ACCEL_CONFIG2      0x1D // Dec 29,  R/W,  DEC2_CFG ACCEL_FCHOICE_B A_DLPF_CFG
#define ICM20602_REG_LP_MODE_CFG        0x1E

#define ICM20602_REG_ACCEL_WOM_THR      0x1F
#define ICM20602_REG_FIFO_EN            0x23 // Dec 35,  R/W,  TEMP_FIFO_ENXG_FIFO_ENYG_FIFO_ENZG_FIFO_ENACCEL_FIFO_ENSLV2_FIFO_ENSLV1_FIFO_ENSLV0_FIFO_EN

#define ICM20602_REG_INT_PIN_CFG        0x37 // Dec 55,  R/W,  INT_LEVEL INT_OPEN LATCH_INT_ENINT_RD_CLEARFSYNC_INT_LEVELFSYNC_INT_ENI2C_BYPASS_EN-
#define ICM20602_REG_INT_ENABLE         0x38 // Dec 56,  R/W,  - MOT_EN -FIFO_OFLOW_ENI2C_MST_INT_EN - -DATA_RDY_EN

#define ICM20602_REG_INT_STATUS         0x3A // Dec 58,  R,    - MOT_INT -FIFO_OFLOW_INTI2C_MST_INT - -DATA_RDY_INT
#define ICM20602_REG_ACCEL_XOUT_H       0x3B // Dec 59,  R,    ACCEL_XOUT[15:8]
#define ICM20602_REG_ACCEL_XOUT_L       0x3C // Dec 60,  R,    ACCEL_XOUT[7:0]
#define ICM20602_REG_ACCEL_YOUT_H       0x3D // Dec 61,  R,    ACCEL_YOUT[15:8]
#define ICM20602_REG_ACCEL_YOUT_L       0x3E // Dec 62,  R,    ACCEL_YOUT[7:0]
#define ICM20602_REG_ACCEL_ZOUT_H       0x3F // Dec 63,  R,    ACCEL_ZOUT[15:8]
#define ICM20602_REG_ACCEL_ZOUT_L       0x40 // Dec 64,  R,    ACCEL_ZOUT[7:0]
#define ICM20602_REG_TEMP_OUT_H         0x41 // Dec 65,  R,    TEMP_OUT[15:8]
#define ICM20602_REG_TEMP_OUT_L         0x42 // Dec 66,  R,    TEMP_OUT[7:0]
#define ICM20602_REG_GYRO_XOUT_H        0x43 // Dec 67,  R,    GYRO_XOUT[15:8]
#define ICM20602_REG_GYRO_XOUT_L        0x44 // Dec 68,  R,    GYRO_XOUT[7:0]
#define ICM20602_REG_GYRO_YOUT_H        0x45 // Dec 69,  R,    GYRO_YOUT[15:8]
#define ICM20602_REG_GYRO_YOUT_L        0x46 // Dec 70,  R,    GYRO_YOUT[7:0]
#define ICM20602_REG_GYRO_ZOUT_H        0x47 // Dec 71,  R,    GYRO_ZOUT[15:8]
#define ICM20602_REG_GYRO_ZOUT_L        0x48 // Dec 72,  R,    GYRO_ZOUT[7:0]

#define ICM20602_REG_SELF_TEST_X_GYRO   0x50
#define ICM20602_REG_SELF_TEST_Y_GYRO  	0x51
#define ICM20602_REG_SELF_TEST_Z_GYRO	0x52
#define ICM20602_REG_SIGNAL_PATH_RESET	0x68
#define ICM20602_REG_ACCEL_INTEL_CTRL	0x69
#define ICM20602_REG_USER_CTRL       	0x6A

#define ICM20602_REG_FIFO_COUNTH		0x72
#define ICM20602_REG_FIFO_COUNTL		0x73
#define ICM20602_REG_FIFO_R_W			0x74

// OFFSET which can be stored
#define ICM20602_REG_XA_OFFSET_H		0x77
#define ICM20602_REG_XA_OFFSET_L		0x78
#define ICM20602_REG_YA_OFFSET_H  		0x7A
#define ICM20602_REG_YA_OFFSET_L		0x7B
#define ICM20602_REG_ZA_OFFSET_H		0x7D
#define ICM20602_REG_ZA_OFFSET_L		0x7E


#define MPU_REG_WHO_AM_I           0x75 // Dec 117, R,    - WHO_AM_I[6:1]    for icm20602, default is 0x12

typedef struct
{ 
  uint8_t                reg;
  uint16_t                setting;      
} I2C_DATATypeDef;

bool API_ICM20602_init(void);

void API_ICM20602_read_reg(void);
void API_ICM20602_read_imu_data(accel_values_t* acc_data, gyro_values_t* gyro_data);

// private
void API_ICM20602_reset(void);
void API_ICM20602_wakeup(void);
uint8_t API_ICM20602_check_id(void);
bool API_65132_init(int panelnum);
bool API_5616_init(void);
bool API_akm9916_init(void);
void API_7737_Check(void);
bool API_8870_init(void);
void API_PNI_Check(void);
void API_LTR559_Check(void);

#endif
// ENDLINE
