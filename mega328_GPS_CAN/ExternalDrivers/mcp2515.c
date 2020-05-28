/******************************************************************************
 * 
 * Controller Area Network (CAN) Demo-Application
 * Atmel AVR with Microchip MCP2515 
 * 
 * Copyright (C) 2005 Martin THOMAS, Kaiserslautern, Germany
 * <eversmith@heizung-thomas.de>
 * http://www.siwawi.arubi.uni-kl.de/avr_projects
 *
 *****************************************************************************
 *
 * File    : mcp2515.c
 * Version : 0.9
 * 
 * Summary : MCP2515 "low-level" driver
 *
 * Parts of this code are adapted from a MCP2510 sample-application 
 * by KVASER AB, http://www.kvaser.com (KVASER-code is marked as free)
 *
 * This code-module is free to use but you have to keep the copyright
 * notice.
 *
 *****************************************************************************/


#include <stdint.h>        /* For uint8_t definition */
#include "mcp2515.h"
#include "mcp2515_defs.h"
#include "mcp2515_bittime.h"

uint8_t (*spi_readwrite)(uint8_t byte_out);

uint8_t McpStatus[MCP_AMOUNT];

void mcp2515_setSpiCallBack(uint8_t (*spi_byte_exchange)(uint8_t byte_out))
{
    if(spi_byte_exchange != 0)
        spi_readwrite = spi_byte_exchange;
}

void mcp2515_reset(MCP_CS cs)
{
	MCP2515_SELECT(cs);
	spi_readwrite(MCP_RESET);
	MCP2515_UNSELECT(cs);
	//delay_ms(10); // rough - but > 128 MCP clock-cycles
}

uint8_t mcp2515_readRegister(MCP_CS cs, uint8_t address)
{
	uint8_t ret;
	
	MCP2515_SELECT(cs);
	spi_readwrite(MCP_READ);
	spi_readwrite(address);
	ret = spi_readwrite(0xFF);
	MCP2515_UNSELECT(cs);
	
	return ret;
}

void mcp2515_readRegisterS(MCP_CS cs, uint8_t address,
	uint8_t values[],  uint8_t n)
{
	uint8_t i;
	
	MCP2515_SELECT(cs);
	spi_readwrite(MCP_READ);
	spi_readwrite(address);
	// mcp2515 has auto-increment of address-pointer
	for (i=0; i<n; i++) {
		values[i] = spi_readwrite(0xFF);
	}
	MCP2515_UNSELECT(cs);
}

void mcp2515_setRegister(MCP_CS cs, uint8_t address,  uint8_t value)
{
	MCP2515_SELECT(cs);
	spi_readwrite(MCP_WRITE);
	spi_readwrite(address);
	spi_readwrite(value);
	MCP2515_UNSELECT(cs);
}

void mcp2515_setRegisterS(MCP_CS cs, uint8_t address,
	 uint8_t values[],  uint8_t n)
{
	uint8_t i;
	
	MCP2515_SELECT(cs);
	spi_readwrite(MCP_WRITE);
	spi_readwrite(address);
	// mcp2515 has auto-increment of address-pointer
	for (i=0; i<n; i++) {
		spi_readwrite(values[i]);
	}
	MCP2515_UNSELECT(cs);
}

void mcp2515_modifyRegister( MCP_CS cs,uint8_t address,
	 uint8_t mask,  uint8_t data)
{
	MCP2515_SELECT(cs);
	spi_readwrite(MCP_BITMOD);
	spi_readwrite(address);
	spi_readwrite(mask);
	spi_readwrite(data);
	MCP2515_UNSELECT(cs);
}

static uint8_t mcp2515_readXXStatus_helper( MCP_CS cs,uint8_t cmd)
{
	uint8_t i;
	
	MCP2515_SELECT(cs);
	spi_readwrite(cmd);
	i = spi_readwrite(0xFF);
	MCP2515_UNSELECT(cs);
	
	return i;
}
	
uint8_t mcp2515_readStatus(MCP_CS cs)
{
	return mcp2515_readXXStatus_helper(cs, MCP_READ_STATUS);
}

uint8_t mcp2515_RXStatus(MCP_CS cs)
{
	return mcp2515_readXXStatus_helper(cs, MCP_RX_STATUS);
}

uint8_t mcp2515_setCANCTRL_Mode(MCP_CS cs, uint8_t newmode)
{
	uint8_t i;
	
	mcp2515_modifyRegister( cs,MCP_CANCTRL, MODE_MASK, newmode);
	
	// verify as advised in datasheet
	i = mcp2515_readRegister(cs,MCP_CANCTRL);
	i &= MODE_MASK;
	
	if ( i == newmode ) {
		return MCP2515_OK; 
	}
	else {
		return MCP2515_FAIL;
	}
}


uint8_t mcp2515_configRate( MCP_CS cs, MCP_CAN_BAUDRATE canSpeed)
{
	uint8_t set, cfg1, cfg2, cfg3;
	
	set = 0;
	
	switch (canSpeed) {
		case (kBAUD500) :
			cfg1 = 0x00;//MCP_4MHz_125kBPS_CFG1 ;
			cfg2 = 0xB8;//MCP_4MHz_125kBPS_CFG2 ;
			cfg3 = 0x05;//MCP_4MHz_125kBPS_CFG3 ;
			set = 1;
			break;
		case (kBAUD250) :
			cfg1 = 0x1 ;
			cfg2 = 0xB8 ;
			cfg3 = 0x05 ;
			set = 1;
			break;
		default:
			set = 0;
			break;
	}
	
	if (set) {
		mcp2515_setRegister(cs,MCP_CNF1, cfg1);
		mcp2515_setRegister(cs,MCP_CNF2, cfg2);
		mcp2515_setRegister(cs,MCP_CNF3, cfg3);
		return MCP2515_OK;
	}
	else {
		return MCP2515_FAIL;
	}
} 

// ---

void mcp2515_read_can_id(MCP_CS cs,  uint8_t mcp_addr,
	uint8_t* ext, uint32_t* can_id )
{
    uint8_t tbufdata[4];
	
    *ext = 0;
    *can_id = 0;
    
	mcp2515_readRegisterS(cs, mcp_addr, tbufdata, 4 );
    
	*can_id = (tbufdata[MCP_SIDH]<<3) + (tbufdata[MCP_SIDL]>>5);
	
    if ( (tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M ) {
		// extended id
        *can_id = (*can_id<<2) + (tbufdata[MCP_SIDL] & 0x03);
        *can_id <<= 16;
		*can_id = *can_id + (uint32_t)(((tbufdata[MCP_EID8]<<8) + tbufdata[MCP_EID0]) & 0xFFFF);
        *ext = 1;
    }
}

uint8_t can_readMessage(MCP_CS cs, CanMsg *msg)
{
	uint8_t stat, res;

	if ((cs == MCP_CS1_U6) && (McpStatus[MCP_CAN1_NUM] != MCP2515_OK))
		return 0;

	stat = mcp2515_readStatus(cs);

	if (stat & MCP_STAT_RX0IF)
	{
		// Msg in Buffer 0
		mcp2515_read_canMsg(cs, MCP_RXBUF_0, msg);
		mcp2515_modifyRegister(cs, MCP_CANINTF, MCP_RX0IF, 0);
		res = 1;
	}
	else if (stat & MCP_STAT_RX1IF)
	{
		// Msg in Buffer 1
		mcp2515_read_canMsg(cs, MCP_RXBUF_1, msg);
		mcp2515_modifyRegister(cs, MCP_CANINTF, MCP_RX1IF, 0);
		res = 1;
	}
	else
	{
		res = 0;
	}

	return res;
}
// Buffer can be MCP_RXBUF_0 or MCP_RXBUF_1
void mcp2515_read_canMsg(MCP_CS  cs, uint8_t buffer_sidh_addr,
	CanMsg* msg)
{

    
    uint8_t mcp_addr;
   // uint8_t  ctrl;
    uint32_t Id;
    uint8_t ex;

	mcp_addr = buffer_sidh_addr;
	
    mcp2515_read_can_id(cs, mcp_addr, &ex,&Id );
        msg->ID = Id;
        msg->Ext = ex;
	//ctrl = mcp2515_readRegister(cs, mcp_addr-1 );
         msg->DLC = mcp2515_readRegister(cs, mcp_addr+4 );
    
//	if (/*(*dlc & RTR_MASK) || */(ctrl & 0x08)) {
//        msg->rtr = 1;
//    } else {
//        msg->rtr = 0;
//    }
    
	msg->DLC &= MCP_DLC_MASK;
    mcp2515_readRegisterS(cs, mcp_addr+5, &(msg->data[0]), msg->DLC );
}


void mcp2515_write_can_id( MCP_CS cs, uint8_t mcp_addr,
	 uint8_t ext,  uint32_t can_id )
{
    uint16_t canid;
    uint8_t tbufdata[4];
	
    canid = (uint16_t)(can_id & 0x0FFFF);
    
	if ( ext == 1) {
        tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
        tbufdata[MCP_EID8] = (uint8_t) (canid / 256);
        canid = (uint16_t)( can_id / 0x10000L );
        tbufdata[MCP_SIDL] = (uint8_t) (canid & 0x03);
        tbufdata[MCP_SIDL] += (uint8_t) ((canid & 0x1C )*8);
        tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        tbufdata[MCP_SIDH] = (uint8_t) (canid / 32 );
    }
    else {
        tbufdata[MCP_SIDH] = (uint8_t) (canid / 8 );
        tbufdata[MCP_SIDL] = (uint8_t) ((canid & 0x07 )*32);
        tbufdata[MCP_EID0] = 0;
        tbufdata[MCP_EID8] = 0;
    }
	mcp2515_setRegisterS( cs, mcp_addr, tbufdata, 4 );
}
// 
uint8_t mcp2515_getNextFreeTXBuf(MCP_CS cs,uint8_t *txbuf_n)
{
	uint8_t res, i, ctrlval;
	uint8_t ctrlregs[MCP_N_TXBUFFERS] = { MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL };
        static uint8_t TXBufNum = MCP_TXB0CTRL;

	res = MCP_ALLTXBUSY;
	*txbuf_n = 0x00;

	// check all 3 TX-Buffers
	for (i=0; i<MCP_N_TXBUFFERS; i++) {
                if (++TXBufNum >= MCP_N_TXBUFFERS) TXBufNum = 0;
		ctrlval = mcp2515_readRegister(cs, ctrlregs[i] );
		if ( (ctrlval & MCP_TXB_TXREQ_M) == 0 ) {

			*txbuf_n = ctrlregs[i]; // return SIDH-address of Buffer
			res = MCP2515_OK;
			return res; /* ! function exit */
		}
	}

	return res;
}

// Buffer can be MCP_TXBUF_0 MCP_TXBUF_1 or MCP_TXBUF_2
void mcp2515_write_canMsg(MCP_CS cs,CanMsg* msg)
{
        // TODO        нужно завести два счётчика буферов, отдельно для каждой MCP2515

        uint8_t tx_control = 0;
        uint8_t dlc;
        uint8_t mcp_addr;
        uint8_t TxBufState = 0;
        // TODO  Обработать ошибку, когда все буферы заняты.
        

        TxBufState = mcp2515_getNextFreeTXBuf( cs, &tx_control);

        if (TxBufState == MCP2515_OK){
            switch (tx_control){
                case MCP_TXB0CTRL:
                    mcp_addr = MCP_TXBUF_0;
                    break;
                case MCP_TXB1CTRL:
                    mcp_addr = MCP_TXBUF_1;
                    break;
                case MCP_TXB2CTRL:
                    mcp_addr = MCP_TXBUF_2;
                    break;
                default:
                    mcp_addr = MCP_TXBUF_2;
                    break;

            }
            dlc = msg->DLC;
            mcp2515_setRegisterS(cs , mcp_addr+5, &(msg->data[0]), dlc );  // write data bytes
            mcp2515_write_can_id(cs, mcp_addr, msg->Ext, msg->ID );  // write CAN id
            // if ( msg->rtr == 1)  dlc |= MCP_RTR_MASK;  // if RTR set bit in byte
            mcp2515_setRegister(cs, (mcp_addr+4), dlc );  // write the RTR and DLC
            mcp2515_start_transmit(cs,tx_control);
        }else{
           dlc = 0;
        }
	
       

}

/*
 ** Start the transmission from one of the tx buffers.
 */
// Buffer can be MCP_TXBUF_0 MCP_TXBUF_1 or MCP_TXBUF_2
void mcp2515_start_transmit(MCP_CS cs, uint8_t buffer_sidh_addr)
{
	// TXBnCTRL_addr = TXBnSIDH_addr - 1
    mcp2515_modifyRegister(cs, buffer_sidh_addr, MCP_TXB_TXREQ_M,
		MCP_TXB_TXREQ_M );
}



void mcp2515_initCANBuffers(MCP_CS cs)
{
	uint8_t i, a1, a2, a3;
	
	// TODO: check why this is needed to receive extended 
	//   and standard frames
	// Mark all filter bits as don't care:
    mcp2515_write_can_id(cs,MCP_RXM0SIDH, 0, 0);
    mcp2515_write_can_id(cs,MCP_RXM1SIDH, 0, 0);
    // Anyway, set all filters to 0:
    mcp2515_write_can_id(cs,MCP_RXF0SIDH, 1, 0); // RXB0: extended
    mcp2515_write_can_id(cs,MCP_RXF1SIDH, 0, 0); //       AND standard
    mcp2515_write_can_id(cs,MCP_RXF2SIDH, 1, 0); // RXB1: extended
    mcp2515_write_can_id(cs,MCP_RXF3SIDH, 0, 0); //       AND standard
    mcp2515_write_can_id(cs,MCP_RXF4SIDH, 0, 0);
    mcp2515_write_can_id(cs,MCP_RXF5SIDH, 0, 0);
	
	// Clear, deactivate the three transmit buffers
	// TXBnCTRL -> TXBnD7
        a1 = MCP_TXB0CTRL;
	a2 = MCP_TXB1CTRL;
	a3 = MCP_TXB2CTRL;
    for (i = 0; i < 14; i++) { // in-buffer loop
		mcp2515_setRegister(cs,a1, 0);
		mcp2515_setRegister(cs,a2, 0);
		mcp2515_setRegister(cs,a3, 0);
        a1++;
		a2++;
		a3++;
    }
	
    // and clear, deactivate the two receive buffers.
    mcp2515_setRegister(cs,MCP_RXB0CTRL, 0);
    mcp2515_setRegister(cs,MCP_RXB1CTRL, 0);
}


// ---

uint8_t mcp2515_init(MCP_CS cs, MCP_NUM num, MCP_CAN_BAUDRATE canSpeed)
{
	uint8_t res;

	MCP2515_UNSELECT(cs);
	// Сброс на всякий случай
	mcp2515_reset(cs);

	//mcp2515_modifyRegister(MCP_CS1_U6 ,MCP_CANCTRL, MODE_MASK, MODE_CONFIG);

	res = mcp2515_setCANCTRL_Mode(cs, MODE_CONFIG);

	if (res == MCP2515_FAIL)
	{
		SetMcpStatus(num, res);
		return res; /* function exit on error */
	}

	mcp2515_modifyRegister(cs, MCP_CANCTRL, CLK_MASK, CLKOUT_PS1);
	res = mcp2515_configRate(cs, canSpeed);

	if (res == MCP2515_OK)
	{
		mcp2515_initCANBuffers(cs);

#if (DEBUG_RXANY==1)
		// enable both receive-buffers to receive any message
		// and enable rollover
		mcp2515_modifyRegister(MCP_RXB0CTRL,
				MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
				MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
		mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
				MCP_RXB_RX_ANY);
#else
		// enable both receive-buffers to receive messages
		// with std. and ext. identifiers
		// and enable rollover
		mcp2515_modifyRegister(cs, MCP_RXB0CTRL,
				MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
				MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
		mcp2515_modifyRegister(cs, MCP_RXB1CTRL, MCP_RXB_RX_MASK,
				MCP_RXB_RX_STDEXT);
#endif
	}
	
	res = mcp2515_setCANCTRL_Mode(cs, MODE_NORMAL);
	SetMcpStatus(num, res);
	return res;
}

// Установить статуы для каждой микрухи
void SetMcpStatus(MCP_NUM num, uint8_t status)
{
    McpStatus[num]  = status;
}

#ifdef MCPDEBUG
void mcp2515_dumpExtendedStatus(void)
{
	uint8_t tec, rec, eflg;
	
	tec  = mcp2515_readRegister(MCP_TEC);
	rec  = mcp2515_readRegister(MCP_REC);
	eflg = mcp2515_readRegister(MCP_EFLG);
	
	term_puts_P("MCP2515 Extended Status:\n");
	Debug_ByteToUart_p(PSTR("MCP Transmit Error Count"), tec);
	Debug_ByteToUart_p(PSTR("MCP Receiver Error Count"), rec);
	Debug_ByteToUart_p(PSTR("MCP Error Flag"), eflg);
	
	if ( (rec>127) || (tec>127) ) {
		term_puts_P("Error-Passive or Bus-Off\n");
	}

	if (eflg & MCP_EFLG_RX1OVR) 
		term_puts_P("Receive Buffer 1 Overflow\n");
	if (eflg & MCP_EFLG_RX0OVR) 
		term_puts_P("Receive Buffer 0 Overflow\n");
	if (eflg & MCP_EFLG_TXBO) 
		term_puts_P("Bus-Off\n");
	if (eflg & MCP_EFLG_TXEP) 
		term_puts_P("Receive Error Passive\n");
	if (eflg & MCP_EFLG_TXWAR) 
		term_puts_P("Transmit Error Warning\n");
	if (eflg & MCP_EFLG_RXWAR) 
		term_puts_P("Receive Error Warning\n");
	if (eflg & MCP_EFLG_EWARN ) 
		term_puts_P("Receive Error Warning\n");
}
#endif

#if 0 
// read-modify-write - better: Bit Modify Instruction
uint8_t mcp2515_setCANCTRL_Mode(uint8_t newmode)
{
	uint8_t i;
	
	i = mcp2515_readRegister(MCP_CANCTRL);
	i &= ~(MODE_MASK);
	i |= newmode;
	mcp2515_setRegister(MCP_CANCTRL, i);
	
	// verify as advised in datasheet
	i = mcp2515_readRegister(MCP_CANCTRL);
	i &= MODE_MASK;
	if ( i == newmode ) {
		return MCP2515_OK; 
	}
	else {
		return MCP2515_FAIL;
	}
}

#endif
