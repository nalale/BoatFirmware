#include "TimerFunc.h"

#include "../MolniaLib/MF_Tools.h"
#include "PwmFunc.h"
#include "../Libs/Btn8982.h"
#include "../Libs/filter.h"

#include "SteeringFunc.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "Main.h"


FILTER_STRUCT _ctrlPos;

typedef struct
{
	uint8_t SteeringMinVal_0p1V;
	uint8_t SteeringMaxVal_0p1V;	
	
	int16_t FeedBackAngle;
	
	uint16_t DriveCurrent_0p1;
	
	uint8_t
	SteeringFuncIsReady	:	1,
	dummy7				:	7;
	
	uint8_t kp;
	uint8_t ki;
	uint8_t kd;
	
} _steer_t;

_steer_t _steeringData;



static int16_t feedback_voltage_tb[4];

static uint16_t _steerCurrentMeasured(void);
static int16_t SteeringControl(int16_t TargetAngle, int16_t SteeringDriveSpeed, uint16_t SteeringFeedback_0p1V);
static int16_t PID_Controller(uint16_t feedback_value, uint16_t set_value);

uint8_t steerInit(uint8_t MinPosLimit_V, uint8_t MaxPosLimit_V)
{
	
	EcuConfig_t cfgEcu = GetConfigInstance();
	
	_steeringData.SteeringMinVal_0p1V = MinPosLimit_V;
	_steeringData.SteeringMaxVal_0p1V = MaxPosLimit_V;
	
	feedback_voltage_tb[0] = MinPosLimit_V;
	feedback_voltage_tb[1] = MaxPosLimit_V;
	feedback_voltage_tb[2] = 0;
	feedback_voltage_tb[3] = 4095;
	
	_steeringData.kp = cfgEcu.SteeringKp;
	_steeringData.ki = cfgEcu.SteeringKi;
	_steeringData.kd = cfgEcu.SteeringKd;
	
//	Filter_init(cfgEcu.fltSteeringLength, cfgEcu.fltSteeringPeriod, &_ctrlPos);
	
	return 0;
}


uint8_t steerThread(Steering_Data_Rx_t *data)
{
	if(data->EndStopLeft_Ack && data->EndStopRight_Ack)
		_steeringData.SteeringFuncIsReady = 1;
	else
		_steeringData.SteeringFuncIsReady = 0;
	
	if(_steeringData.SteeringFuncIsReady)
	{				
		_steeringData.DriveCurrent_0p1 = _steerCurrentMeasured();
		SteeringControl(data->TargetAngle, data->SteeringSpeed_rads, data->Feedback_V);
	}
	return 0;
}


uint16_t _steerCurrentMeasured()
{
	uint16_t LeftSteeringCurrent = btnGetCurrent(0);
	uint16_t RightSteeringCurrent = btnGetCurrent(1);
	
	return (LeftSteeringCurrent > RightSteeringCurrent)? LeftSteeringCurrent : RightSteeringCurrent;
}

int16_t SteeringControl(int16_t TargetAngle, int16_t SteeringDriveSpeed, uint16_t SteeringFeedback_0p1V)
{    		
	if(SteeringFeedback_0p1V == 0)
		return 0;
    
    _steeringData.FeedBackAngle = interpol(feedback_voltage_tb, 2, SteeringFeedback_0p1V);
    	
	int16_t reaction = PID_Controller(_steeringData.FeedBackAngle, TargetAngle);
    
	//uint16_t _pwm_cmd = Filter(steering_speed_cmd, &_ctrlPos);
	
	Btn8982FaultList_e _ch1_state = btnGetCircuitState(0);
	Btn8982FaultList_e _ch2_state = btnGetCircuitState(1);
	
	
	if(reaction > 20)
	{
		if(_ch2_state == BTN_F_CURRENT_ABOVE_THRESHOLD)
		{
			EcuConfig_t cfgEcu = GetConfigInstance();
			btnInit(1, A_OUT2_CSENS, cfgEcu.SteeringMaxCurrent_0p1A);
		}
		
		uint8_t val = reaction;
		
		btnSetOutputLevel(1, val);
		btnSetOutputLevel(0, 0);
	}
	else if(reaction < -20)
	{
		if(_ch1_state == BTN_F_CURRENT_ABOVE_THRESHOLD)
		{
			EcuConfig_t cfgEcu = GetConfigInstance();
			btnInit(0, A_OUT1_CSENS, cfgEcu.SteeringMaxCurrent_0p1A);
		}
		
		uint8_t val = -reaction;
		
		btnSetOutputLevel(1, 0);
		btnSetOutputLevel(0, val);
	}
	else
	{
		btnSetOutputLevel(1, 0);
		btnSetOutputLevel(0, 0);
	}
    
    return 0;
}

uint16_t steerGetDriveCurrent()
{
	return _steeringData.DriveCurrent_0p1;
}

int16_t steerGetFeedBackAngle()
{
	return _steeringData.FeedBackAngle;
}

uint8_t GetSteeringBrakeValue(int16_t ActualSpeed, int16_t* BrakeSpeedTable)
{
    int16_t result_brake = interpol((int16_t*)BrakeSpeedTable, 6, ActualSpeed);
    result_brake = (result_brake > 100)? 100 : (result_brake < 0)? 0: result_brake;
    
    return (uint8_t)result_brake;
}


int16_t PID_Controller(uint16_t feedback_value, uint16_t set_value)
{
	static int16_t integral = 0;
	static int16_t pre_error = 0;
	
	int16_t error_value = set_value - feedback_value;          					// Calculate the error.
	integral = integral + error_value;                 							// Calculate integral.
	int16_t derivative = error_value - pre_error;              					// Calculate derivative.
	pre_error = error_value;													// Save as previous error.
	
	int16_t output = (_steeringData.kp * error_value / 100) + (_steeringData.ki * integral / 100) + (_steeringData.kd * derivative / 100);  // Calculate the output, pwm.

	if (output > 255) output = 255;												// Limit the output to maximum 255.
	else if (output < -255) output = -255;
	
	return output;
}

uint8_t CalibrateSteering(int16_t DriveCurrent_0p1A, uint16_t SteeringFeedback_0p1V)
{
	uint8_t res = 0;
	static uint8_t step = 0; 
	static uint8_t cnt = 0;
	static uint32_t timestamp = 0;
	static uint16_t feedback_prev = 0;
	
		switch(step)
		{
			case 0:				
				PwmUpdate(1, 70);
				PwmUpdate(2, 0);
				step = 1;
				break;
			case 1:
				if(GetTimeFrom(timestamp) > 500)
				{
					timestamp = GetTimeStamp();
					if((SteeringFeedback_0p1V > feedback_prev - 1) && (SteeringFeedback_0p1V < feedback_prev + 1))
						cnt++;
					else
					{
						feedback_prev = SteeringFeedback_0p1V;
						cnt = 0;
					}				
					
					if(cnt == 2)
					{
						_steeringData.SteeringMaxVal_0p1V = SteeringFeedback_0p1V + 2;
						PwmUpdate(1, 0);
						PwmUpdate(2, 0);
						cnt = 0;
						step = 2;
						break;
					}
				}
			break;
				
			case 2:
				PwmUpdate(1, 0);
				PwmUpdate(2, 70);	
				step = 3;
				break;
			
			case 3:			
				if(GetTimeFrom(timestamp) > 500)
				{
					timestamp = GetTimeStamp();
					if((SteeringFeedback_0p1V > feedback_prev - 1) && (SteeringFeedback_0p1V < feedback_prev + 1))
						cnt++;
					else
					{
						feedback_prev = SteeringFeedback_0p1V;
						cnt = 0;
					}					
					
					if(cnt == 2)
					{
						_steeringData.SteeringMinVal_0p1V = SteeringFeedback_0p1V - 2;
						PwmUpdate(1, 0);
						PwmUpdate(2, 0);
						cnt = 0;
						step = 4;
						break;
					}
				}				
					
			break;
		
			case 4:		
				PwmUpdate(1, 0);
				PwmUpdate(2, 0);		
				res = 1;
			break;
		}		
	
	return res;
}


