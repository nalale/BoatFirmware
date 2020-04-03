#ifndef _BATTERY_MEASURING_H_
#define _BATTERY_MEASURING_H_



void ModuleStatisticCalculating(BatteryData_t* Handle, const EcuConfig_t *ecuConfig, const int16_t *CellsVoltageArray, const int16_t *CellsTempArray);

void BatteryStatisticCalculating(BatteryData_t *Handle, const BatteryData_t *SourceData, const EcuConfig_t *ecuConfig);

uint32_t GetEnergyFromMinUcell(int16_t *OcvTable, uint32_t Voltage, uint32_t TotalCapatity);
uint16_t StateOfChargeCalculation(int16_t Current, uint32_t* CurrentEnergy, uint16_t ModuleCapacity);
void GetCurrentLimit(const BatteryData_t* handle, const EcuConfig_t *ecuConfig, int16_t* Discharge, int16_t* Charge);

















#endif


