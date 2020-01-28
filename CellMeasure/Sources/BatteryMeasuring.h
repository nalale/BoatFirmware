#ifndef _BATTERY_MEASURING_H_
#define _BATTERY_MEASURING_H_



void ModuleStatisticCalculating(BatteryData_t* Handle, const EcuConfig_t *ecuConfig, const int16_t *CellsVoltageArray, const int16_t *CellsTempArray);

void BatteryStatisticCalculating(BatteryData_t *Handle, const BatteryData_t *SourceData, const EcuConfig_t *ecuConfig);

uint32_t GetEnergyFromMinUcell(int16_t *OcvTable, uint16_t Voltage, uint16_t ModuleCapacity);
uint16_t CapacityCulc(int16_t Current, uint32_t* CurrentEnergy, uint16_t ModuleCapacity);
void GetCurrentLimit(int16_t* Discharge, int16_t* Charge);
uint16_t TargetVoltageCulc(uint16_t MinSystemCellVoltage, uint16_t MinModuleCellVoltage);















#endif


