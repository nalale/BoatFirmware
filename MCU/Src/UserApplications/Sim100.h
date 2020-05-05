#ifndef _SIM100_H_
#define _SIM100_H_

#include "stdint.h"


enum sim100_OpCodes
{
	opCode_ReadIsolation = 0xE0,
	opCode_ReadResistance,
	opCode_ReadCapacitance,
	opCode_ReadVoltage,
	opCode_ReadBatteryVoltage,
	opCode_ReadError,
	opCode_WriteMaxVoltage = 0xF0,
};

typedef union
{
	uint8_t Value;
	struct
	{
		uint8_t
		dummy1				: 2,	// -
		LowVoltage			: 1,	// Напряжение батареи ниже 15 вольт
		HighVoltage			: 1,	// Напряжение батареи выше установленного
		dummy2				: 1,	// -
		HighUncertainity	: 1,	// Высокая неопределенность измерений
		NoNew				: 1,	// Нет новых измерений с последнего запроса результатов
		HwError				: 1;	// Аппаратная неисправность (см. sim100_Errors)
	};
} sim100_Status;

typedef union
{
	uint8_t Value;
	struct
	{
		uint8_t
		dummy1				: 2,	// -
		Vpwr				: 1,	// Напряжение питания за пределами допустимого диапазона
		Vexi				: 1,	// Напряжение возбуждения за пределами допустимого диапазона
		VxR					: 1,	// Неправильная полярность подключения высоковольтных контактов
		CH					: 1,	// Обрыв в цепях заземления
		Vx1					: 1,	// Обрыв в цепи +
		Vx2					: 1;	// Обрыв в цепи -
	};
} sim100_Errors;


typedef struct
{
	uint32_t Sim100OfflineTime;
	
	
	sim100_Status Status;
	sim100_Errors Errors;
	
	uint16_t Isolation;				// Изоляция Ом/В
	uint8_t IsolationUncertainity;	// Неопределенность измерения изоляции в %
	uint16_t StoredEnergy;			// Сохраненная энергия в емкости между батареей и шасси в мДж
	uint8_t CapUncertainity;		// Неопределенность измерения емкости в %
	
	uint16_t Rp;					// Сопротивление изоляции между плюсом и шасси в кОм
	uint8_t RpUncertainity;			// Неопределенность измерения сопротивления Rp в %
	uint16_t Rn;					// Сопротивление изоляции между минусом и шасси в кОм
	uint8_t RnUncertainity;			// Неопределенность измерения сопротивления Rn в %
	
	uint16_t Cp;					// Емкость изоляции между плюсом и шасси в нФ
	uint8_t CpUncertainity;			// Неопределенность измерения емкости Cp в %
	uint16_t Cn;					// Емкость изоляции между минусом и шасси в нФ
	uint8_t CnUncertainity;			// Неопределенность измерения емкости Cn в %
	
	uint16_t Vp;					// Напряжение между плюсом и шасси в вольтах
	uint8_t VpUncertainity;			// Неопределенность измерения напряжения Vp в %
	uint16_t Vn;					// Напряжение между минусом и шасси в вольтах
	uint8_t VnUncertainity;			// Неопределенность измерения напряжения Vn в %	

	uint16_t Vb;					// Напряжение батареи в вольтах
	uint8_t VbUncertainity;			// Неопределенность измерения напряжения Vb в %
	uint16_t Vmax;					// Максимальное напряжение батареи за все время работы в вольтах
	uint8_t VmaxUncertainity;		// Неопределенность измерения напряжения Vmax в %	
} sim100Data_t;








#endif




