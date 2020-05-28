#ifndef _BATTERY_MEASURING_H_
#define _BATTERY_MEASURING_H_



void ModuleStatisticCalculating(BatteryData_t* Handle, const EcuConfig_t *ecuConfig, const int16_t *CellsVoltageArray, const int16_t *CellsTempArray);
void BatteryStatisticCalculating(BatteryData_t *Handle, const BatteryData_t *SourceData, const EcuConfig_t *ecuConfig);
void GetCurrentLimit(const BatteryData_t* handle, const EcuConfig_t *ecuConfig, int16_t* Discharge, int16_t* Charge);


int8_t sysEnergy_Init(uint32_t InitialSystemEnergy_As, uint16_t MaxCellVoltageThreshold_mV, uint16_t MinCellVoltageThreshold_mV);
uint32_t sysEnergy_InitEnergyFromMinUcell(int16_t *OcvTable, uint32_t Voltage, uint32_t TotalCapatity);
uint16_t sysEnergy_CoulombCounting(int16_t Current, uint32_t* CurrentEnergy, uint32_t ModuleCapacity);
uint32_t sysEnergy_EnergyEstimation(uint32_t *CurrentEnergy_As, uint16_t MaxCellVoltage_mV, uint16_t MinCellVoltageThreshold_mV);














#endif


