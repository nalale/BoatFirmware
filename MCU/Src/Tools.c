#include "Main.h"
#include "Tools.h"



uint16_t interpol(int16_t *array, int16_t points_num, int16_t x)
{
	uint8_t cnt = 1;
	int16_t y1, y0, x0, x1, dy, ddx, dx;
	// ��������� ��������� 1)
	if (x <= array[0])
		return array[points_num];
	if (x >= array[points_num - 1])
		return array[2 * points_num - 1];
	// ����� ������ ��������, �������� �� �������, ������� ����� ������,���
	// ��������
	while (x >= array[cnt])
		cnt++;
	// ���� � ������� ���� ������� ��������, �� ������� ��������������� ��������
	// �������
	if (x == array[cnt])
		return array[points_num + cnt];
	// ���� ������������� �����, �� ������ ������ ����� �������.

	y1 = array[points_num + cnt];
	y0 = array[points_num + cnt - 1];
	x0 = array[cnt - 1];
	x1 = array[cnt];
	dy = y1 - y0;
	ddx = x - x0;
	dx = x1 - x0;
	return y0 + ((int32_t) ddx * dy) / dx;

//    return array[points_num + cnt - 1] + ((x -  array[cnt - 1]) * (array[points_num+ cnt] - array[points_num + cnt - 1])) / (array[cnt] - array[cnt - 1]);
}

