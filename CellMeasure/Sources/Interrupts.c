#include <stdlib.h>
#include <stdint.h>

#include "CanFunc.h"
#include "lpc17xx_can.h"

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
		else if(IntStatus & (1 << CANINT_DOIE))
		{
			CAN_SetCommand(LPC_CAN1, CAN_CMR_RRB);
			CAN_SetCommand(LPC_CAN1, CAN_CMR_CDO);
		}		
	}
	
	IntStatus = CAN_GetCTRLStatus(LPC_CAN2, CANCTRL_STS);
	if(IntStatus & (1 << CANINT_RIE))
	{
		CanMsg *msg = canRxQueue_GetWriteMsg(1);
		if(msg != 0)
			CanReceiveThread(1, msg);
		else if(IntStatus & (1 << CANINT_DOIE))
		{
			CAN_SetCommand(LPC_CAN2, CAN_CMR_RRB);
			CAN_SetCommand(LPC_CAN2, CAN_CMR_CDO);		
		}				
	}
	
	IntStatus = CAN_GetCTRLStatus(LPC_CAN1, CANCTRL_INT_CAP);
	IntStatus = CAN_GetCTRLStatus(LPC_CAN1, CANCTRL_ERR_WRN);	
	
	
}

