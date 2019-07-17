#include <system_LPC17xx.h>
#include <lpc17xx.h>
#include <stdint.h>
#include <lpc17xx_i2c.h>
#include <lpc17xx_can.h>


#include "../Libs/max11612.h"
#include "CanFunc.h"

void I2C2_IRQHandler(void)
{
	static uint8_t cnt = 0;
	
	I2C_MasterHandler(LPC_I2C2);
	
	if (I2C_MasterTransferComplete(LPC_I2C2))
	{
		cnt = 0;
		Max11612_SetData();
	}
	else
	{		
		if(++cnt >= 4)
			Max11612_ClearData();
	}
}


void CAN_IRQHandler(void)
{
	uint8_t IntStatus;
//	uint32_t data1;
	/* Get CAN status */
	IntStatus = CAN_GetCTRLStatus(LPC_CAN1, CANCTRL_STS);
	
	if(IntStatus & (1 << CANINT_RIE))
	{
		CanMsg *msg = canRxQueue_GetWriteMsg(0);
		if(msg != 0)
			CanReceiveThread(0, msg);		
	}
	
	IntStatus = CAN_GetCTRLStatus(LPC_CAN2, CANCTRL_STS);
	if(IntStatus & (1 << CANINT_RIE))
	{
		CanMsg *msg = canRxQueue_GetWriteMsg(1);
		if(msg != 0)
			CanReceiveThread(1, msg);		
	}
	
	IntStatus = CAN_GetCTRLStatus(LPC_CAN1, CANCTRL_INT_CAP);
	IntStatus = CAN_GetCTRLStatus(LPC_CAN1, CANCTRL_ERR_WRN);	
	
	
}


