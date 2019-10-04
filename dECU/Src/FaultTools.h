#ifndef _FAULTS_TEST_H_
#define _FAULTS_TEST_H_

#include <stdint.h>


#define DTC_TEST_RESULT_UNCOMPLITED			0
#define DTC_TEST_RESULT_PASSED				1
#define DTC_TEST_RESULT_FAILED				2




#pragma anon_unions

typedef enum
{
	dtc_General_EcuConfig = 0,
	dtc_General_EcuSupplyOutOfRange,
	dtc_General_UnexpectedPowerOff,
	dtc_General_Interlock,
	dtc_General_EcuDataTimeNotCorrect,	
	
	// Таймаут CAN
	dtc_CAN_mEcu,
	dtc_CAN_Battery,
	
	dtc_PwmCircuit_1,	
	dtc_PwmCircuit_2,	
	dtc_PwmCircuit_3,	
	dtc_MeasuringCircuit,	
	dtc_PowerSupplyCircuit,
	
} dtCodes_e;


// Для всех типов неисправностей
typedef struct
{
	// DTCExtendedData
	uint8_t OccurrenceCounter;		// Сколько раз (рабочих циклов) появлялась неисправность	
	uint32_t DateTime;				// Дата и время последнего возникновения неисправности.
	
	uint16_t Voltage;
	int8_t Load;
	int8_t AmbientTemperature;
	uint32_t Mileage;
	
	// CAN line
	uint32_t CanRxErrorCount;			// Количество ошибок приема с последнего включения
	uint8_t CanBusLoad;					// Загрузка шины в %
	
	
	
} dtcFRZF_General;


// Различные биты свойства неисправности
typedef union
{
	uint8_t Value;
	struct
	{
		uint8_t
		WarningIndicatorRequested		: 1,		// Требуется ли индицировать о неисправности или нет
		IsCritical						: 1,		// Выходить из рабочего режима или нет
		AutoRestore						: 1,		// Автоматически страсывать TestFailedThisOperationCycle, если ошибка становится неактивной
		PowerOff						: 1,		// Отключиться полностью при возникновении неисправности
		
		dummy							: 4;
	};
} dtcPropBits_t;

#define DTC_BIT_NONE					0x00
#define DTC_BIT_WARNING_ENABLE			0x01
#define DTC_BIT_OPERATION_DISABLE		0x02
#define DTC_BIT_AUTO_RESTORE			0x04
#define DTC_BIT_POWER_OFF				0x08

// Описание диагностических данных типа Freeze Frame.
typedef struct
{
	const uint16_t DID;						// Идентификатор данных (DID_e)
	const uint8_t Length;					// Длина значения в байтах
	const void* Ref;						// Ссылка на значение из структуры Freeze Frame
} DiagnosticValueFRZF;

// Свойства неисправности
typedef struct
{
	uint16_t Code;									// Код неисправности (см. dtCodes_e)
	dtcPropBits_t Bits;
	uint8_t AgingThreshold;							// Счетчик старения (см. ISO 14229-1, стр.373)
	int8_t TestFailedThreshold;						// Необходимое количество сбойных ситуаций, когда можно сказать что Test = Failed (см. ISO 14229-1, стр.371)
	int8_t TestPassedThreshold;						// Необходимое количество ситуаций без сбоев, когда можно сказать что Test = Passed (см. ISO 14229-1, стр.371)
	uint16_t TestSamplePeriod;						// Период выполнения теста в мс
	uint8_t FreezeFrameItemsCount;					// Количество элементов в массиве FreezeFrameItems
	const DiagnosticValueFRZF* FreezeFrameItems;	// Указатель на массив элементов стоп-кадра DiagnosticValueFRZF
} dtcProperty_t;

// Статус неисправности
typedef struct
{
	uint8_t
	TestFailed							:	1,
	ConfirmedDTC						:	1,
	TestFailedThisOperationCycle		:	1,
	WarningIndicatorRequested			:	1,
	TestNotCompletedThisOperationCycle	: 	1,
	dummy1								:	5;
} dtcStatus_t;

#define DTC_CLEAR_STATUS					0x10	// 0b10000 - Состояние для dtcStatus_t после сброса ошибок
#define DTC_IS_STATUS_FAULT					0x07	// 0b00111 - Биты статуса, которые считаются неисправностью

// Данные о неисправности
typedef struct
{
	dtcProperty_t* Property;	// Свойства неисправности
	uint8_t Category;				// Уточнение неисправности (см. dtcCategory_e)
	dtcStatus_t Status;			
	int8_t FaultDetectionCounter;
	uint16_t SamplePeriodCounter;	
} dtcItem_t;


uint8_t FaultHandler(void);
uint8_t FaultsTest(uint8_t FaultsTestIsEnabled);
uint8_t FillFaultsList(uint16_t *Array, uint8_t *FaultNum, uint8_t IsActualFaults);

uint8_t SaveFaults(void);
uint8_t ReadFaults(void);
uint8_t ClearFaults(void);











#endif

