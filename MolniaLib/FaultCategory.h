#ifndef _FAULT_CATEGORY_H_
#define _FAULT_CATEGORY_H_





typedef enum
{
	// **************************** General Failure Information ***********************************
	
	dctCat_NoSubtypeInformation = 0x00,			// ��� ��������� � �������� ��� DTC
	dctCat_GeneralElectricalFailure = 0x01,		// ����� ������������� �������������
	dctCat_GeneralSignalFailure = 0x02,			// ����� �������������, ��������� ���������� ��������
	dctCat_PWM = 0x03,							// ����� �������������, ��������� � ���
	dctCat_SystemInternalFailure = 0x04,		// ����� �������������, ��������� � ���������� ���������� ��� ����������� ��������� ���
	dctCat_SystemProgrammingFailure = 0x05,		// ����� �������������, ��������� � ������������/���������� ���
	dctCat_AlgorithmBasedFailure = 0x06,		// ����� �������������, ��������� � ���������� ������ ���
	dctCat_MechanicalFailure = 0x07,			// ����� �������������, ��������� � ������������ ��������������
	dctCat_BusSignalOrMessageFailure = 0x08,	// ����� �������������, ��������� � ����� ������
	dctCat_ComponentFailure = 0x09,				// ����� �������������, ��������� � ����������/���������� �����������
	
	// **************************** General Electrical Failures ***********************************
	// This category includes standard wiring failure modes (i.e. shorts and opens), and direct current (DC) quantities related by Ohm's Law.
	dctCat_CircuitShortToGround = 0x11,
	dctCat_CircuitShortToBattery = 0x12,
	dctCat_CircuitOpen = 0x13,
	dctCat_CircuitShortToGroundOrOpen = 0x14,
	dctCat_CircuitShortToBatteryOrOpen = 0x15,
	dctCat_CircuitVoltageBelowThreshold = 0x16,
	dctCat_CircuitVoltageAboveThreshold = 0x17,
	dctCat_CircuitCurrentBelowThreshold = 0x18,
	dctCat_CircuitCurrentAboveThreshold = 0x19,
	dctCat_CircuitResistanceBelowThreshold = 0x1A,
	dctCat_CircuitResistanceAboveThreshold = 0x1B,
	dctCat_CircuitVoltageOutOfRange = 0x1C,
	dctCat_CircuitCurrentOutOfRange = 0x1D,
	dctCat_CircuitResistanceOutOfRange = 0x1E,
	dctCat_CircuitIntermittent = 0x1F,
	
	// ***************************** General Signal Failures **************************************
	// specifies quantities related to amplitude, frequency or rate of change, and wave shape.
	
	dctCat_SignalAmplitudeLowerMinimum = 0x21,
	dctCat_SignalAmplitudeHigherMaximum = 0x22,
	dctCat_SignalStuckLow = 0x23,					// ������ ������ ����� �������� ������
	dctCat_SignalStuckHigh = 0x24,					// ������ ������ ����� �������� �������
	dctCat_SignalWaveformFailure = 0x25,
	dctCat_SignalRateOfChangeBelowThreshold = 0x26,	// ������ �������� ������� ��������
	dctCat_SignalRateOfChangeAboveThreshold = 0x27,	// ������ �������� ������� ������
	dctCat_SignalBiasLevelFailure = 0x28,			// �������� �������� ������� ����� ������� � �.�.
	dctCat_SignalInvalid = 0x29,					// �������� ������
	dctCat_SignalErratic = 0x2F,					// Signal is momentarily implausible (not long enough for �signal invalid�) or discontinuous.
	
	// ***************************** System Internal Failures *************************************

	dctCat_GeneralChecksumFailure = 0x41,			// �������� ����������� ����� ��� ��������� ���� ������
	dctCat_GeneralMemoryFailure = 0x42,				// ������ ������ ��� ��������� �� ����
	dctCat_SpecialMemoryFailure = 0x43,				// ������ �������������� ������ ��� ��������� �� ����
	dctCat_DataMemoryFailure = 0x44,				// ������ ������ ������ (RAM)
	dctCat_ProgramMemoryFailure = 0x45,				// ������ ����������� ������ (ROM/Flash)
	dctCat_CalibrationMemoryFailure = 0x46,			// ������ ������ ����������/���������� (EEPROM)
	dctCat_WatchdogOrSafetyFailure = 0x47,
	dctCat_SupervisionSoftwareFailure = 0x48,
	dctCat_InternalElectronicFailure = 0x49,		// ����������� ����������� ���
	dctCat_IncorrectComponentInstalled = 0x4A,		// � ��� ��������� �������� (����������������) ���������
	dctCat_OverTemperature = 0x4B,					// ����������� ��� �� ��������� �������� ���������
	
		// **************************** System Programming Failures ***********************************
	
	dctCat_NotProgrammed = 0x51,
	dctCat_NotActivated = 0x52,						// ��������� ����� ��������� �� ��������
	dctCat_Deactivated = 0x53,						// ��������� ����� ��������� ���������
	dctCat_MissingCalibration = 0x54,				// �������� ���������� ��� ��������/���������� � �.�.
	dctCat_NotConfigured = 0x55,					// ��� �� ���������������
	
	// ****************************** Algorithm Based Failures ************************************
		
	dctCat_SignalCalculationFailure = 0x61,
	dctCat_SignalCompareFailure = 0x62,				// ��� ��������� 2-� ��� ����� �������� �� �������������
	dctCat_CircuitOrComponentProtectionTimeout = 0x63,	// ���� ������� ���������� �������� ������� �����
	dctCat_SignalPlausibilityFailure = 0x64,		// ������������� ������
	dctCat_SignalHasTooFewTransitions = 0x65,
	dctCat_SignalHasTooManyTransitions = 0x66,
	dctCat_SignalIncorrectAfterEvent = 0x67,
	dctCat_EventInformation = 0x68,
	
	// ********************************* Mechanical Failures **************************************
			
	dctCat_ActuatorStuck = 0x71,					// ��������/����/�������� � �.�. �� ���������
	dctCat_ActuatorStuckOpen = 0x72,				// ��������/����/�������� � �.�. �������� �������� �� ��������� ��������� �������
	dctCat_ActuatorStuckClosed = 0x73,				// ��������/����/�������� � �.�. �������� �������� �� ��������� ��������� �������
	dctCat_ActuatorSlipping = 0x74,					// ��������/����/�������� � �.�. ������������ ������� ��������
	dctCat_EmergencyPositionNotReachable = 0x75,	// 
	dctCat_WrongMountingPosition = 0x76,			// ������������ ��������� ����������
	dctCat_CommandedPositionNotReachable = 0x77,	// �������� ������� �� ����������
	dctCat_AlignmentOrAdjustmentIncorrect = 0x78,	// ������������ ����������� ����������
	dctCat_MechanicalLinkageFailure = 0x79,			// �������� ��������, �� ��� �� ��������� - �� ��������� (��������)
	dctCat_FluidLeakOrSealFailure = 0x7A,			// 
	dctCat_LowFluidLevel = 0x7B,					// ������ ������� ��������
	
} dtcCategory_e;



















#endif
