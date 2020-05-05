#ifndef _TRIM_H_
#define _TRIM_H_

#include "../BoardDefinitions/MarineEcu_Board.h"

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

typedef struct
{
	uint8_t *FeedBack_mV;
	uint8_t MovCmd;	
	uint8_t Position;
	uint16_t MaxVoltage_0p1V;
	uint16_t MinVoltage_0p1V;
	TrimFaults_t Faults;

} TrimData_t;

uint8_t TrimInit(TrimData_t *TrimData, uint8_t* pFeedback_0p1V, uint16_t MaxFeedback_0p1V, uint16_t MinFeedback_0p1V);
uint8_t TrimProc(TrimData_t *TrimData, uint8_t cmdUp, uint8_t cmdDown);
uint8_t TrimGetState(const TrimData_t *TrimData);










#endif

