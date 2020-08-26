/*
 * TransmissionDriver.c
 *
 *  Created on: 16 мая 2020 г.
 *      Author: a.lazko
 */

#include <stdint.h>
#include "TimerFunc.h"
#include "TransmissionDriver.h"

static TransmissionHandle_t transHandle;

int8_t transmissionInit(uint16_t SpeedForShifting, void (*ShiftCommandCallBack)(TransmissionGear_e DemandGear))
{
	if(ShiftCommandCallBack == 0)
		return -1;

	transHandle.SpeedForShifting = (int16_t)SpeedForShifting;
	transHandle.CmdCallBack = ShiftCommandCallBack;

	return 0;
}

int8_t transmissionThread(int16_t ShaftSpeed)
{
	if(transHandle.CmdCallBack == 0)
		return -1;

	//if((transHandle.ActualGear != transHandle.DemandGear) && (transHandle.DemandGear != GEAR_NEU))
	//{

	if(transHandle.ActualGear != transHandle.DemandGear && transHandle.DemandGear != GEAR_NEU && !transHandle.IsShifting)
	{
		transHandle.Timer = GetTimeStamp();
		transHandle.IsShifting = 1;
	}

	if(ShaftSpeed < transHandle.SpeedForShifting && ShaftSpeed > -transHandle.SpeedForShifting)
	{
		if(transHandle.DemandGear == GEAR_FW)
			transHandle.CmdCallBack(GEAR_FW);
		else if(transHandle.DemandGear == GEAR_BW)
			transHandle.CmdCallBack(GEAR_BW);

		if(GetTimeFrom(transHandle.Timer) >= 1500 ||
				(transHandle.ActuatorPosition == 0 && transHandle.ActuatorPositionPrev == 1))
		{
			transHandle.ActualGear = transHandle.DemandGear;
			transHandle.IsShifting = 0;
		}
	}
//	}
//	else
//	{
//		//transHandle.CmdCallBack(GEAR_NEU);
//		transHandle.IsShifting = 0;
//	}

	transHandle.ActuatorPositionPrev = transHandle.ActuatorPosition;

	return 0;
}

int8_t transmissionSetGear(TransmissionGear_e gear)
{
	if(gear > GEAR_BW)// || gear < GEAR_NEU)
		return -1;

	transHandle.DemandGear = gear;
	return 0;
}

int8_t transmissionMovementPermission()
{
	return transHandle.ActuatorPosition;
}

TransmissionGear_e transmissionGetActualGear()
{
	return transHandle.ActualGear;
}

int8_t transmissionShiftInProcess()
{
	return transHandle.IsShifting;
}

int8_t transmissionSetEndSwithPosition(uint8_t Position)
{
	transHandle.ActuatorPosition = Position;
	return 0;
}
