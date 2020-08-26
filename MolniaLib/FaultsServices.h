#ifndef _FAULTS_SERVICES_H_
#define _FAULTS_SERVICES_H_

#define DTC_TEST_RESULT_UNCOMPLITED			0
#define DTC_TEST_RESULT_PASSED				1
#define DTC_TEST_RESULT_FAILED				2

// Максимальное количество в списке ошибок
#define MAX_FAULTS_NUM				10

extern uint8_t MemEcuDtcWrite(uint8_t *pData, uint16_t Length);
extern void MemEcuDtcRead(uint8_t *pData, uint16_t Length);

#pragma anon_unions

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

typedef struct
{
	struct
	{
		uint8_t
		CriticalFaultExist				: 1,				// Выходить из рабочего режима или нет (серьезная ошибка)
		WarningIndicatorRequested		: 1,				// Требуется ли индицировать о неисправности или нет
		PowerOff						: 1,				// Отключиться полностью при возникновении неисправности

		dummy : 5;
	};
} dtcEnvironment_t;

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

uint8_t FillFaultsList(dtcItem_t** dtcList, int16_t dtcListSize, uint16_t *FaultsList, uint8_t IsActualFaults);

void SetGeneralFRZR(dtcFRZF_General* f);
void dtcSetFault(dtcItem_t* it, dtcEnvironment_t* env);
uint8_t dtcFaultDetection(dtcItem_t* it, dtcEnvironment_t *env, uint8_t isFault);

uint8_t SaveFaults(dtcItem_t** dtcList, int16_t dtcListLength);
uint8_t ReadFaults(dtcItem_t** dtcList, int16_t dtcListLength);
int8_t ClearFaults(dtcItem_t** dtcList, int16_t dtcListSize);




#endif
