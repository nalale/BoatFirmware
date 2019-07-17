#include <stdint.h>
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "PwmFunc.h"
#include "../MolniaLib/MF_Tools.h"


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
uint32_t GetDiscretIO() {
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

