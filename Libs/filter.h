#ifndef _FILTER_H_
#define	_FILTER_H_



#define MAX_FILTER_LENGH 255


typedef struct
{
	uint8_t OK;								// Статус инициализации
	uint8_t lenght;							// Длинна массива
	uint8_t SampleTime_ms;					// Шаг дискретизации фильтра
	uint16_t cur_item;						// Номер последней записис в массив
	uint32_t TimeStamp;						// Штамп времени
	int16_t Array[MAX_FILTER_LENGH];		// Номер последней записис в массив
	int32_t sum;							// Сумма

} FILTER_STRUCT;



int16_t  Filter (int16_t input, FILTER_STRUCT * str);
uint8_t Filter_init  (uint16_t lenght, uint8_t SampleTime_ms, FILTER_STRUCT * str);
void Filter_clear(FILTER_STRUCT * str);
void Filter_free  (FILTER_STRUCT * str);
void Filter_set  (FILTER_STRUCT * str, int16_t num);

#endif


