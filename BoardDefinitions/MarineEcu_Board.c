#include <stdint.h>

#include "../Libs/filter.h"

#include "../BoardDefinitions/MarineEcu_Board.h"
#include "../MolniaLib/MF_Tools.h"

#include "AdcFunc.h"

void SetDOutput(uint8_t num, uint8_t state)
{
	// Если выходом запрещено управлять через логические выходы
	/*if((1 << num) & LOGIC_OUT_DISABLE)
		return;
	*/
	if(num < 4)
		return;
	
	switch(num)
	{
		case 4:
			SET_C_OUT5(state);
			break;
		case 5:
			SET_C_OUT6(state);
			break;
		case 6:
			SET_C_OUT7(state);
			break;
		case 7:
			SET_C_OUT8(state);
			break;
		case 8:
			SET_C_OUT9(state);
			break;
		case 9:
			SET_C_OUT10(state);
			break;
	}
}

/*
 *  Функция возвращает состояние дискретных входов, выходов
 */
uint32_t boardMarineECU_GetDiscreteIO() {
    uint8_t b = 0;
    uint32_t tmp = 0;

    // Входы
    if (D_IN1) SET_BIT(tmp, b);
    b++;
    if (D_IN2) SET_BIT(tmp, b);
    b++;
	if (D_IN3) SET_BIT(tmp, b);
    b++;
    if (D_IN4) SET_BIT(tmp, b);
    b++;	
	if (D_IN5) SET_BIT(tmp, b);
    b++;
    if (D_IN6) SET_BIT(tmp, b);
    b++;
	if (D_IN7) SET_BIT(tmp, b);
    b++;
    if (D_IN8) SET_BIT(tmp, b);
    b++;
	if (D_IN9) SET_BIT(tmp, b);
    b++;
    if (D_IN10) SET_BIT(tmp, b);
    b++;

    b = 16;
    // Выходы
    if (C_OUT5_STATE) SET_BIT(tmp, b);
    b++;
    if (C_OUT6_STATE) SET_BIT(tmp, b);
    b++;
	if (C_OUT7_STATE) SET_BIT(tmp, b);
    b++;
    if (C_OUT8_STATE) SET_BIT(tmp, b);
    b++;
	if (C_OUT9_STATE) SET_BIT(tmp, b);
    b++;
    if (C_OUT10_STATE) SET_BIT(tmp, b);
    b++;

    return tmp;
}

static void boardPortInit(void)
{
	FIO_SetDir(0, 0x40, DIR_OUT);
	SET_CS_OUT(1);

    FIO_SetDir(1, 0xFFFFFFFF, DIR_OUT);
    SET_C_OUT5(1);
	SET_C_OUT6(1);
	SET_C_OUT7(1);
	SET_C_OUT8(1);
	SET_C_OUT9(1);
	SET_C_OUT10(1);

	SET_PU_D_IN1(0);
	SET_PU_D_IN2(0);


    FIO_SetDir(2, 0xFFFFFFFF, DIR_OUT);
    SET_D_OUT1_EN(0);
	SET_D_OUT2_EN(0);
	SET_D_OUT3_EN(0);
	SET_D_OUT4_EN(0);

    FIO_SetDir(4, 0xFFFFFFFF, DIR_OUT);
    //FIO_SetValue(4, 0x0);
	SET_PU_D_IN3(0);
	SET_PU_D_IN4(0);

	FIO_SetDir(1, 0xC713, DIR_IN);
	FIO_SetDir(0, (1 << 30) | (1 << 29), DIR_IN);
}

// Board Variables
static uint16_t _ecuPowerSupply = 0;
static FILTER_STRUCT fltVoltage;

void boardMarineECU_Init()
{
	boardPortInit();

	// Фильтр питания ECU
	Filter_init(50, 1, &fltVoltage);
}

void boardMarineECU_Thread()
{
	// 5.7 - делитель напряжения.
	uint16_t voltage_mV = Filter((GetVoltageValue(A_CHNL_KEY)) * 57 / 10 , &fltVoltage);
	 _ecuPowerSupply = voltage_mV / 100;
}

uint16_t boardMarineECU_GetVoltage()
{
	return _ecuPowerSupply;
}
