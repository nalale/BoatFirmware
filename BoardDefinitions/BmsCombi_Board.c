#include "BmsCombi_Board.h"
#include "../MolniaLib/MF_Tools.h"

#include <lpc17xx_gpio.h>
#include <lpc17xx_pinsel.h>

// ECU Variables
static uint16_t _ecuPowerSupply = 0;
static uint32_t _boardIOStatus = 0;

FILTER_STRUCT fltVoltage;

void IO_Init()
{
	GPIO_SetDir(0, KEY, OUT_DIR);
	GPIO_SetDir(2, D_OUT1, OUT_DIR);
	GPIO_SetDir(2, D_OUT2, OUT_DIR);
	GPIO_SetDir(0, D_OUT3, OUT_DIR);
	GPIO_SetDir(0, D_OUT4, OUT_DIR);
	
	GPIO_SetDir(1, CS1_OUT, OUT_DIR);
	GPIO_SetDir(1, CS2_OUT, OUT_DIR);
	
	GPIO_SetDir(2, D_IN1, IN_DIR);
	GPIO_SetDir(2, D_IN2, IN_DIR);
	GPIO_SetDir(2, D_IN3, IN_DIR);
	GPIO_SetDir(2, D_IN4, IN_DIR);
	
	GPIO_SetDir(2, D_OUT1_STATUS, IN_DIR);
	GPIO_SetDir(0, D_OUT2_STATUS, IN_DIR);
	GPIO_SetDir(0, D_OUT3_STATUS, IN_DIR);
	GPIO_SetDir(0, D_OUT4_STATUS, IN_DIR);
	
	GPIO_SetValue(1, CS1_OUT | CS2_OUT);
	
	SET_D_OUT1(GPIO_LOW);
	SET_D_OUT2(GPIO_LOW);
	SET_D_OUT3(GPIO_LOW);
	SET_D_OUT4(GPIO_LOW);    
}

void gpio_ltc6804_cs_set(uint32_t cs_num, uint8_t state)
{
	if(state == 0)
		GPIO_ClearValue(1, cs_num);//CS1_OUT); //CS2_OUT
	else
		GPIO_SetValue(1, cs_num);	
}

uint32_t gpio_get_states()
{
	uint8_t b = 0;
    uint32_t tmp = 0;

    // Входы
    if (GET_D_IN1) SET_BIT(tmp, b);
    b++;
    if (GET_D_IN2) SET_BIT(tmp, b);
    b++;
	if (GET_D_IN3) SET_BIT(tmp, b);
    b++;
    if (GET_D_IN4) SET_BIT(tmp, b);
    b++;	

    b = 16;
    // Выходы
    if (GET_D_OUT1) SET_BIT(tmp, b);
    b++;
    if (GET_D_OUT2) SET_BIT(tmp, b);
    b++;
	if (GET_D_OUT3) SET_BIT(tmp, b);
    b++;
    if (GET_D_OUT4) SET_BIT(tmp, b);
    b++;

    return tmp;
}

void boardThread()
{
	uint16_t voltage_mV = Filter((GetVoltageValue(A_CHNL_KEY) * 11) , &fltVoltage);
	 _ecuPowerSupply = voltage_mV / 100;
	
	_boardIOStatus = gpio_get_states();
}

uint16_t boardBMSCombi_GetVoltage()
{
	return _ecuPowerSupply;
}

uint32_t boardBMSCombi_GetDiscreteIO() {
    return _boardIOStatus;
}

