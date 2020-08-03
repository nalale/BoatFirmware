#ifndef CURRENTSENS_H
#define	CURRENTSENS_H

#include "../MolniaLib/FaultCategory.h"

// NOTE: ������� �� ������, �.�. �� ���� ������� ��� ��� HASS (��. csHassIp)
typedef enum { cstDHAB_S34, cstHASS_50, cstHASS_100, cstHASS_200, cstHASS_300, cstHASS_400, cstNone,

} CurrentSensorType_e;


// *************************** ��� DHAB_S34 ************************************
// ������� ������� ������
#define CUR_SENS_FINE_FACTOR_mv_p_0p1AL   4L
#define CUR_SENS_COARSE_FACTOR_mv_p_0p1AL 1L


// *****************************************************************************


// ̸����� ����
#define CURSENS_DEAD_ZONE_A_0p1                 10  
#define TOP_FAULT_LEVEL                       2600
// ������ ��������� ������� ����������
#define BOT_FAULT_LEVEL                       2400


// ��������� �������� ������� ����

typedef union
{
	struct 
	{
		// ��� ������� ������ ��������� ������� ����
		int32_t CurSensFineVolt_mV;
		// ��� ������� ������ ��������� ������� ����
		int32_t CurSensCoarseVolt_mV;
		// ��� �������� ������� ������
		uint16_t CurSensFineZeroVolt_mV;
		// ��� �������� ������� ������
		uint16_t CurSensCoarseZeroVolt_mV;
	} S34;
	
	struct
	{
		// ������� ���������� � �� ����� Vref � Vout ��� ���������� ���� (�� ����� ����������)
		int32_t VOffsetIdle;
		int32_t Vout;
		int32_t Vref;
	} HASS;
} CURRENT_SENS_STR;




CurrentSensorType_e csGetCurrentSensorType(void);
void csSetCurrentSensorType(CurrentSensorType_e type, uint8_t CWDirection);
void csCalibrateCurrentSensor(void);
int16_t csGetAverageCurrent(void);
uint8_t csGetCircuitState(void);
void csCalibrateIsDone(void);

#endif	/* CURRENTSENS_H */

