#ifndef _FILTER_H_
#define	_FILTER_H_



#define MAX_FILTER_LENGH 255


typedef struct
{
	uint8_t OK;								// ������ �������������
	uint8_t lenght;							// ������ �������
	uint8_t SampleTime_ms;					// ��� ������������� �������
	uint16_t cur_item;						// ����� ��������� ������� � ������
	uint32_t TimeStamp;						// ����� �������
	int16_t Array[MAX_FILTER_LENGH];		// ����� ��������� ������� � ������
	int32_t sum;							// �����

} FILTER_STRUCT;



int16_t  Filter (int16_t input, FILTER_STRUCT * str);
uint8_t Filter_init  (uint16_t lenght, uint8_t SampleTime_ms, FILTER_STRUCT * str);
void Filter_clear(FILTER_STRUCT * str);
void Filter_free  (FILTER_STRUCT * str);
void Filter_set  (FILTER_STRUCT * str, int16_t num);

#endif


