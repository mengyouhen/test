/*
 * IMU_common.h
 *
 *  Created on: 2016Äê7ÔÂ22ÈÕ
 *      Author: feitao
 */

#ifndef API_IMU_IMU_COMMON_H_
#define API_IMU_IMU_COMMON_H_

/**@brief Structure to hold acceleromter values.
 * Sequence of z, y, and x is important to correspond with
 * the sequence of which z, y, and x data are read from the sensor.
 * All values are unsigned 16 bit integers
*/
typedef struct
{
    int16_t z;
    int16_t y;
    int16_t x;
}accel_values_t;

/**@brief Structure to hold gyroscope values.
 * Sequence of z, y, and x is important to correspond with
 * the sequence of which z, y, and x data are read from the sensor.
 * All values are unsigned 16 bit integers
*/
typedef struct
{
    int16_t z;
    int16_t y;
    int16_t x;
}gyro_values_t;




#endif /* API_IMU_IMU_COMMON_H_ */
