#ifndef _TRIM_H_
#define _TRIM_H_

#include "../BoardDefinitions/MarineEcu_Board.h"

typedef enum
{
	paramTrim_VoltageFB_0p1V,
	paramTrim_Position,
	paramTrim_Status,
	paramTrim_Fault,
	paramTrim_Cmd,
} TrimParameters_e;

typedef enum
{
	cmdTrim_Stop = 0,
	cmdTrim_Down,
	cmdTrim_Up,
	
} TrimCmd_e;

typedef struct
{
	uint8_t Value;

	uint8_t
		F_Position	:	1,
		F_Feedback	:	1,
		dummy1		:	6;
} TrimFaults_t;

typedef enum
{
	TRIM_F_NONE,
	TRIM_F_POSITION,
	TRIM_F_FEEDBACK,
}TrimFaults_e;

typedef enum
{
	stTrim_Disable,
	stTrim_Enable,
	stTrim_Warning,
	stTrim_Fault,
} TrimStatus_e;

typedef struct
{
	const uint8_t *FeedBack_mV;
	uint8_t MovCmd;	
	uint8_t Position;
	uint16_t VoltagePosTable_0p1V[4];
	uint16_t DriveUpperLimit_0p1V;
	TrimFaults_t Faults;
	TrimStatus_e Status;

} TrimData_t;

uint8_t TrimInit(TrimData_t *TrimData, const uint8_t* pFeedback_0p1V, uint16_t MaxFeedback_0p1V, uint16_t MinFeedback_0p1V, uint16_t DriveUpperLimitFB_0p1V);
uint8_t TrimProc(TrimData_t *TrimData, int16_t ShaftVelocity);

uint8_t TrimSetCmd(TrimData_t *TrimData, uint8_t cmdUp, uint8_t cmdDown);
uint16_t TrimGetParameter(const TrimData_t *TrimData, TrimParameters_e Parameter);









#endif

