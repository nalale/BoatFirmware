#ifndef _MERCURY_AR_03_CL_H_
#define _MERCURY_AR_03_CL_H_


typedef enum {
	MERQURY_REQ_FROM_RESET = 0x01,
	MERQURY_REQ_FOR_CURRENT_YEAR,
	MERQURY_REQ_FOR_PREV_YEAR,
	MERQURY_REQ_FOR_MONTH,
	MERQURY_REQ_FOR_CURRENT_DAY,
	MERQURY_REQ_FOR_PREV_DAY,
	MERQURY_REQ_FOR_PHASE
	
} MERCURY_ARRAY_NUM_t;

typedef enum {
	MERQURY_CMD_TEST = 0x00,
	MERQURY_CMD_OPEN,
	MERQURY_CMD_CLOSE,
	MERQURY_CMD_WRITE,	
	MERQURY_CMD_READ_TIME,
	MERQURY_CMD_READ_ARRAY,
	MERQURY_CMD_READ_PHYS_MEM,
	MERQURY_CMD_WRITE_PHYS_ADDR,
	MERQURY_CMD_READ_PARAMETERS
	
} MERCURY_CMD_t;


uint8_t GetCommandMsg(MERCURY_CMD_t command, MERCURY_ARRAY_NUM_t array_number, uint8_t address, uint8_t *buf, uint8_t *length);
uint16_t UpdCRC(uint8_t *buf, uint16_t len);









#endif
