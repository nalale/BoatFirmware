#include "TrimFunc.h"

uint8_t TrimInit(TrimData_t *TrimData, uint8_t* pFeedback_0p1V, uint16_t MaxFeedback_0p1V, uint16_t MinFeedback_0p1V)
{
	TrimData->FeedBack_mV = pFeedback_0p1V;			// Установка указателя на данные обратной связи
	TrimData->MaxVoltage_0p1V = MaxFeedback_0p1V;
	TrimData->MinVoltage_0p1V = MinFeedback_0p1V;

	return 0;
}

uint8_t TrimProc(TrimData_t *TrimData, uint8_t cmdUp, uint8_t cmdDown)
{
	if(cmdUp || cmdDown)
	{
		uint8_t fb_correct = (*(TrimData->FeedBack_mV) > TrimData->MaxVoltage_0p1V) && (*(TrimData->FeedBack_mV) < TrimData->MinVoltage_0p1V);
		TrimData->MovCmd = cmdDown | (cmdUp << 1);

		if((TrimData->MovCmd == 1) && fb_correct)
		{
			SET_C_OUT6(1);
			SET_C_OUT5(1);
		}
		else if((TrimData->MovCmd == 2) && fb_correct)
		{
			SET_C_OUT6(0);
			SET_C_OUT5(1);
		}
		else
		{
			if(!fb_correct)
			{
				// Если напряжение меньше 0.5 Вольт - обрыв линии обратной связи
				if(*(TrimData->FeedBack_mV) < 5)
					TrimData->Faults.F_Feedback = 1;
				else
					TrimData->Faults.F_Position = 1;
			}

			SET_C_OUT6(0);
			SET_C_OUT5(0);
		}
	}
	else
	{
		SET_C_OUT6(0);
		SET_C_OUT5(0);
	}

	return 0;
}

uint8_t TrimGetState(const TrimData_t *TrimData)
{
	return TrimData->Faults.Value;
}
