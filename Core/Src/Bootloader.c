/*
 * Bootloader.c
 *
 *  Created on: Sep 20, 2023
 *      Author: E0162112
 */

#include "stdint.h"
#include "Bootloader.h"
#include "main.h"
#include "Bit_Math.h"


Bootloader_Params_t BootLoader ;

void Bootloader_Init(void)
{
	/*FPEC unlock sequence */
	FPEC_REGISTER->FLASH_KEYR = KEY1 ;
	FPEC_REGISTER->FLASH_KEYR = KEY2 ;

}

void Bootloader_main(void)
{
	uint8_t UART_RxData;
	switch(BootLoader.state)
	{
	case BL_NOT_INITIALIZED :
		/*check if bootloader msg is being received*/
		if (BootLoader.timeoutCount < 255)
		{
			UART_1_RECEIVE( &UART_RxData, 10);
			if (UART_RxData == BOOTLOADER_START_MSG)
			{
				Bootloader_EraseData();
				BootLoader.state == BL_INITIALIZED;
			}
			else if (UART_RxData == APPLICATION_START_MSG)
			{
				BootLoader.state == BL_DONE;
			}
			else
			{
				BootLoader.timeoutCount++;
			}
		}
		else
		{
			BootLoader.state == BL_DONE;
		}
		break;

	case BL_INITIALIZED:
		BootLoader.length = 0;
		BootLoader.offsetAddress = 0;
		BootLoader.address &= 0xffff0000;
		BootLoader.timeoutCount = 0;
		BootLoader.sum = 0;
		BootLoader.state = BL_RECEIVE_LENGTH_BYTE;
		break;

	case BL_RECEIVE_LENGTH_BYTE:
		/*Receive length byte*/
		UART_1_RECEIVE( &UART_RxData, 10);
		BootLoader.length = UART_RxData;
		BootLoader.sum += BootLoader.length;
		BootLoader.state = BL_RECEIVE_ADDRESS_BYTES;
		break;

	case BL_RECEIVE_ADDRESS_BYTES:
		/*Receive 2 address bytes*/
		UART_1_RECEIVE( &UART_RxData, 10); //receive higher byte of address
		BootLoader.sum += UART_RxData;
		BootLoader.offsetAddress = (uint16_t)(UART_RxData << 8);

		UART_1_RECEIVE( &UART_RxData, 10);	//receive lower byte of address
		BootLoader.sum += UART_RxData;
		BootLoader.offsetAddress |= (uint16_t)UART_RxData;

		BootLoader.state = BL_RECEIVE_TYPE_BYTE;
		break;

	case BL_RECEIVE_TYPE_BYTE:
		/*receive type byte*/
		UART_1_RECEIVE( &UART_RxData, 10); //receive type byte
		BootLoader.type = UART_RxData;
		BootLoader.sum += BootLoader.type;

		if (BootLoader.type == BL_TYPE_ADDRESS_HIGH)
		{
			BootLoader.state = BL_RECEIVE_BASE_ADDRESS_BYTES;
		}
		else if (BootLoader.type == BL_TYPE_DATA)
		{
			BootLoader.state = BL_RECEIVE_DATA_BYTES;
		}
		else if (BootLoader.type == BL_TYPE_EOF)
		{
			BootLoader.state = BL_DONE;
		}
		break;

	case BL_RECEIVE_BASE_ADDRESS_BYTES:
		if(UART_1_RECEIVE( &UART_RxData, 10))
		{
			BootLoader.sum += UART_RxData;
			BootLoader.baseAddress = (uint16_t)(UART_RxData << 8);	//receive type byte
		}

		if(UART_1_RECEIVE( &UART_RxData, 10))
		{
			BootLoader.sum += UART_RxData;
			BootLoader.baseAddress |= (uint16_t)(UART_RxData);		//receive type byte
		}

		BootLoader.address = (uint32_t)((uint32_t)(BootLoader.baseAddress << 16) | (uint32_t)(BootLoader.offsetAddress));

		BootLoader.state == BL_RECEIVE_CHECKSUM_BYTE;
		break;

	case BL_RECEIVE_DATA_BYTES:
		/*Loop for the LENGTH of line and flash*/

		for (uint8_t i = 0; i < BootLoader.length; i+=2)
		{
			uint8_t temp1 , temp2;
			uint16_t data;

			UART_1_RECEIVE( &UART_RxData, 10);
			temp1 = UART_RxData;
			BootLoader.sum += temp1;

			UART_1_RECEIVE( &UART_RxData, 10);
			temp2 = UART_RxData;
			BootLoader.sum += temp2;

			data = (uint16_t)((uint16_t)(temp1<<8) | (uint16_t)(temp2));

			Bootloader_WriteData(data , (BootLoader.address + i));
		}

		BootLoader.state = BL_RECEIVE_CHECKSUM_BYTE;
		break;

	case BL_RECEIVE_CHECKSUM_BYTE:
		/*Receive checksum byte and calculate msg checksum then compare the 2 numbers*/
		UART_1_RECEIVE( &UART_RxData, 10);
		BootLoader.sum = ~BootLoader.sum;
		BootLoader.sum +=1;
		if (UART_RxData == BootLoader.sum)
		{
			BootLoader.state = BL_INITIALIZED;
			UART_1_TRANSMIT("ok");
		}

		break;

	case BL_ERROR:

		NVIC_SystemReset();

	case BL_DONE:
		/*Jump to application address*/
		break;

	}

}

void Bootloader_EraseData(void)
{
	/*Erase all flash memory from 0x8001000 to end of flash*/

	SET_BIT(FPEC_REGISTER->FLASH_CR , OFFSET_FLASH_CR_PER); // enable page erase

	for (uint32_t i = 0 ; i < APPLICATION_END_ADDRESS; i+=1024)
	{
		while(GET_BIT(FPEC_REGISTER->FLASH_SR,OFFSET_FLASH_SR_BSY));	//Wait for busy flag to clear
		FPEC_REGISTER->FLASH_AR = APPLICATION_START_ADDRESS + i;		//load memory address into FLASH_AR register
		SET_BIT(FPEC_REGISTER->FLASH_CR , OFFSET_FLASH_CR_STRT);		//Start erasing

	}
	CLR_BIT(FPEC_REGISTER->FLASH_CR , OFFSET_FLASH_CR_PER); 			// disable page erase
}

void Bootloader_WriteData(uint16_t Data, uint32_t Address)
{

	SET_BIT(FPEC_REGISTER->FLASH_CR , OFFSET_FLASH_CR_PG); 				// enable page program
	*((volatile uint16_t*)Address) = Data;
	while(GET_BIT(FPEC_REGISTER->FLASH_SR,OFFSET_FLASH_SR_BSY));		// Wait for busy flag to clear
	SET_BIT(FPEC_REGISTER->FLASH_SR, OFFSET_FLASH_SR_EOP);
	CLR_BIT(FPEC_REGISTER->FLASH_CR , OFFSET_FLASH_CR_PG);
}

uint8_t LineCheckSum_Calculate(uint8_t arr[] ,uint8_t length)
{
	uint16_t retval = 0;
	for (int i =0; i < length; i++)
	{
		retval += arr[i];
	}
	retval = retval & 0xff;
	retval = ~retval;
	retval +=1;
	return (uint8_t)retval;

}
