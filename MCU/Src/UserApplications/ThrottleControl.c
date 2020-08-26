/*
 * DriveControl.c
 *
 *  Created on: 1 мая 2020 г.
 *      Author: a.lazko
 */


/* ****************************************************************
 * ******************** Drive Control Function Module ************************
 * ****************************************************************
 */
 
#include "ThrottleControl.h"
#include "../MolniaLib/MF_Tools.h"
#include "../Libs/filter.h"

typedef struct
{
	int16_t FirstChannel[6];
	int16_t SecondChannel[6];
	const uint8_t* pFirstChannelV;
	const uint8_t* pSecondChannelV;
	driveAccUnitStatuses_e AccUnitStatus;// = dr_NonInit;
	driveDemandDirection_e AccDirection;// = dr_NeuDirect;
	uint8_t AccPosition;
	int16_t Table[12];
} AccPedalData_t;

static AccPedalData_t pedalData;
static FILTER_STRUCT fltAccCh_1;
static FILTER_STRUCT fltAccCh_2;


// First init struct
void thrHandleControlInit(uint8_t NeuV, uint8_t MinV, const uint8_t* Table, const uint8_t* FstChannelSrc, const uint8_t* SndChannelSrc)
{
	pedalData.FirstChannel[0] = MinV;
	pedalData.FirstChannel[1] = NeuV;
	pedalData.FirstChannel[2] = 100;
	pedalData.FirstChannel[3] = 0;
	
	for(uint8_t i = 0; i < 12; i++)
		pedalData.Table[i] = Table[i];

	pedalData.pFirstChannelV = FstChannelSrc;
	pedalData.pSecondChannelV = SndChannelSrc;
	pedalData.AccUnitStatus = dr_NonInit;
	pedalData.AccDirection = dr_NeuDirect;	

	pedalData.AccUnitStatus = dr_OK;

	Filter_init(100, 20, &fltAccCh_1);
	Filter_init(100, 20, &fltAccCh_2);
}

uint8_t thrHandleControlThread(int16_t MotorSpeed)
{

	if(pedalData.pFirstChannelV == 0 || pedalData.pSecondChannelV == 0)
	{
		pedalData.AccUnitStatus = dr_Fault;
		pedalData.AccPosition = 0;
		return 1;
	}

	// Filter both channel
	uint8_t ch_1 = *pedalData.pFirstChannelV;	//Filter(*pedalData.pFirstChannelV, &fltAccCh_1);
	uint8_t ch_2 = *pedalData.pSecondChannelV;	//Filter(*pedalData.pSecondChannelV, &fltAccCh_2);	
	
	if(ch_1 > ch_2 + 1)
	{
		if(ch_1 <= pedalData.FirstChannel[1] && ch_2 >= pedalData.FirstChannel[0])
		{		
			uint8_t pedal = interpol(pedalData.Table, 6, ch_2);
			pedalData.AccDirection = dr_BwDirect;

			pedalData.AccPosition = pedal;//Filter(pedal, &fltAccCh_2); //
		}
		else
		{
			pedalData.AccUnitStatus = dr_Fault;
			pedalData.AccDirection = dr_NeuDirect;
			pedalData.AccPosition = 0;
			Filter_set(&fltAccCh_2, 0);
		}
		
		Filter_set(&fltAccCh_1, 0);
	}
	else if(ch_2 > (--ch_1 + 1))		// --ch_1 => ch_1 has error offset
	{
		if(ch_2 <= pedalData.FirstChannel[1] + 5 && ch_1 >= pedalData.FirstChannel[0] - 5)
		{
			uint8_t pedal = interpol(pedalData.Table, 6, ch_1);
			pedalData.AccDirection = dr_FwDirect;

			pedalData.AccPosition = pedal;//Filter(pedal, &fltAccCh_1); //
		}
		else
		{
			pedalData.AccUnitStatus = dr_Fault;
			pedalData.AccDirection = dr_NeuDirect;
			pedalData.AccPosition = 0;
			Filter_set(&fltAccCh_1, 0);
		}	
		
		Filter_set(&fltAccCh_2, 0);
	}
	else
	{
		pedalData.AccDirection = dr_NeuDirect;
		pedalData.AccPosition = 0;
		
		Filter_set(&fltAccCh_1, 0);
		Filter_set(&fltAccCh_2, 0);
	}	

	return 0;
}

// Receive two channels from acceleration unit
uint8_t thrHandleGetDemandAcceleration()
{
    return pedalData.AccPosition;
}

driveDemandDirection_e thrHandleGetDirection()
{
    return pedalData.AccDirection;
}

driveAccUnitStatuses_e thrHandleGetStatus()
{
	return pedalData.AccUnitStatus;
}
