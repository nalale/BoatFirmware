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
	
	// ������� CAN
	dtc_CAN_mEcu,
	dtc_CAN_Battery,
	
	dtc_PwmCircuit_1,	
	dtc_PwmCircuit_2,	
	dtc_PwmCircuit_3,	
	dtc_MeasuringCircuit,	
	dtc_PowerSupplyCircuit,
	
} dtCodes_e;


// ��� ���� ����� ��������������
typedef struct
{
	// DTCExtendedData
	uint8_t OccurrenceCounter;		// ������� ��� (������� ������) ���������� �������������	
	uint32_t DateTime;				// ���� � ����� ���������� ������������� �������������.
	
	uint16_t Voltage;
	int8_t Load;
	int8_t AmbientTemperature;
	uint32_t Mileage;
	
	// CAN line
	uint32_t CanRxErrorCount;			// ���������� ������ ������ � ���������� ���������
	uint8_t CanBusLoad;					// �������� ���� � %
	
	
	
} dtcFRZF_General;


// ��������� ���� �������� �������������
typedef union
{
	uint8_t Value;
	struct
	{
		uint8_t
		WarningIndicatorRequested		: 1,		// ��������� �� ������������ � ������������� ��� ���
		IsCritical						: 1,		// �������� �� �������� ������ ��� ���
		AutoRestore						: 1,		// ������������� ���������� TestFailedThisOperationCycle, ���� ������ ���������� ����������
		PowerOff						: 1,		// ����������� ��������� ��� ������������� �������������
		
		dummy							: 4;
	};
} dtcPropBits_t;

#define DTC_BIT_NONE					0x00
#define DTC_BIT_WARNING_ENABLE			0x01
#define DTC_BIT_OPERATION_DISABLE		0x02
#define DTC_BIT_AUTO_RESTORE			0x04
#define DTC_BIT_POWER_OFF				0x08

// �������� ��������������� ������ ���� Freeze Frame.
typedef struct
{
	const uint16_t DID;						// ������������� ������ (DID_e)
	const uint8_t Length;					// ����� �������� � ������
	const void* Ref;						// ������ �� �������� �� ��������� Freeze Frame
} DiagnosticValueFRZF;

// �������� �������������
typedef struct
{
	uint16_t Code;									// ��� ������������� (��. dtCodes_e)
	dtcPropBits_t Bits;
	uint8_t AgingThreshold;							// ������� �������� (��. ISO 14229-1, ���.373)
	int8_t TestFailedThreshold;						// ����������� ���������� ������� ��������, ����� ����� ������� ��� Test = Failed (��. ISO 14229-1, ���.371)
	int8_t TestPassedThreshold;						// ����������� ���������� �������� ��� �����, ����� ����� ������� ��� Test = Passed (��. ISO 14229-1, ���.371)
	uint16_t TestSamplePeriod;						// ������ ���������� ����� � ��
	uint8_t FreezeFrameItemsCount;					// ���������� ��������� � ������� FreezeFrameItems
	const DiagnosticValueFRZF* FreezeFrameItems;	// ��������� �� ������ ��������� ����-����� DiagnosticValueFRZF
} dtcProperty_t;

// ������ �������������
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

#define DTC_CLEAR_STATUS					0x10	// 0b10000 - ��������� ��� dtcStatus_t ����� ������ ������
#define DTC_IS_STATUS_FAULT					0x07	// 0b00111 - ���� �������, ������� ��������� ��������������

// ������ � �������������
typedef struct
{
	dtcProperty_t* Property;	// �������� �������������
	uint8_t Category;				// ��������� ������������� (��. dtcCategory_e)
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

