#include "main.h"
#include "filter.h"
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <stdlib.h> 
#include <string.h>
#include "TimerFunc.h"



/*******************************************************************************
//  ���������� 
 *******************************************************************************/
int16_t Filter(int16_t input, FILTER_STRUCT * str)
{
	if (GetTimeFrom(str->TimeStamp) > str->SampleTime_ms)
	{
		// ����������������� ������ �����
		str->sum += input;
		// ����������������� ������� �����
		str->sum -= str->Array[str->cur_item];
		// ������ ������ ��������
		str->Array[str->cur_item] = input;
		// ����������������� �������� 
		if ((++(str->cur_item)) >= str->lenght) str->cur_item = 0;
		//
		str->TimeStamp = GetTimeStamp();
		// ����������� ���������������� �������� 
		return str->sum / str->lenght;
	}
	else
	{
		return str->sum / str->lenght;
	}

}
/*******************************************************************************
//  ������������� ��������� ���������� 
 *******************************************************************************/
uint8_t Filter_init(uint16_t lenght, uint8_t SampleTime_ms, FILTER_STRUCT * str)
{
	if (lenght > MAX_FILTER_LENGH)
		lenght = MAX_FILTER_LENGH;
	str->lenght = lenght;               // ���������� 
	str->OK = true;
	Filter_clear(str);
	return true;

}


void Filter_clear(FILTER_STRUCT * str)
{
	str->sum = 0;
	str->cur_item = 0;
	for (int i = 0; i < str->lenght; i++)
	{
		str->Array[i] = 0;
	}
}

/*******************************************************************************
//  ������� � ������ ����������� �������� 
*******************************************************************************/
void Filter_set  (FILTER_STRUCT * str, int16_t num)
{	
    uint8_t i = 0;
    for (i = 0; i < str->lenght; i++){
        str->Array[i] = num;
    }
}

