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
		LowVoltage			: 1,	// ���������� ������� ���� 15 �����
		HighVoltage			: 1,	// ���������� ������� ���� ��������������
		dummy2				: 1,	// -
		HighUncertainity	: 1,	// ������� ���������������� ���������
		NoNew				: 1,	// ��� ����� ��������� � ���������� ������� �����������
		HwError				: 1;	// ���������� ������������� (��. sim100_Errors)
	};
} sim100_Status;

typedef union
{
	uint8_t Value;
	struct
	{
		uint8_t
		dummy1				: 2,	// -
		Vpwr				: 1,	// ���������� ������� �� ��������� ����������� ���������
		Vexi				: 1,	// ���������� ����������� �� ��������� ����������� ���������
		VxR					: 1,	// ������������ ���������� ����������� �������������� ���������
		CH					: 1,	// ����� � ����� ����������
		Vx1					: 1,	// ����� � ���� +
		Vx2					: 1;	// ����� � ���� -
	};
} sim100_Errors;


typedef struct
{
	uint32_t Sim100OfflineTime;
	
	
	sim100_Status Status;
	sim100_Errors Errors;
	
	uint16_t Isolation;				// �������� ��/�
	uint8_t IsolationUncertainity;	// ���������������� ��������� �������� � %
	uint16_t StoredEnergy;			// ����������� ������� � ������� ����� �������� � ����� � ���
	uint8_t CapUncertainity;		// ���������������� ��������� ������� � %
	
	uint16_t Rp;					// ������������� �������� ����� ������ � ����� � ���
	uint8_t RpUncertainity;			// ���������������� ��������� ������������� Rp � %
	uint16_t Rn;					// ������������� �������� ����� ������� � ����� � ���
	uint8_t RnUncertainity;			// ���������������� ��������� ������������� Rn � %
	
	uint16_t Cp;					// ������� �������� ����� ������ � ����� � ��
	uint8_t CpUncertainity;			// ���������������� ��������� ������� Cp � %
	uint16_t Cn;					// ������� �������� ����� ������� � ����� � ��
	uint8_t CnUncertainity;			// ���������������� ��������� ������� Cn � %
	
	uint16_t Vp;					// ���������� ����� ������ � ����� � �������
	uint8_t VpUncertainity;			// ���������������� ��������� ���������� Vp � %
	uint16_t Vn;					// ���������� ����� ������� � ����� � �������
	uint8_t VnUncertainity;			// ���������������� ��������� ���������� Vn � %	

	uint16_t Vb;					// ���������� ������� � �������
	uint8_t VbUncertainity;			// ���������������� ��������� ���������� Vb � %
	uint16_t Vmax;					// ������������ ���������� ������� �� ��� ����� ������ � �������
	uint8_t VmaxUncertainity;		// ���������������� ��������� ���������� Vmax � %	
} sim100Data_t;








#endif




