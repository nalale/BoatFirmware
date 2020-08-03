#ifndef _STEERING_FUNC_H_
#define _STEERING_FUNC_H_

typedef struct{
	uint16_t Kp;
	uint16_t Ki;
	uint16_t Kd;
} PID_Struct_t;

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

typedef struct
{
	const uint8_t *PositionVoltageFB_0p1V;
	const uint16_t *fstChannelCurrentFB_0p1A;
	const uint16_t *sndChannelCurrentFB_0p1A;
	void (*PumpControlFunc)(uint8_t Cmd);
}SteeringDependencies_t;

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

	const uint8_t *FeedbackVoltage_0p1V;
	const uint16_t *fstDriverCurrent_0p1A;
	const uint16_t *sndDriverCurrent_0p1A;

	uint16_t DriveCurrent_0p1;
	uint16_t CurrentLimit_0p1A;

	uint16_t kp;
	uint16_t ki;
	uint16_t kd;

	uint8_t SteeringFuncIsReady;
	int16_t UserDemandAngle;

	uint8_t ActivatePump;
	void (*PumpControlFunc)(uint8_t Cmd);

	SteeringFaults_t Faults;
	uint8_t TurnOnDemand;
} SteeringData_t;

uint8_t SteeringInit(SteeringData_t *_steeringData, const PID_Struct_t *PID, const SteeringDependencies_t* Dependencies, uint8_t MinPosLimit_V, uint8_t MaxPosLimit_V, uint16_t CurrentLimit);
uint8_t steeringThread(SteeringData_t *_steeringData);
uint8_t SteeringSetState(SteeringData_t *_steeringData, uint8_t State);
uint8_t steeringSetAngle(SteeringData_t *_steeringData, int16_t TargetAngle);

uint16_t SteeringGetStatus(const SteeringData_t *_steeringData);
uint16_t SteeringGetDriveCurrent(const SteeringData_t *_steeringData);
int16_t SteeringGetFeedBackAngle(const SteeringData_t *_steeringData);
uint16_t SteeringGetFeedBackVoltage(const SteeringData_t *_steeringData);
int16_t SteeringGetState(const SteeringData_t *_steeringData);
int8_t SteeringGetFeedBackPercent(const SteeringData_t *_steeringData);
uint8_t SteeringCtrlPump(const SteeringData_t *handle);

#endif
