#ifndef _STEERING_FUNC_H_
#define _STEERING_FUNC_H_


typedef struct
{
	int16_t TargetAngle;   
    int16_t ActualAngle;
    int16_t SteeringSpeed_rads;
    uint8_t NumSteeringRev;

    uint8_t
        EndStopLeft_Ack     :   1,
        EndStopRight_Ack    :   1,
        SetZeroRegBrake_Ack	:   1,
		HelmIsReady			:	1,
		dummy				:	4;
		
} Helm_Data_Rx_t;

typedef struct
{
	uint8_t Value;
	uint8_t
		F_Feedback	:	1,
		F_Position	:	1,
		F_Current1	:	1,
		F_Current2	:	1,
		F_dummy		:	3;
}SteeringFaults_t;

typedef enum
{
	STEERING_F_NONE = 0,
	STEERING_F_FEEDBACK,
	STEERING_F_POSITION,
	STEERING_F_CURRENT1,
	STEERING_F_CURRENT2,
}STEERING_FAULTS_e;

typedef struct
{
	uint8_t SteeringMinVal_0p1V;
	uint8_t SteeringMaxVal_0p1V;

	int16_t FeedBackAngle;
	int16_t FeedbackVoltageRanges[4];

	uint8_t *FeedbackVoltage_0p1V;
	uint16_t *fstDriverCurrent_0p1A;
	uint16_t *sndDriverCurrent_0p1A;

	uint16_t DriveCurrent_0p1;
	uint16_t CurrentLimit_0p1A;

	uint8_t kp;
	uint8_t ki;
	uint8_t kd;

	uint8_t SteeringFuncIsReady;

	SteeringFaults_t Faults;

} SteeringData_t;

uint8_t SteeringInit(SteeringData_t *_steeringData, uint8_t *FeedbackVoltage_0p1V, uint8_t MinPosLimit_V, uint8_t MaxPosLimit_V, uint16_t *fstDriverCurrent, uint16_t *sndDriverCurrent, uint16_t CurrentLimit);
uint8_t SteeringProc(SteeringData_t *_steeringData, int16_t TargetAngle);

uint16_t SteeringGetDriveCurrent(const SteeringData_t *_steeringData);
int16_t SteeringGetFeedBackAngle(const SteeringData_t *_steeringData);
int16_t SteeringGetState(const SteeringData_t *_steeringData);

uint8_t steerGetBrakingValue(int16_t ActualSpeed, int16_t* BrakeSpeedTable);

uint16_t HelmGetTargetAngle(Helm_Data_Rx_t *HelmData);
#endif
