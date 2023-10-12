/*
 * FPEC.h
 *
 *  Created on: Sep 20, 2023
 *      Author: E0162112
 */

#ifndef INC_FPEC_H_
#define INC_FPEC_H_

#include "stdint.h"
typedef struct
{
	volatile uint32_t FLASH_ACR;
	volatile uint32_t FLASH_KEYR;
	volatile uint32_t FLASH_OPTKEYR;
	volatile uint32_t FLASH_SR;
	volatile uint32_t FLASH_CR;
	volatile uint32_t FLASH_AR;
	volatile uint32_t reserved;
	volatile uint32_t FLASH_OBR;
	volatile uint32_t FLASH_WRPR;
}FPEC_Registers_t;

typedef enum
{
	OFFSET_FLASH_CR_PG,
	OFFSET_FLASH_CR_PER,
	OFFSET_FLASH_CR_MER,
	OFFSET_FLASH_CR_OPTPG = 4,
	OFFSET_FLASH_CR_OPTER,
	OFFSET_FLASH_CR_STRT,
	OFFSET_FLASH_CR_LOCK,
	OFFSET_FLASH_CR_OPTWRE = 9,
	OFFSET_FLASH_CR_ERRIE,
	OFFSET_FLASH_CR_EOPIE = 12,
}FLASH_CR_Offset_t;

typedef enum
{
	OFFSET_FLASH_SR_BSY,
	OFFSET_FLASH_SR_PGERR = 2,
	OFFSET_FLASH_SR_WRPRTERR = 4,
	OFFSET_FLASH_SR_EOP,
}FLASH_SR_Offset_t;

#define RDPRT_KEY		0x00A5
#define KEY1			0x45670123
#define KEY2			0xCDEF89AB

#define FPEC_REGISTER		((volatile FPEC_Registers_t*)0x40022000)

#endif /* INC_FPEC_H_ */
