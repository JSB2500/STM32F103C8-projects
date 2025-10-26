/*
 * pcf8574.h
 *
 *  Created on: Dec 30, 2014
 *      Author: peter
 */

#ifndef INC_PCF8574_H_
#define INC_PCF8574_H_

#include "stm32f1xx_hal.h"

/**
 * Provides possible return values for the functions
 */
typedef enum{
	PCF8574_OK,		/**< Everything went OK */
	PCF8574_ERROR	/**< An error occured */
} PCF8574_RESULT;

/**
 * PCF8574 handle structure which wraps all the necessary variables together in
 * order to simplify the communication with the chip
 */
typedef struct{
	uint8_t				PCF_I2C_ADDRESS;	/**< address of the chip you want to communicate with */
	uint32_t			PCF_I2C_TIMEOUT;	/**< timeout value for the communication in milliseconds */
	I2C_HandleTypeDef 	*pI2C;				/**< Pointer to I2C_HandleTypeDef structure */
	void				(*errorCallback)(PCF8574_RESULT);
} PCF8574_HandleTypeDef;

/** @var PCF8574_Type0Pins[8] - characterization of pins for hardware of type 0
 */
extern uint32_t PCF8574_Type0Pins[];

/**
 * Writes a given value to the port of PCF8574
 * @param	handle - a pointer to the PCF8574 handle
 * @param	val - a value to be written to the port
 * @return	whether the function was successful or not
 */
PCF8574_RESULT PCF8574_Write(PCF8574_HandleTypeDef* handle, uint8_t val);

/**
 * Reads the current state of the port of PCF8574
 * @param	handle - a pointer to the PCF8574 handle
 * @param	val - a pointer to the variable that will be assigned a value from the chip
 * @return	whether the function was successful or not
 */
PCF8574_RESULT PCF8574_Read(PCF8574_HandleTypeDef* handle, uint8_t* val);

#endif /* INC_PCF8574_H_ */
