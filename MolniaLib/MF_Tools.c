#include "Main.h"
#include "../MolniaLib/MF_Tools.h"
#include "stdlib.h"


int16_t interpol(const int16_t *array, int16_t points_num, int16_t x)
{
	uint8_t cnt = 1;
	int16_t y1, y0, x0, x1, dy, ddx, dx;
	// Обещанное упрощение 1)
	if (x <= array[0])
		return array[points_num];
	if (x >= array[points_num - 1])
		return array[2 * points_num - 1];
	// Найдём первое значение, ординаты из таблицы, которое будет больше,чем
	// заданное
	while (x >= array[cnt])
		cnt++;
	// Если в таблице есть искомая ордината, то вернуть соответствующее значение
	// Функции
	if (x == array[cnt])
		return array[points_num + cnt];
	// Если промежуточное число, то решаем просто через тангенс.

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

/***************************************************************************************
                    Проверка CRC
***************************************************************************************/
uint16_t CRC16(const uint16_t * buf, uint16_t len)
{
	uint16_t crc = 0xFFFF;
	uint16_t DummyInt;
	for (int pos = 0; pos < len; pos++)
	{
		DummyInt = *buf;
		buf++;
		crc ^= DummyInt;
		for (int i = 8; i != 0; i--)
		{
			if ((crc & 0x0001) != 0)
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
				crc >>= 1;
		}
	}
	// Помните, что младший и старший байты поменяны местами, используйте соответственно (или переворачивайте)
	return crc;
}






// List
List_t* ListInit(uint16_t val)
{
	List_t * head = NULL;
	head = malloc(sizeof(List_t));
	if (head == NULL) {
		return NULL;
	}

	head->val = val;
	head->next = NULL;
	
	return head;
}


// Добавить элемент в конец списка
uint8_t ListPush(List_t **head, uint16_t val) 
{
    List_t **current = head;
	
	if(*head == NULL)
	{
		*head = ListInit(val);
		return 1;
	}
	
    while ((*current)->next != NULL) {
        *current = (*current)->next;
    }

    /* now we can add a new variable */
    (*current)->next = malloc(sizeof(List_t));
	
	if((*current)->next == NULL)
		return 0;
	
    (*current)->next->val = val;
    (*current)->next->next = NULL;
	return 1;
}

// Извлечь первый элемент списка
int ListPop(List_t ** head) {    
    List_t * next_node = NULL;

    if (*head == NULL) {
        return 0;
    }

    next_node = (*head)->next;
    free(*head);
    *head = next_node;

    return 1;
}

// Удалить элемент списка
uint16_t ListRemoveByIndex(List_t ** head, uint16_t n) {
    int i = 0;
    List_t * current = *head;
    List_t * temp_node = NULL;
	
	if(current == NULL)
		return 0;

    if (n == 0) {
        return ListPop(head);
    }

    for (i = 0; i < n-1; i++) {
        if (current->next == NULL) {
            return 0;
        }
        current = current->next;
    }

    temp_node = current->next;
    current->next = temp_node->next;
    free(temp_node);

    return 1;
}

// Получить элемент списка
uint16_t ListGetValueByIndex(List_t *head, uint16_t n) {
    int i = 0;
    List_t current = *head;
	
	if(head == NULL)
		return 0;

    if (n == 0) {
        return current.val;	
    }

    for (i = 0; i < n-1; i++) 
	{
        if (current.next == NULL) {
            return 0;
        }
        current = *current.next;
    }

    return current.val;
}

uint16_t ListLength(List_t * head) {
    List_t * current = head;
	uint16_t result = 0;
	
    while (current != NULL) {
        result++;
        current = current->next;
    }
	
	return result;	
}



