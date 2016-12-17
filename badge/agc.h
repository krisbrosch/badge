#ifndef AGC_H_
#define AGC_H_
#include <stdlib.h>
#include "driverlib.h"

#define LINMAG_WHO_AM_I 0b01001001
#define GYRO_WHO_AM_I 	0b11010100

typedef enum {
	AGC_LIN_MAG_ADDR  	= 0b0011101, 	//0x3b >> 1
	AGC_GYRO_ADDR		= 0b1101011 	//0xD6 >> 1
} Agc_Address_t;

//#define AGC_LIN_MAG_ADDR  	0b0011101 	//0x3b >> 1
//#define AGC_GYRO_ADDR		0b1101011 	//0xD6 >> 1

//SUB ADDRESSES:
//XM/LINMAG:
typedef enum {
	AGC_LINMAG_REG_OUT_TEMP_L_XM 		= 0x05,
	AGC_LINMAG_REG_OUT_TEMP_H_XM       = 0x06,
	AGC_LINMAG_REG_STATUS_REG_M        = 0x07,
	AGC_LINMAG_REG_OUT_X_L_M           = 0x08,
	AGC_LINMAG_REG_OUT_X_H_M           = 0x09,
	AGC_LINMAG_REG_OUT_Y_L_M           = 0x0A,
	AGC_LINMAG_REG_OUT_Y_H_M           = 0x0B,
	AGC_LINMAG_REG_OUT_Z_L_M           = 0x0C,
	AGC_LINMAG_REG_OUT_Z_H_M           = 0x0D,
	AGC_LINMAG_REG_WHO_AM_I_XM         = 0x0F,
	AGC_LINMAG_REG_INT_CTRL_REG_M      = 0x12,
	AGC_LINMAG_REG_INT_SRC_REG_M       = 0x13,
	AGC_LINMAG_REG_CTRL_REG1_XM        = 0x20,
	AGC_LINMAG_REG_CTRL_REG2_XM        = 0x21,
	AGC_LINMAG_REG_CTRL_REG5_XM        = 0x24,
	AGC_LINMAG_REG_CTRL_REG6_XM        = 0x25,
	AGC_LINMAG_REG_CTRL_REG7_XM        = 0x26,
	AGC_LINMAG_REG_OUT_X_L_A           = 0x28,
	AGC_LINMAG_REG_OUT_X_H_A           = 0x29,
	AGC_LINMAG_REG_OUT_Y_L_A           = 0x2A,
	AGC_LINMAG_REG_OUT_Y_H_A           = 0x2B,
	AGC_LINMAG_REG_OUT_Z_L_A           = 0x2C,
	AGC_LINMAG_REG_OUT_Z_H_A           = 0x2D
} Agc_LinMag_Registers_t;

typedef enum {
	AGC_LINMAG_LINRANGE_2G		= (0b000 << 3),
	AGC_LINMAG_LINRANGE_4G		= (0b001 << 3),
	AGC_LINMAG_LINRANGE_6G		= (0b010 << 3),
	AGC_LINMAG_LINRANGE_8G		= (0b011 << 3),
	AGC_LINMAG_LINRANGE_16G		= (0b100 << 3)
} Agc_LinMag_LinRange_t;

typedef enum {
	AGC_LINMAG_LINDATARATE_POWERDOWN      = (0b0000 << 4),
	AGC_LINMAG_LINDATARATE_3_125HZ        = (0b0001 << 4),
	AGC_LINMAG_LINDATARATE_6_25HZ         = (0b0010 << 4),
	AGC_LINMAG_LINDATARATE_12_5HZ         = (0b0011 << 4),
	AGC_LINMAG_LINDATARATE_25HZ           = (0b0100 << 4),
	AGC_LINMAG_LINDATARATE_50HZ           = (0b0101 << 4),
	AGC_LINMAG_LINDATARATE_100HZ          = (0b0110 << 4),
	AGC_LINMAG_LINDATARATE_200HZ          = (0b0111 << 4),
	AGC_LINMAG_LINDATARATE_400HZ          = (0b1000 << 4),
	AGC_LINMAG_LINDATARATE_800HZ          = (0b1001 << 4),
	AGC_LINMAG_LINDATARATE_1600HZ         = (0b1010 << 4)
} Agc_LinMag_LinDataRate_t;

typedef enum {
	AGC_LINMAG_MAGGAIN_2GAUSS			= (0b00 << 5),  // +/- 2 gauss
	AGC_LINMAG_MAGGAIN_4GAUSS			= (0b01 << 5),  // +/- 4 gauss
	AGC_LINMAG_MAGGAIN_8GAUSS			= (0b10 << 5),  // +/- 8 gauss
	AGC_LINMAG_MAGGAIN_12GAUSS			= (0b11 << 5)   // +/- 12 gauss
} Agc_LinMag_MagGain_t;

typedef enum {
	AGC_LINMAG_MAGDATARATE_3_125HZ			= (0b000 << 2),
	AGC_LINMAG_MAGDATARATE_6_25HZ			= (0b001 << 2),
	AGC_LINMAG_MAGDATARATE_12_5HZ			= (0b010 << 2),
	AGC_LINMAG_MAGDATARATE_25HZ				= (0b011 << 2),
	AGC_LINMAG_MAGDATARATE_50HZ				= (0b100 << 2),
	AGC_LINMAG_MAGDATARATE_100HZ			= (0b101 << 2)
} Agc_LinMag_MagDataRate_t;

//GYRO
typedef enum {
	AGC_GYRO_REG_WHO_AM_I_G		= 0x0F,
	AGC_GYRO_REG_CTRL_REG1_G	= 0x20,
	AGC_GYRO_REG_CTRL_REG3_G	= 0x22,
	AGC_GYRO_REG_CTRL_REG4_G	= 0x23,
	AGC_GYRO_REG_OUT_X_L_G		= 0x28,
	AGC_GYRO_REG_OUT_X_H_G		= 0x29,
	AGC_GYRO_REG_OUT_Y_L_G		= 0x2A,
	AGC_GYRO_REG_OUT_Y_H_G		= 0x2B,
	AGC_GYRO_REG_OUT_Z_L_G		= 0x2C,
	AGC_GYRO_REG_OUT_Z_H_G		= 0x2D
} Agc_Gyro_Registers_t;

typedef enum {
	AGC_GYRO_GYROSCALE_245DPS	= (0b00 << 4),  // +/- 245 degrees per second rotation
	AGC_GYRO_GYROSCALE_500DPS	= (0b01 << 4),  // +/- 500 degrees per second rotation
	AGC_GYRO_GYROSCALE_2000DPS	= (0b10 << 4)   // +/- 2000 degrees per second rotation
} Agc_Gyro_GyroScale_t;

typedef struct vector_s {
	float x;
	float y;
	float z;
} Agc_Vector_t;

typedef enum {
	AGC_LINEAR_ACCELEROMETER_SENSOR = 1,
	AGC_MAGNETOMETER_SENSOR = 2,
	AGC_GYROSCOPE_SENSOR = 3
} Agc_Sensor_t;

typedef enum {
	//ACTUAL 9DOF X positive Toward WIFI
	//ACTUAL 9DOF Y positive Toward JTAG
	//ACTUAL 9DOF Z positive Into board

	AGC_X_AXIS = 1,
	AGC_Y_AXIS = 2,
	AGC_Z_AXIS = 3
} Agc_Axis_t;

void i2c_init();
uint8_t agc_read_register(Agc_Address_t address, uint8_t reg);
void agc_write_register(Agc_Address_t address, uint8_t reg, uint8_t data);
int16_t agc_read_sensor_axis(Agc_Sensor_t sensor, Agc_Axis_t axis);

#endif  /*AGC_H_*/
