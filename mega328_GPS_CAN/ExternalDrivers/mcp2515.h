#ifndef MCP2515_H_
#define MCP2515_H_

#include <avr/io.h>

#include "mcp2515_defs.h"


#define MCP_CS_PORT PORTB
//Количество микросхем MCP
#define MCP_AMOUNT 2

// Номер MCP
typedef enum {
	MCP_CAN1_NUM
} MCP_NUM;

// Чипселекты для MCP
typedef enum {
	MCP_CS1_U6 = 2
} MCP_CS;

typedef enum {
    kBAUD500 = 0, kBAUD250,
} MCP_CAN_BAUDRATE;

typedef struct
{   
    uint8_t 
	Ext : 	4,
	Rtr	:	4;
	
    uint8_t DLC;
    uint32_t ID;
    uint8_t data[8];
} CanMsg;

#define MCP_N_TXBUFFERS (3)

#define MCP_RXBUF_0 (MCP_RXB0SIDH)
#define MCP_RXBUF_1 (MCP_RXB1SIDH)

#define MCP_TXBUF_0 (MCP_TXB0SIDH)
#define MCP_TXBUF_1 (MCP_TXB1SIDH)
#define MCP_TXBUF_2 (MCP_TXB2SIDH)

// #define MCP2515_SELECT() (SPI_SS_LOW())
// #define MCP2515_UNSELECT() (SPI_SS_HIGH())

#define MCP2515_SELECT(x)   ( MCP_CS_PORT &= ~(1<<x) )
#define MCP2515_UNSELECT(x) ( MCP_CS_PORT |=  (1<<x) )

#define MCP2515_OK         (0)
#define MCP2515_FAIL       (1)
#define MCP_ALLTXBUSY      (2)

void mcp2515_reset(MCP_CS);

void mcp2515_setSpiCallBack(uint8_t (*spi_readwrite)(uint8_t byte_out));

uint8_t mcp2515_readRegister(MCP_CS, uint8_t address);
void mcp2515_setRegisterS(MCP_CS, uint8_t address, uint8_t values[], uint8_t n);

void mcp2515_setRegister(MCP_CS, uint8_t address, uint8_t value);
void mcp2515_setRegisterS(MCP_CS, uint8_t address, uint8_t values[], uint8_t n);
void mcp2515_modifyRegister(MCP_CS, uint8_t address, uint8_t mask, uint8_t data);

uint8_t mcp2515_readStatus(MCP_CS);
uint8_t mcp2515_RXStatus(MCP_CS);

uint8_t mcp2515_setCANCTRL_Mode(MCP_CS, uint8_t newmode);

uint8_t mcp2515_init(MCP_CS cs, MCP_NUM num, MCP_CAN_BAUDRATE canSpeed);
uint8_t mcp2515_configRate( MCP_CS cs, MCP_CAN_BAUDRATE canSpeed);

void mcp2515_write_can_id(MCP_CS, uint8_t mcp_addr, uint8_t ext, uint32_t can_id);
void mcp2515_read_can_id(MCP_CS, uint8_t mcp_addr, uint8_t* ext, uint32_t* can_id);
uint8_t can_readMessage(MCP_CS cs, CanMsg *msg);


void mcp2515_write_canMsg(MCP_CS, CanMsg* msg);
void mcp2515_read_canMsg(MCP_CS, uint8_t buffer_sidh_addr, CanMsg* msg);

void mcp2515_start_transmit(MCP_CS, uint8_t mcp_addr);

uint8_t mcp2515_getNextFreeTXBuf(MCP_CS, uint8_t *txbuf_n);
void SetMcpStatus(MCP_NUM num, uint8_t status);
#endif
