/*
 * Bootloader.h
 *
 *  Created on: Sep 20, 2023
 *      Author: E0162112
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

#include "FPEC.h"

#define APPLICATION_START_ADDRESS		0x08002000
#define APPLICATION_END_ADDRESS			0x08007C00
#define APPLICATION_LENGTH				64

#define BOOTLOADER_START_MSG			0x1f
#define	APPLICATION_START_MSG			0xf1


typedef enum
{
	BL_NOT_INITIALIZED,
	BL_INITIALIZED,
	BL_RECEIVE_LENGTH_BYTE,
	BL_RECEIVE_ADDRESS_BYTES,
	BL_RECEIVE_TYPE_BYTE,
	BL_RECEIVE_DATA_BYTES,
	BL_RECEIVE_BASE_ADDRESS_BYTES,
	BL_RECEIVE_CHECKSUM_BYTE,
	BL_ERROR,
	BL_DONE,
}Bootloader_State_t;

typedef enum
{
	BL_TYPE_ADDRESS_HIGH = 0x04,
	BL_TYPE_DATA			= 0x00,
	BL_TYPE_EOF			= 0x01,
	BL_TYPE_DEFAULT		= 0xff,
}BootLoader_types;

typedef struct
{
	Bootloader_State_t state ;
	uint8_t timeoutCount ;
	uint8_t length ;
	BootLoader_types type ;
	uint16_t sum ;
	uint16_t baseAddress;
	uint16_t offsetAddress ;
	uint32_t address;


}Bootloader_Params_t;

void Bootloader_main(void);
void Bootloader_EraseData(void);
void Bootloader_WriteData(uint16_t Data, uint32_t Address);
uint8_t LineCheckSum_Calculate(uint8_t arr[] ,uint8_t length);

#endif /* INC_BOOTLOADER_H_ */
