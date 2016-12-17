#include "agc.h"
#include "uart.h"

/* I2C Master Configuration Parameter */
const eUSCI_I2C_MasterConfig i2cConfig = {
		EUSCI_B_I2C_CLOCKSOURCE_SMCLK,			//SMCLK Clock Source
		48000000,								//SMCLK = 3MHz
		EUSCI_B_I2C_SET_DATA_RATE_400KBPS,		//Desired I2C Clock of 400khz
		0,										//No byte counter threshold
		EUSCI_B_I2C_NO_AUTO_STOP				//No Autostop
		//EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD				//No Autostop
};

void i2c_init(){
	//Select port 6 for I2C - setting pints 4 / 5 to input primary module function
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
			GPIO_PIN4 | GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_I2C_initMaster(EUSCI_B1_BASE, &i2cConfig);
	MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, AGC_LIN_MAG_ADDR); //XM ADDRESS

	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
	MAP_I2C_enableModule(EUSCI_B1_BASE);
	MAP_I2C_clearInterruptFlag(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 + EUSCI_B_I2C_RECEIVE_INTERRUPT0 + EUSCI_B_I2C_NAK_INTERRUPT);

	//CHECK READING AGC REGISTERS OK
//#define LINMAG_WHO_AM_I 0b01001001
//#define GYRO_WHO_AM_I 	0b11010100
	uint8_t who = agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_WHO_AM_I_XM);
	if (who != LINMAG_WHO_AM_I){
		ser_printf("FAILURE STARTING AGC: %x", who);
	}

	who = agc_read_register(AGC_GYRO_ADDR, AGC_GYRO_REG_WHO_AM_I_G);
	if (who != GYRO_WHO_AM_I){
		ser_printf("FAILURE STARTING AGC: %x", who);
	}

	//init 9dof sane values:
	//Linear accelerometer/magnetometer:
	agc_write_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_CTRL_REG1_XM, 0x67);
	agc_write_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_CTRL_REG5_XM, 0b11110000);
	agc_write_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_CTRL_REG7_XM, 0b00000000);

	//init Gyro module:
	agc_write_register(AGC_GYRO_ADDR, AGC_GYRO_REG_CTRL_REG1_G, 0x0F);
}

uint8_t agc_read_register(Agc_Address_t address, uint8_t reg){
	volatile int x;
	MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, address); //XM ADDRESS
	uint8_t result = -1;

	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
	MAP_I2C_masterSendSingleByte(EUSCI_B1_BASE, reg);
	while (MAP_I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);

	for (x=0; x<100; x++);
	result = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
	while (MAP_I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);
	for (x=0; x<100; x++);
	return result;
}

void agc_write_register(Agc_Address_t address, uint8_t reg, uint8_t data){
	MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, address); //XM ADDRESS
	volatile int x;

	MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
	MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE, reg);
	while (MAP_I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);

	for (x=0; x<200; x++);
	MAP_I2C_masterSendMultiByteFinish(EUSCI_B1_BASE, data);
	while (MAP_I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);

	for (x=0; x<200; x++);
}

int16_t agc_read_sensor_axis(Agc_Sensor_t sensor, Agc_Axis_t axis){
	int16_t result = -1;
	switch (sensor) {
		case AGC_LINEAR_ACCELEROMETER_SENSOR:
			switch(axis){
				case AGC_X_AXIS:
					result = agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_X_H_A) << 8;
					result |= agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_X_L_A);
					break;
				case AGC_Y_AXIS:
					result = agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_Y_H_A) << 8;
					result |= agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_Y_L_A);
					break;
				case AGC_Z_AXIS:
					result = agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_Z_H_A) << 8;
					result |= agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_Z_L_A);
					break;
			}
			break;

		case AGC_MAGNETOMETER_SENSOR:
			switch(axis){
				case AGC_X_AXIS:
					result = agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_X_H_M) << 8;
					result |= agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_X_L_M);
					break;
				case AGC_Y_AXIS:
					result = agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_Y_H_M) << 8;
					result |= agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_Y_L_M);
					break;
				case AGC_Z_AXIS:
					result = agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_Z_H_M) << 8;
					result |= agc_read_register(AGC_LIN_MAG_ADDR, AGC_LINMAG_REG_OUT_Z_L_M);
					break;
			}
			break;
		case AGC_GYROSCOPE_SENSOR:
			switch(axis){
				case AGC_X_AXIS:
					result = agc_read_register(AGC_GYRO_ADDR, AGC_GYRO_REG_OUT_X_H_G) << 8;
					result |= agc_read_register(AGC_GYRO_ADDR, AGC_GYRO_REG_OUT_X_L_G);
					break;
				case AGC_Y_AXIS:
					result = agc_read_register(AGC_GYRO_ADDR, AGC_GYRO_REG_OUT_Y_H_G) << 8;
					result |= agc_read_register(AGC_GYRO_ADDR, AGC_GYRO_REG_OUT_Y_L_G);
					break;
				case AGC_Z_AXIS:
					result = agc_read_register(AGC_GYRO_ADDR, AGC_GYRO_REG_OUT_Z_H_G) << 8;
					result |= agc_read_register(AGC_GYRO_ADDR, AGC_GYRO_REG_OUT_Z_L_G);
					break;
			}
			break;
	}
	return result;
}
