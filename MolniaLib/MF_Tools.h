#ifndef _TOOL_H_
#define _TOOL_H_

// Возвращает количество элементов в указанном массиве
#define ARRAY_LEN(a)			(sizeof(a) / sizeof(a[0]))

#define SET_BIT(reg, bit)		(reg |= (1L << bit))
#define CLR_BIT(reg, bit)		(reg &= ~(1L << bit))


int16_t interpol(const int16_t *array, int16_t points_num, int16_t x);
uint16_t CRC16(const uint16_t * buf, uint16_t len);


// List
typedef struct node {
    uint16_t val;
    struct node * next;
} List_t;


uint8_t ListPush(List_t ** head, uint16_t val);
uint16_t ListGetValueByIndex(List_t *head, uint16_t n);
uint16_t ListRemoveByIndex(List_t ** head, uint16_t n);
uint16_t ListLength(List_t * head);



#endif
