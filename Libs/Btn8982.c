#include "../BoardDefinitions/MarineEcu_Board.h"
#include "Btn8982.h"
#include "PwmFunc.h"
#include "TimerFunc.h"
#include "AdcFunc.h"
#include "filter.h"

#include <stdint.h>


// �������� ������ �� ������ ����� ��������� ���� � ����������
// ������ �� ���������� ����. ���������� ������, ����� �������� ������ �� ������ ������� ��� (target = 0).

#define RES_IS  500//1000      // Ohm 
#define dK      195       // *100
#define Iis_lim	5000	//uA

typedef struct {
    uint8_t
    Fault : 1,
    IsInit : 1,
	CalibProc	:	1,
    dummy1 : 5;

    Btn8982FaultList_e FaultType;
    uint16_t Current;
    uint16_t CurrentOffset;
    uint8_t TargetPwm;
    
    uint16_t CurrentThreshold;
	uint8_t MeasuringChannel;
	
	uint32_t CalibrationTms;
} btnData_t;

static FILTER_STRUCT csFilter[DRIVER_NUM];
static btnData_t btnData[DRIVER_NUM];


static void _btnMonitor(uint8_t Channel);
static uint16_t _loadCurrentCalculate(uint16_t Current_IS_uA, uint16_t Current_Offset_uA);
static void _resetChannel(uint8_t Channel);

void btnInit(uint8_t Number, uint8_t MeasuringChannel, uint16_t CurrentTreshold_0p1A)
{
    btnData[Number].CurrentThreshold = CurrentTreshold_0p1A;
    btnData[Number].Fault = 0;
    btnData[Number].FaultType = BTN_F_NO_FAULT;
    btnData[Number].TargetPwm = 0;    
    btnData[Number].Current = 0;
    btnData[Number].CurrentOffset = 0;    
	
	// ������������� ���������������� ������ Pwm
	Pwm_Ch_Init(Number);
	// ������ ���������� ������ btn8982
	btnInhibit(Number, 1);
	btnData[Number].MeasuringChannel = MeasuringChannel;
	
	Filter_init(100, 5, &csFilter[Number]);
}

uint8_t btnCalibrate(uint8_t Channel) {
    if (Channel >= DRIVER_NUM)
        return 0;
	
	if(!btnData[Channel].CalibProc)
		btnData[Channel].CalibrationTms = GetTimeStamp();
	else if(GetTimeFrom(btnData[Channel].CalibrationTms) > 1000)
	{
		btnData[Channel].IsInit = 1;
		return 1;
	}
	
	btnData[Channel].CalibProc = 1;
	
	uint8_t measuring_channel = btnData[Channel].MeasuringChannel;
    btnData[Channel].CurrentOffset = Filter(GetVoltageValue(measuring_channel) * 1000 / RES_IS, &(csFilter[Channel])); // uA   (mV*1000[uV])/(Ohm) => uA
		
	return 0;
	
//	if(btnData[Channel].CurrentOffset > 250 || btnData[Channel].CurrentOffset < 100)
//	{
//		btnData[Channel].FaultType = BTN_F_CURRENT_OFFSET_NVALID;
//        btnData[Channel].TargetPwm = 0;
//        btnInhibit(Channel, 0);
//        btnData[Channel].Fault = 1;
//	}
}

void btnInhibit(uint8_t num, uint8_t state) {
    switch (num) {
        case 0:
            SET_D_OUT1_EN(state);
            break;
        case 1:
            SET_D_OUT2_EN(state);
            break;
		case 2:
			SET_D_OUT3_EN(state);
			break;
		case 3:
			SET_D_OUT4_EN(state);
			break;
    }
}

void btnProc() {
    for (uint8_t i = 0; i < DRIVER_NUM; i++)
        _btnMonitor(i);
}

static void _btnMonitor(uint8_t Channel) 
{
    static uint32_t circOpenTimeStamp = 0;

    if (Channel >= DRIVER_NUM || !btnData[Channel].IsInit)
        return;

	uint8_t measuring_channel = btnData[Channel].MeasuringChannel;
	
    uint16_t voltage = (measuring_channel != 0xFF)? GetVoltageValue(measuring_channel) : 0;
    uint16_t Current_IS_uA = Filter(voltage * 1000 / RES_IS, &(csFilter[Channel]));	//voltage * 1000 / RES_IS;
	
	btnData[Channel].Current = _loadCurrentCalculate(Current_IS_uA, btnData[Channel].CurrentOffset);
	
	if(btnData[Channel].Fault > 0)
		return;

    if (btnData[Channel].TargetPwm > (uint8_t)30 || btnData[Channel].Current < (uint16_t)10)
        circOpenTimeStamp = GetTimeStamp();

    // ��������� ������
    if (btnData[Channel].TargetPwm == 0 && Current_IS_uA > Iis_lim)// voltage > 2925)
    {
		btnData[Channel].Fault = 1;
		btnData[Channel].FaultType = BTN_F_SHORT_TO_BAT;     
		_resetChannel(Channel);
        btnInhibit(Channel, 0);        
    }
    else if (btnData[Channel].TargetPwm > 2 && Current_IS_uA > Iis_lim) //voltage > 2925)
    {
		btnData[Channel].Fault = 1;
		btnData[Channel].FaultType = BTN_F_SHORT_TO_GROUND;   		
		_resetChannel(Channel);	      
        btnInhibit(Channel, 0);
        
    }
    else if(btnData[Channel].Current > btnData[Channel].CurrentThreshold)
    {        
		btnData[Channel].Fault = 1;
		btnData[Channel].FaultType = BTN_F_CURRENT_ABOVE_THRESHOLD;	
		_resetChannel(Channel);
		
        //btnInhibit(Channel, 0);
    }
    else if (GetTimeFrom(circOpenTimeStamp) > 2000)
        btnData[Channel].FaultType = BTN_F_CIRCUIT_OPEN;
//
//    if(!btnData[Channel].Fault)
//    {
//        // ������ ����
//        btnData[Channel].Current = _loadCurrentCalculate(Current_IS_uA, btnData[Channel].CurrentOffset);
//	}
//    else
//        btnData[Channel].Current = 0;
    
}

void btnSetOutputLevel(uint8_t Channel, uint8_t Level) 
{
    if (Channel >= DRIVER_NUM)
        return;   
    
	// If channel in op condition set target level
	// If channel in fault condition reset output, 
	// if channel in fault condition and target level is 0, then restore output	
	if(btnData[Channel].Fault)
	{
		if(btnData[Channel].FaultType == BTN_F_CURRENT_ABOVE_THRESHOLD && Level == 0)
		{
			btnData[Channel].Fault = 0;
			btnData[Channel].FaultType = BTN_F_NO_FAULT;
		}
		Level = 0;
	}
	
	btnData[Channel].TargetPwm = Level;
   	
    PwmUpdate(Channel, btnData[Channel].TargetPwm);
}

uint8_t btnGetOutputLevel(uint8_t Channel)
{
    if (Channel >= DRIVER_NUM)
        return UINT8_MAX;
    
    return btnData[Channel].TargetPwm;
}

uint16_t btnGetCurrent(uint8_t Channel) {
    if (Channel >= DRIVER_NUM)
        return UINT16_MAX;

    return btnData[Channel].Current;
}

Btn8982FaultList_e btnGetCircuitState(uint8_t Channel) {
    if (Channel >= DRIVER_NUM)
        return BTN_F_NO_FAULT;

    return btnData[Channel].FaultType;
}

void btnClearFaults(uint8_t Channel)
{
    btnData[Channel].Fault = 0;
    btnData[Channel].FaultType = BTN_F_NO_FAULT;
}

void _resetChannel(uint8_t Channel)
{
	PwmUpdate(Channel, 0);
}

uint16_t _loadCurrentCalculate(uint16_t Current_IS_uA, uint16_t Current_Offset_uA)
{
	uint16_t _load_current_mA = 0;
	
	if(Current_IS_uA > Current_Offset_uA)
		_load_current_mA = (dK * (Current_IS_uA - Current_Offset_uA)) / 1000;  // 0p1A, dKe+3 * Ae-6 / 1e+5 =>  0p1A   
	
	return _load_current_mA;
}


