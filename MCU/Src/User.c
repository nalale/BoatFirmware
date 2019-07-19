#include "User.h"
#include "TrimFunc.h"
#include "SteeringFunc.h"
#include "mVcu_Ecu.h"

#include "TimerFunc.h"
#include "CanFunc.h"
#include "PwmFunc.h"
#include "I2cFunc.h"
#include "AdcFunc.h"
#include "UartFunc.h"
#include "SpiFunc.h"

#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_can.h"

#include "../Libs/Btn8982.h"
#include "../MolniaLib/MF_Tools.h"
#include "../Libs/filter.h"

#include "../BoardDefinitions/MarineEcu_Board.h"

FILTER_STRUCT fltAcc;

void ecuInit(ObjectDictionary_t *dictionary);
void HardwareInit(void);
void PortInit(void);
uint8_t DrainSupply(void);
uint8_t TemperatureControl(void);

void AppInit(ObjectDictionary_t *dictionary)
{
    HardwareInit();
    ecuInit(dictionary);          
}

typedef struct
{
	int16_t FirstChannel[4];
	int16_t SecondChannel[4];
} AccPedalData_t;

AccPedalData_t pedalData;

void PllInit()
{
    
//  LPC_SC->PLL0CFG   = PLL0CFG_Val;      /* configure PLL0                     */
//  LPC_SC->PLL0FEED  = 0xAA;
//  LPC_SC->PLL0FEED  = 0x55;

//  LPC_SC->PLL0CON   = 0x01;             /* PLL0 Enable                        */
//  LPC_SC->PLL0FEED  = 0xAA;
//  LPC_SC->PLL0FEED  = 0x55;
//  while (!(LPC_SC->PLL0STAT & (1<<26)));/* Wait for PLOCK0                    */

//  LPC_SC->PLL0CON   = 0x03;             /* PLL0 Enable & Connect              */
//  LPC_SC->PLL0FEED  = 0xAA;
//  LPC_SC->PLL0FEED  = 0x55;
//  while ((LPC_SC->PLL0STAT & ((1<<25) | (1<<24))) != ((1<<25) | (1<<24)));  /* Wait for PLLC0_STAT & PLLE0_STAT */

//  LPC_SC->PLL1CFG   = PLL1CFG_Val;
//  LPC_SC->PLL1FEED  = 0xAA;
//  LPC_SC->PLL1FEED  = 0x55;

//  LPC_SC->PLL1CON   = 0x01;             /* PLL1 Enable                        */
//  LPC_SC->PLL1FEED  = 0xAA;
//  LPC_SC->PLL1FEED  = 0x55;
//  while (!(LPC_SC->PLL1STAT & (1<<10)));/* Wait for PLOCK1                    */

//  LPC_SC->PLL1CON   = 0x03;             /* PLL1 Enable & Connect              */
//  LPC_SC->PLL1FEED  = 0xAA;
//  LPC_SC->PLL1FEED  = 0x55;
//  while ((LPC_SC->PLL1STAT & ((1<< 9) | (1<< 8))) != ((1<< 9) | (1<< 8)));  /* Wait for PLLC1_STAT & PLLE1_STAT */
  
  
}

void HardwareInit(void)
{
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0 | CLKPWR_PCONP_PCPWM1 | CLKPWR_PCONP_PCSSP1 | 
                        CLKPWR_PCONP_PCAN1 | CLKPWR_PCONP_PCAN2 | CLKPWR_PCONP_PCGPIO | CLKPWR_PCONP_PCI2C2 |
						CLKPWR_PCONP_PCUART0,
                        ENABLE);
    PortInit();
    Tmr_Init(1000);
    Can_Init(1, 1);
    Pwm_Init(10000);
    Adc_Init();
    I2c_Init();
	Uart_Init(0, 9600);
	
	Spi_Init(DATABIT_16);
	
	NVIC_EnableIRQ(CAN_IRQn);
}



void PortInit(void)
{    
	FIO_SetDir(0, 0x40, DIR_OUT);
	SET_CS_OUT(1);
	
    FIO_SetDir(1, 0xFFFFFFFF, DIR_OUT);
    //FIO_SetValue(1, 0xFFFFFFFF);
	SET_C_OUT5(1);
	SET_C_OUT6(1);
	SET_C_OUT7(1);
	SET_C_OUT8(1);
	SET_C_OUT9(1);
	SET_C_OUT10(1);
	
	SET_PU_D_IN1(0);
	SET_PU_D_IN2(0);
	
    
    FIO_SetDir(2, 0xFFFFFFFF, DIR_OUT);
    SET_A_OUT1_EN(0);
	SET_A_OUT2_EN(0);
	SET_A_OUT3_EN(0);
	SET_A_OUT4_EN(0);
    
    FIO_SetDir(4, 0xFFFFFFFF, DIR_OUT);
    //FIO_SetValue(4, 0x0);
	SET_PU_D_IN3(0);
	SET_PU_D_IN4(0);
	
	FIO_SetDir(1, 0xC713, DIR_IN);
	FIO_SetDir(0, (1 << 30) | (1 << 29), DIR_IN);
	
}

void InverterPower(uint8_t Cmd)
{
	SetDOutput(6, Cmd);
}

void SteeringPumpPower(uint8_t Cmd)
{
	Cmd = (Cmd >= 1)? 255 : 0;
	
	btnSetOutputLevel(2, Cmd);
}

void AccPedalInit(uint8_t MaxV, uint8_t NeuV)
{
	pedalData.FirstChannel[0] = MaxV;	
	pedalData.FirstChannel[1] = NeuV;
	pedalData.FirstChannel[2] = 100;
	pedalData.FirstChannel[3] = 0;	
	
	pedalData.SecondChannel[0] = 0;	
	pedalData.SecondChannel[1] = 0;
	pedalData.SecondChannel[2] = 0;
	pedalData.SecondChannel[3] = 0;
	
	Filter_init(150, 10, &fltAcc);
}

uint8_t GetAccelerationPosition(uint8_t sensor_voltage_0p1, uint8_t sensor2_voltage_0p1)
{
    int16_t result = 0;    
	
	uint8_t ch_1 = Filter(sensor_voltage_0p1, &fltAcc);
    result = interpol(pedalData.FirstChannel, 2, ch_1);
    return result;
}

int8_t GetDriveDirection(uint8_t sensor_voltage_0p1, uint8_t sensor2_voltage_0p1)
{
	EcuConfig_t cfgEcu = GetConfigInstance();
    return ((sensor_voltage_0p1 > cfgEcu.AccPedalFstCh_0V - 1) && (sensor_voltage_0p1 < cfgEcu.AccPedalFstCh_0V + 1))? 0 : 1;
}

int16_t GetTargetTorque(uint8_t AccPosition, int8_t GearSelected)
{
    uint16_t result = GearSelected * OD.MaxMotorTorque * AccPosition / 100;
    
    return result;
}

uint8_t InverterControl(uint8_t LockupEnable, uint8_t BatContactorClose, uint8_t IsTimeout, uint8_t Cmd)
{    
    if(LockupEnable || IsTimeout || !BatContactorClose || (Cmd == 0))
        return 0;
    else
        return 1;   
}

uint8_t BatteryControl(uint8_t Timeout)
{
	if(Timeout)
		return 0;
	
	return 1;
}



uint8_t ChargersCircuitOn(uint8_t Cmd)
{
	if(Cmd == 1)
		btnSetOutputLevel(3, 255);
	else if(Cmd == 0)
		btnSetOutputLevel(3, 0);
	
	return 0;
}

uint8_t CheckChargingCond(uint8_t TerminalState, uint8_t BatteryState)
{
	static uint32_t _connectedTimeStamp = 0;
	static uint8_t _chargingStop = 0;
	
	uint8_t _targetCurrent = 0;
	if(TerminalState == 1)
	{
		if(BatteryState == OD.SB.BatteryIsOperate && !_chargingStop)
		{
			if(GetTimeFrom(_connectedTimeStamp) > 4000)
			{
				EcuConfig_t cfgEcu = GetConfigInstance();
				
				uint8_t locMaxCurrent = cfgEcu.MaxChargingCurrent_A / cfgEcu.ChargersNumber;
				uint8_t locCCL = -OD.BatteryDataRx.CCL / cfgEcu.ChargersNumber;
				ChargersCircuitOn(1);
				
				_targetCurrent = (-OD.BatteryDataRx.CCL < cfgEcu.MaxChargingCurrent_A)? locCCL : locMaxCurrent;
				if(_targetCurrent <= 1)
				{
					_targetCurrent = 0;
					_chargingStop = 1;
					ChargersCircuitOn(0);
				}
			}
		}
		else
			_connectedTimeStamp = GetTimeStamp();
	}
	else
	{		
		_chargingStop = 0;
		ChargersCircuitOn(0);
	}
	
	return _targetCurrent;
}

uint8_t SystemThreat()
{	
	uint8_t _inverterEnabled = 0;

	if(OD.MovControlDataRx.AccPosition > 1 && OD.InvertorDataRx.InverterState != VSM_FaultState)
		_inverterEnabled = 1;
	else
		_inverterEnabled = 0;
	
	// Управление состоянием инвертора
	OD.TractionData.InverterEnable = InverterControl(OD.InvertorDataRx.LockupIsEnable, OD.SB.BatteryIsOperate, OD.FaultsBits.SteeringTimeout, _inverterEnabled);
	OD.BatteryReqState = BatteryControl(OD.FaultsBits.InverterFault || OD.FaultsBits.InverterTimeout);
	OD.TargetSteeringAngle = HelmGetTargetAngle(&OD.HelmData);

	InverterPower(OD.SB.cmdInverterPowerSupply);
	SteeringPumpPower(OD.SB.cmdSteeringPump);

	TrimProc(&OD.TrimDataRx, OD.SB.cmdTrimUp, OD.SB.cmdTrimDown);		// Управление тримом
	SteeringProc(&OD.SteeringData, OD.TargetSteeringAngle);				// Управление рулевой колонкой

	OD.SB.cmdDrainPumpOn = DrainSupply();								// Управление дренажными помпами
	TemperatureControl();
	
	return 0;
}

uint8_t TemperatureControl()
{
	static uint32_t _coolingOffDelay = 0;

	if(OD.InvertorDataRx.InverterIsEnable)
	{
		EcuConfig_t cfgEcu = GetConfigInstance();

		OD.SB.cmdMotorPumpCooling = 1;

		if((OD.InvertorDataRx.InverterTemperature > cfgEcu.InvCoolingOn) || (OD.InvertorDataRx.MotorTemperature > cfgEcu.MotorCoolingOn))
			OD.SB.cmdHeatsinkPump = 1;
		else if((OD.InvertorDataRx.InverterTemperature < cfgEcu.InvCoolingOn - 15) || (OD.InvertorDataRx.MotorTemperature < cfgEcu.MotorCoolingOn - 15))
			OD.SB.cmdHeatsinkPump = 0;

		_coolingOffDelay = GetTimeStamp();

	}
	else
	{
		if(GetTimeFrom(_coolingOffDelay) > 15000)
		{
			OD.SB.cmdMotorPumpCooling = 0;
			OD.SB.cmdHeatsinkPump = 0;

			return 1;
		}
	}

	return 0;
}

uint8_t DrainSupply()
{
	static uint32_t _waterSwitch1OnDelay = 0; 
	
	if(OD.SB.stWaterSwitch1 || OD.SB.stWaterSwitch2 || OD.SB.stManualDrainSwitch)
	{
		if(GetTimeFrom(_waterSwitch1OnDelay) > 2000)
		{
			return 1;
		}
	}
	else
	{
		_waterSwitch1OnDelay = GetTimeStamp();
		return 0;
	}
	
	return 0;
}

