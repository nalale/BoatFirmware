#include <stdint.h>

#include "filter.h"
#include "CurrentSens.h"
#include "AdcFunc.h"

#include "Main.h"

int8_t csCWDirection;
CurrentSensorType_e csType;
CURRENT_SENS_STR csEnv;
FILTER_STRUCT csFilter;

// NOTE: ������� ��� � CurrentSensorType_e
const int16_t csHassIp[] = {0, 500, 1000, 2000, 3000, 4000};		// ����������� ���������� ��� � HASS * 10



void csSetCurrentSensorType(CurrentSensorType_e type, uint8_t CWDirection)
{
	csType = type;
	csCWDirection = CWDirection;
	Filter_init(MAX_FILTER_LENGH, 1, &csFilter);
}


void csCalibrateCurrentSensor()
{
	OD.CurrentSensorVoltage[0] = ((OD.A_IN[0]) * 198) >> 7;
	OD.CurrentSensorVoltage[1] = ((OD.A_IN[1]) * 198) >> 7;
	
	if(csType == cstDHAB_S34)
	{
		// ���� ��� ����������
		csEnv.S34.CurSensFineZeroVolt_mV = OD.CurrentSensorVoltage[0];
		csEnv.S34.CurSensCoarseZeroVolt_mV = OD.CurrentSensorVoltage[1];
	}
	else
	{
		csEnv.HASS.VOffsetIdle = Filter(OD.CurrentSensorVoltage[0] - OD.CurrentSensorVoltage[1], &csFilter);
		if(csEnv.HASS.VOffsetIdle > 25)
			csEnv.HASS.VOffsetIdle = 25;
		else if(csEnv.HASS.VOffsetIdle < -25)
			csEnv.HASS.VOffsetIdle = -25;
	}
}


/* �������� �������� ����
 * ����������
 *  �������� ����, ���������� �� 10;
 */
int16_t csGetCurrent()
{
	int16_t current;

	OD.CurrentSensorVoltage[0] = ((OD.A_IN[0]) * 198) >> 7;
	OD.CurrentSensorVoltage[1] = ((OD.A_IN[1]) * 198) >> 7;
	

	if (csType == cstDHAB_S34)
	{		
		csEnv.S34.CurSensFineVolt_mV = OD.CurrentSensorVoltage[0] - (int16_t)csEnv.S34.CurSensFineZeroVolt_mV;
		current = -(csEnv.S34.CurSensFineVolt_mV) / (CUR_SENS_FINE_FACTOR_mv_p_0p1AL);

		if ((current > 500) || (current < -500))
		{
			csEnv.S34.CurSensCoarseVolt_mV = OD.CurrentSensorVoltage[1] - (int16_t)csEnv.S34.CurSensCoarseZeroVolt_mV;
			// �� ���� ����� � �� ������
			current = -csEnv.S34.CurSensCoarseVolt_mV /* /CUR_SENS_COARSE_FACTOR_mv_p_0p1AL*/;
		}
	}
	else	// ������� ���� HASS
	{
		// V = Vref + (0.625 * I / Ipn), ��� Ipn - ����������� ���������� ��� (��� HASS_50 = 50�, ��� HASS_100 = 100� � �.�.)
		// I = (Vout - Vref) * Ipn / 0.625
		csEnv.HASS.Vref = OD.CurrentSensorVoltage[0];
		csEnv.HASS.Vout = OD.CurrentSensorVoltage[1];
		int32_t I = (csEnv.HASS.Vout - csEnv.HASS.Vref + csEnv.HASS.VOffsetIdle) * csHassIp[csType] / 625;
		current = (int16_t)I;
	}

	// ���� ������������������
	if ((current < CURSENS_DEAD_ZONE_A_0p1) && (current > -CURSENS_DEAD_ZONE_A_0p1))
		current = 0;
	

	if(csCWDirection)
	{
		// ���� ������ ���� ���������� ��������. ���� ��� ���������.
		current = -current;
	}
	
	return current;
}


int16_t csGetAverageCurrent()
{
	return Filter(csGetCurrent(), &csFilter);
}

// ���������� ��������� ������������� ���� �������
uint8_t csGetCircuitState()
{
	if (csType == cstDHAB_S34)
	{
//		if(OD.CurrentSensorVoltage[0] > 4900 || OD.CurrentSensorVoltage[1] > 4900)		// ��������� �� +
//			return dctCat_CircuitShortToBattery;
//		else if(OD.CurrentSensorVoltage[0] < 150 || OD.CurrentSensorVoltage[1] < 150)	// ����� ��� ��������� �� -
//			return dctCat_CircuitShortToGroundOrOpen;
//		else if(csEnv.S34.CurSensFineZeroVolt_mV > 2600 || csEnv.S34.CurSensCoarseZeroVolt_mV > 2600)	// ���������� ���� �������������� ������
//			return dctCat_CircuitVoltageAboveThreshold;
//		else if(csEnv.S34.CurSensFineZeroVolt_mV < 2400 || csEnv.S34.CurSensCoarseZeroVolt_mV < 2400)	// ���������� ���� �������������� ������
//			return dctCat_CircuitCurrentBelowThreshold;
	}
	else
	{
		// HASS:
		// Vout_max = 4,6V, Vout_min = 0,4V
		// Vref_max = 2,6V, Vref_min = 2,4V
		if(csEnv.HASS.Vout > 4600 || csEnv.HASS.Vref > 4600)		// ��������� �� +
			return dctCat_CircuitShortToBattery;
		else if(csEnv.HASS.Vout < 400 || csEnv.HASS.Vref < 400)		// ����� ��� ��������� �� -
			return dctCat_CircuitShortToGroundOrOpen;
		else if(csEnv.HASS.Vref > 4000)								// ���������� ���� �������������� ������
			return dctCat_CircuitVoltageAboveThreshold;
		else if(csEnv.HASS.Vref < 1000)								// ���������� ���� �������������� ������
			return dctCat_CircuitVoltageBelowThreshold;
	}
	
	return 0;
}

