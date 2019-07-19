#include "TLE_6368g2.h"
#include "SpiFunc.h"
#include "TimerFunc.h"
#include "lpc17xx_spi.h"
#include "../BoardDefinitions/MarineEcu_Board.h"

#pragma anon_unions

typedef union
{
  uint16_t value;
  struct 
  {
   
    uint8_t      
          WD_trig               :1,
          WD_off3               :1,
          WD2                   :1,
          WD1                   :1,
          reset2                :1,
          reset1                :1,
          WD_off2               :1,
          Sleep                 :1;  
          
    uint8_t
          T6_control            :1,
          T5_control            :1,
          T4_control            :1,
          T3_control            :1,
          T2_control            :1,
          T1_control            :1,
          empty                 :1,
          WD_off1               :1;
  };
}TLE6368_ControlWord_t;

typedef union
{
  uint16_t value;
  struct 
  {
    uint8_t
            DCDC_Status         :1,
            WD_Error            :1,
            R_Error3            :1,
            R_Error2            :1,
            R_Error1            :1,
            WD_Window           :1,
            RamGood2            :1,
            RamGood1            :1;
            
            
    uint8_t                        
			T6_status           :1,
            T5_status           :1,
            T4_status           :1,
            T3_status           :1,
            T2_status           :1,
            T1_status           :1,
            TempWarm            :1,     
            Error               :1; 
            //Empty               :1;
  };
}TLE6368_StatusWord_t;

static TLE6368_ControlWord_t _controlWorld;
static TLE6368_StatusWord_t _statusWorld;
static uint8_t _is_init = 0;
static uint8_t _power_on = 0;
static uint32_t _init_delay_ts = 0;

uint16_t TLE6368_Protocol(uint16_t *controlword);


void TLE_Init(void)
{
	_controlWorld.Sleep = 0;		// 1 - Normal, 0 - Sleep
	_controlWorld.WD_off1 = 1;		// Window watchdog func
	_controlWorld.WD_off2 = 0;		// 101 - OFF, 010 - ON
	_controlWorld.WD_off3 = 1;
	_controlWorld.reset1 = 0;		// reset delay valid at warm start
	_controlWorld.reset2 = 0;		// 00 - 64ms, 10 - 32 ms, 01 - 16ms, 00 - 8ms
	_controlWorld.T1_control = 0;//1;
	_controlWorld.T2_control = 0;//1;
	_controlWorld.T3_control = 0;//1;
	_controlWorld.T4_control = 0;//1;
	_controlWorld.T5_control = 0;//1;
	_controlWorld.T6_control = 0;
	_controlWorld.WD1 = 0;
	_controlWorld.WD2 = 0;
	_controlWorld.WD_trig = 0;

	_is_init = 1;
	_init_delay_ts = GetTimeStamp();
}

void TLE_Proc()
{
	// устройство корректное запускает контроллер только через определенное время после подачи питания
	if(!_is_init || GetTimeFrom(_init_delay_ts) < 200)
		return;
	
	_controlWorld.Sleep = (_power_on)? 1 : 0;		// 1 - Normal, 0 - Sleep

	_statusWorld.value = TLE6368_Protocol(&_controlWorld.value);

}

void TLE_GoToSleep(void)
{
	_power_on = 0;		// 1 - Normal, 0 - Sleep
	
	_is_init = 1;
	
//	TLE6368_Protocol(&_controlWorld.value);
}

void TLE_GoToPowerSupply()
{
	_power_on = 1;
	_controlWorld.WD_trig = 0;
	
	_is_init = 1;
	
//	TLE6368_Protocol(&_controlWorld.value);
}

Tle6368FaultList_e TLE_GetCircuitState(void)
{
	Tle6368FaultList_e result = TLE_F_NO_FAULT;
	if(_statusWorld.Error && _is_init)
	{
		if(_statusWorld.TempWarm)
			result = TLE_F_OVER_TEMP;
		else if(_statusWorld.WD_Error)
			result = TLE_F_WD_ERROR;
		else if((!_statusWorld.T1_status && _controlWorld.T1_control) ||
				(!_statusWorld.T2_status && _controlWorld.T2_control) ||
				(!_statusWorld.T3_status && _controlWorld.T3_control) ||
				(!_statusWorld.T4_status && _controlWorld.T4_control))
			result = TLE_F_OUT_SHORTED;
		else if(!_statusWorld.RamGood1 || !_statusWorld.RamGood2)
			result = TLE_F_COLD_START;
		else
			result = TLE_F_GENERAL_ERROR;
	}
	return result;
}

uint16_t TLE6368_Protocol(uint16_t *controlword)
{
	if(!_is_init)
		return 0;
	
	uint16_t status = 0xffff;

	SET_CS_OUT(0);
	SpiReadWrite((uint8_t*)controlword, 2, (uint8_t*)&status);
	SET_CS_OUT(1);
	return status;
}

//uint16_t TLE6368_ReadStatusWord(void)
//{
//  uint16_t status = 0xffff;
//  SpiReadWrite(NULL, 2, (uint8_t*)&rx);
//  return rx;
//}

