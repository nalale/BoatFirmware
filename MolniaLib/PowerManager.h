#ifndef _PM_H_
#define _PM_H_


typedef enum
{
	PM_PowerOff = 0,
	PM_PowerOn1,
	PM_PowerOn2,
	PM_ShutDown,
} PowerStates_e;

void PM_Proc(uint16_t voltageEcu, uint8_t IsPowerManager);

/*
Return value:
0 - Power supply
1 - Power off
*/
PowerStates_e PM_GetPowerState(void);

#endif
