#ifndef _STEERING_FUNC_H_
#define _STEERING_FUNC_H_


typedef struct
{
	int16_t TargetAngle;   
    int16_t ActualAngle;
    int16_t SteeringSpeed_rads;
    uint8_t NumSteeringRev;
    
	uint16_t LeftSteeringCurrent;
	uint16_t RightSteeringCurrent;
	int16_t SteeringTotalCurrent_0p1;
	
	uint16_t Feedback_V;
	
    uint8_t
        EndStopLeft_Ack     :   1,
        EndStopRight_Ack    :   1,
        SetZeroRegBrake_Ack	:   1,
		dummy				:	5;	
		
} Steering_Data_Rx_t;



uint8_t steerThread(Steering_Data_Rx_t *data);
uint8_t steerInit(uint8_t MinPosLimit_V, uint8_t MaxPosLimit_V);
uint8_t steerGetBrakingValue(int16_t ActualSpeed, int16_t* BrakeSpeedTable);
uint16_t steerGetDriveCurrent(void);
int16_t steerGetFeedBackAngle(void);


#endif
