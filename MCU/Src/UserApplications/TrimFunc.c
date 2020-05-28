#include "UserApplications/TrimFunc.h"
#include "../MolniaLib/MF_Tools.h"

uint8_t TrimInit(TrimData_t *TrimData, const uint8_t* pFeedback_0p1V, uint16_t MaxFeedback_0p1V, uint16_t MinFeedback_0p1V, uint16_t DriveUpperLimitFB_0p1V)
{
	TrimData->FeedBack_mV = pFeedback_0p1V;			// ”становка указател€ на данные обратной св€зи
	TrimData->VoltagePosTable_0p1V[0] = MaxFeedback_0p1V;
	TrimData->VoltagePosTable_0p1V[1] = MinFeedback_0p1V;
	TrimData->VoltagePosTable_0p1V[2] = 100;
	TrimData->VoltagePosTable_0p1V[3] = 0;
	TrimData->DriveUpperLimit_0p1V = DriveUpperLimitFB_0p1V;

	return 0;
}

uint8_t TrimProc(TrimData_t *TrimData, int16_t ShaftVelocity)
{
	if(TrimData == 0 || TrimData->FeedBack_mV == 0)
		return 1;

	uint16_t *upV = (ShaftVelocity < 20 && ShaftVelocity > -20)? &TrimData->VoltagePosTable_0p1V[0] : &TrimData->DriveUpperLimit_0p1V;		// 1.3
	uint16_t *downV = &TrimData->VoltagePosTable_0p1V[1];	// 3.5
	const uint8_t *fbV = TrimData->FeedBack_mV;
	
	// Operation condition for up cmd
	uint8_t up_cmd_cond = (*upV < *downV)? *fbV > *upV :
											*fbV < *upV;

	// Operation condition for down cmd
	uint8_t down_cmd_cond = (*upV < *downV)? *fbV < *downV :
										*fbV > *downV;

	if((TrimData->MovCmd == cmdTrim_Up) && up_cmd_cond)
	{
		TrimData->Status = stTrim_Enable;
		SET_C_OUT6(0);
		SET_C_OUT5(1);		
	}
	else if((TrimData->MovCmd == cmdTrim_Down) && down_cmd_cond)
	{
		TrimData->Status = stTrim_Enable;
		SET_C_OUT6(1);
		SET_C_OUT5(1);
	}
	else
	{
		TrimData->Status = stTrim_Disable;

		SET_C_OUT6(0);
		SET_C_OUT5(0);

		if(*(TrimData->FeedBack_mV) < 2)
		{
			TrimData->Status = stTrim_Fault;
			TrimData->Faults.F_Feedback = 1;
		}
	}
	
	if(*fbV < TrimData->DriveUpperLimit_0p1V - 1)
		TrimData->Status = stTrim_Warning;

	TrimData->Position = interpol((int16_t*)TrimData->VoltagePosTable_0p1V, 2, *TrimData->FeedBack_mV);

	return 0;
}

uint8_t TrimSetCmd(TrimData_t *TrimData, uint8_t cmdUp, uint8_t cmdDown)
{
	if(TrimData == 0)
		return 1;

	TrimData->MovCmd = cmdDown | (cmdUp << 1);

	return 0;
}

uint16_t TrimGetParameter(const TrimData_t *TrimData, TrimParameters_e Parameter)
{
	if(TrimData == 0)
		return 0;

	switch(Parameter)
	{
		case paramTrim_VoltageFB_0p1V:
			return (TrimData->FeedBack_mV == 0)? 0 : *TrimData->FeedBack_mV;
		case 	paramTrim_Position:
			return TrimData->Position;
		case 	paramTrim_Status:
			return TrimData->Status;
		case 	paramTrim_Fault:
			return TrimData->Faults.Value;
		case 	paramTrim_Cmd:
			return TrimData->MovCmd;

		default:
			return 0;
	}


}
