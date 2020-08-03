#ifndef CURRENTSENS_H
#define	CURRENTSENS_H

#include "../MolniaLib/FaultCategory.h"

// NOTE: Порядок не менять, т.к. от него зависит ток для HASS (см. csHassIp)
typedef enum { cstDHAB_S34, cstHASS_50, cstHASS_100, cstHASS_200, cstHASS_300, cstHASS_400, cstNone,

} CurrentSensorType_e;


// *************************** Для DHAB_S34 ************************************
// Масштаб точного канала
#define CUR_SENS_FINE_FACTOR_mv_p_0p1AL   4L
#define CUR_SENS_COARSE_FACTOR_mv_p_0p1AL 1L


// *****************************************************************************


// Мёртвая зона
#define CURSENS_DEAD_ZONE_A_0p1                 10  
#define TOP_FAULT_LEVEL                       2600
// Нижний аварийный уровень калибровки
#define BOT_FAULT_LEVEL                       2400


// Структура настроек датчика тока

typedef union
{
	struct 
	{
		// Код точного канала измерения датчика тока
		int32_t CurSensFineVolt_mV;
		// Код грубого канала измерения датчика тока
		int32_t CurSensCoarseVolt_mV;
		// Код смещения точного канала
		uint16_t CurSensFineZeroVolt_mV;
		// Код смещения точного канала
		uint16_t CurSensCoarseZeroVolt_mV;
	} S34;
	
	struct
	{
		// Разница напряжений в мВ между Vref и Vout при отсутствии тока (во время калибровки)
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

