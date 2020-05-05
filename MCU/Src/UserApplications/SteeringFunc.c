#include "Main.h"
#include "../MolniaLib/MF_Tools.h"

#include "../Libs/Btn8982.h"
#include "../Libs/filter.h"
#include "../BoardDefinitions/MarineEcu_Board.h"

#include "PwmFunc.h"
#include "TimerFunc.h"
#include "ExternalProtocol.h"

#include "UserApplications/SteeringFunc.h"

static int16_t SteeringControl(SteeringData_t *_steeringData, int16_t TargetAngle);
static int16_t PID_Controller(SteeringData_t *_steeringData, uint16_t feedback_value, uint16_t set_value);

uint8_t SteeringInit(SteeringData_t *_steeringData, PID_Struct_t *PID,
		const uint8_t *srcFeedbackVoltage_0p1V, uint8_t MinPosLimit_V, uint8_t MaxPosLimit_V,
		const uint16_t *srcFstDriverCurrent, const uint16_t *srcSndDriverCurrent, uint16_t CurrentLimit_0p1A)
{
	
	_steeringData->FeedbackVoltage_0p1V = srcFeedbackVoltage_0p1V;

	_steeringData->fstDriverCurrent_0p1A = srcFstDriverCurrent;
	_steeringData->sndDriverCurrent_0p1A = srcSndDriverCurrent;

	_steeringData->SteeringMinVal_0p1V = MinPosLimit_V;
	_steeringData->SteeringMaxVal_0p1V = MaxPosLimit_V;
	_steeringData->CurrentLimit_0p1A = CurrentLimit_0p1A;
	
	_steeringData->FeedbackVoltageRanges[0] = MinPosLimit_V;
	_steeringData->FeedbackVoltageRanges[1] = MaxPosLimit_V;
	_steeringData->FeedbackVoltageRanges[2] = 0;
	_steeringData->FeedbackVoltageRanges[3] = 4095;
	
	_steeringData->kp = PID->Kp;
	_steeringData->ki = PID->Ki;
	_steeringData->kd = PID->Kd;

	_steeringData->SteeringFuncIsReady = 1;
	
	return 0;
}


uint8_t steeringThread(SteeringData_t *_steeringData)
{
	if(!_steeringData->SteeringFuncIsReady)
		return 1;

	_steeringData->DriveCurrent_0p1 = SteeringGetDriveCurrent(_steeringData);
	_steeringData->FeedBackAngle = interpol(_steeringData->FeedbackVoltageRanges, 2, *(_steeringData->FeedbackVoltage_0p1V));

	SteeringControl(_steeringData, _steeringData->UserDemandAngle);
	
	return 0;
}

uint8_t steeringSetAngle(SteeringData_t *_steeringData, int16_t TargetAngle)
{
	if(_steeringData == 0)
		return 1;

	_steeringData->UserDemandAngle = TargetAngle;
	return 0;
}

int16_t SteeringControl(SteeringData_t *_steeringData, int16_t TargetAngle)
{    		
	int16_t reaction = PID_Controller(_steeringData, _steeringData->FeedBackAngle, TargetAngle);

	if(reaction > 20)
	{		
		uint8_t val = reaction;
		
		if(_steeringData->DriveCurrent_0p1 < _steeringData->CurrentLimit_0p1A && !_steeringData->Faults.F_Current1)
		{
			_steeringData->Faults.F_Current2 = 0;
			btnSetOutputLevel(1, val);
			btnSetOutputLevel(0, 0);
		}
		else
		{
			_steeringData->Faults.F_Current1 = 1;
			btnSetOutputLevel(1, 0);
			btnSetOutputLevel(0, 0);
		}
	}
	else if(reaction < -20)
	{		
		uint8_t val = -reaction;
		
		if(_steeringData->DriveCurrent_0p1 < _steeringData->CurrentLimit_0p1A && !_steeringData->Faults.F_Current2)
		{
			_steeringData->Faults.F_Current1 = 0;
			btnSetOutputLevel(1, 0);
			btnSetOutputLevel(0, val);
		}
		else
		{
			_steeringData->Faults.F_Current2 = 1;
			btnSetOutputLevel(1, 0);
			btnSetOutputLevel(0, 0);
		}
	}
	else
	{
		btnSetOutputLevel(1, 0);
		btnSetOutputLevel(0, 0);
	}
    
    return 0;
}

uint16_t SteeringGetDriveCurrent(const SteeringData_t *_steeringData)
{
	return (*(_steeringData->fstDriverCurrent_0p1A) > *(_steeringData->sndDriverCurrent_0p1A))? *(_steeringData->fstDriverCurrent_0p1A) : *(_steeringData->sndDriverCurrent_0p1A);
}

int16_t SteeringGetFeedBackAngle(const SteeringData_t *_steeringData)
{
	return _steeringData->FeedBackAngle;
}

int16_t SteeringGetState(const SteeringData_t *_steeringData)
{
	return _steeringData->Faults.Value;
}

int16_t PID_Controller(SteeringData_t *_steeringData, uint16_t feedback_value, uint16_t set_value)
{
	static int16_t integral = 0;
	static int16_t pre_error = 0;
	
	int16_t error_value = set_value - feedback_value;          					// Calculate the error.
	integral = integral + error_value;                 							// Calculate integral.
	int16_t derivative = error_value - pre_error;              					// Calculate derivative.
	pre_error = error_value;													// Save as previous error.
	
	int16_t output = (_steeringData->kp * error_value / 100) + (_steeringData->ki * integral / 100) + (_steeringData->kd * derivative / 100);  // Calculate the output, pwm.

	if (output > 255) output = 255;												// Limit the output to maximum 255.
	else if (output < -255) output = -255;
	
	return output;
}



