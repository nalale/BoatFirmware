#include "Main.h"
#include "../MolniaLib/MF_Tools.h"

#include "../Libs/Btn8982.h"
#include "../Libs/filter.h"
#include "../BoardDefinitions/MarineEcu_Board.h"

#include "SteeringFunc.h"
#include "PwmFunc.h"
#include "TimerFunc.h"
#include "ExternalProtocol.h"

FILTER_STRUCT _ctrlPos;

static int16_t SteeringControl(SteeringData_t *_steeringData, int16_t TargetAngle);
static int16_t PID_Controller(SteeringData_t *_steeringData, uint16_t feedback_value, uint16_t set_value);

uint8_t SteeringInit(SteeringData_t *_steeringData, uint8_t *FeedbackVoltage_0p1V, uint8_t MinPosLimit_V, uint8_t MaxPosLimit_V, uint16_t *fstDriverCurrent, uint16_t *sndDriverCurrent, uint16_t CurrentLimit_0p1A)
{
	EcuConfig_t cfgEcu = GetConfigInstance();
	
	_steeringData->FeedbackVoltage_0p1V = FeedbackVoltage_0p1V;

	_steeringData->fstDriverCurrent_0p1A = fstDriverCurrent;
	_steeringData->sndDriverCurrent_0p1A = sndDriverCurrent;

	_steeringData->SteeringMinVal_0p1V = MinPosLimit_V;
	_steeringData->SteeringMaxVal_0p1V = MaxPosLimit_V;
	_steeringData->CurrentLimit_0p1A = CurrentLimit_0p1A;
	
	_steeringData->FeedbackVoltageRanges[0] = MinPosLimit_V;
	_steeringData->FeedbackVoltageRanges[1] = MaxPosLimit_V;
	_steeringData->FeedbackVoltageRanges[2] = 0;
	_steeringData->FeedbackVoltageRanges[3] = 4095;
	
	_steeringData->kp = cfgEcu.SteeringKp;
	_steeringData->ki = cfgEcu.SteeringKi;
	_steeringData->kd = cfgEcu.SteeringKd;

	_steeringData->SteeringFuncIsReady = 1;

//	Filter_init(cfgEcu.fltSteeringLength, cfgEcu.fltSteeringPeriod, &_ctrlPos);
	
	return 0;
}


uint8_t SteeringProc(SteeringData_t *_steeringData, int16_t TargetAngle)
{
//	if(data->EndStopLeft_Ack && data->EndStopRight_Ack)
//		_steeringData.SteeringFuncIsReady = 1;
//	else
//		_steeringData.SteeringFuncIsReady = 0;

	if(!_steeringData->SteeringFuncIsReady)
		return 1;

	_steeringData->DriveCurrent_0p1 = SteeringGetDriveCurrent(_steeringData);
	_steeringData->FeedBackAngle = interpol(_steeringData->FeedbackVoltageRanges, 2, *(_steeringData->FeedbackVoltage_0p1V));

	SteeringControl(_steeringData, TargetAngle);
	
	return 0;
}


int16_t SteeringControl(SteeringData_t *_steeringData, int16_t TargetAngle)
{    		
	EcuConfig_t cfgEcu = GetConfigInstance();
	
	int16_t reaction = PID_Controller(_steeringData, _steeringData->FeedBackAngle, TargetAngle);
    
	//uint16_t _pwm_cmd = Filter(steering_speed_cmd, &_ctrlPos);
	
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

uint8_t GetSteeringBrakeValue(int16_t ActualSpeed, int16_t* BrakeSpeedTable)
{
    int16_t result_brake = interpol((int16_t*)BrakeSpeedTable, 6, ActualSpeed);
    result_brake = (result_brake > 100)? 100 : (result_brake < 0)? 0: result_brake;
    
    return (uint8_t)result_brake;
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

uint16_t HelmGetTargetAngle(Helm_Data_Rx_t *HelmData)
{
	uint16_t result;

	HelmData->HelmIsReady = SendSteeringStartupMsg();

	if(HelmData->HelmIsReady)
		result = HelmData->TargetAngle;
	else
		result = 2048;

	return result;
}

