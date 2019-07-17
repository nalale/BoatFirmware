#ifndef _BATTERY_MEASURING_H_
#define _BATTERY_MEASURING_H_



void GetCellStatistics(BatteryData_t* bat);
void GetBatteriesStatistic(BatteryData_t *Data, BatteryData_t *BatteryData, uint8_t BatteryCount, uint8_t IsModulesData);
uint32_t GetEnergyFromMinUcell(int16_t *OcvTable, uint16_t Voltage, uint16_t ModuleCapacity);
uint16_t CapacityCulc(int16_t Current, uint32_t* CurrentEnergy, uint16_t ModuleCapacity);
void GetCurrentLimit(int16_t* Discharge, int16_t* Charge);
uint16_t TargetVoltageCulc(uint16_t MinSystemCellVoltage, uint16_t MinModuleCellVoltage);















#endif


