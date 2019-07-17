#ifndef _FAULT_CATEGORY_H_
#define _FAULT_CATEGORY_H_





typedef enum
{
	// **************************** General Failure Information ***********************************
	
	dctCat_NoSubtypeInformation = 0x00,			// Нет категории и подтипов для DTC
	dctCat_GeneralElectricalFailure = 0x01,		// Общая электрическая неисправность
	dctCat_GeneralSignalFailure = 0x02,			// Общая неисправность, связанная измерением сигналов
	dctCat_PWM = 0x03,							// Общая неисправность, связанная с ШИМ
	dctCat_SystemInternalFailure = 0x04,		// Общая неисправность, связанная с внутренней аппаратной или программной проблемой ЭБУ
	dctCat_SystemProgrammingFailure = 0x05,		// Общая неисправность, связанная с калибровками/активацией ЭБУ
	dctCat_AlgorithmBasedFailure = 0x06,		// Общая неисправность, связанная с алгоритмом работы ЭБУ
	dctCat_MechanicalFailure = 0x07,			// Общая неисправность, связанная с механической неисправностью
	dctCat_BusSignalOrMessageFailure = 0x08,	// Общая неисправность, связанная с шиной данных
	dctCat_ComponentFailure = 0x09,				// Общая неисправность, связанная с состоянием/поведением компонентов
	
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
	dctCat_SignalStuckLow = 0x23,					// Сигнал долгое время остается низким
	dctCat_SignalStuckHigh = 0x24,					// Сигнал долгое время остается высоким
	dctCat_SignalWaveformFailure = 0x25,
	dctCat_SignalRateOfChangeBelowThreshold = 0x26,	// Сигнал меняется слишком медленно
	dctCat_SignalRateOfChangeAboveThreshold = 0x27,	// Сигнал меняется слишком быстро
	dctCat_SignalBiasLevelFailure = 0x28,			// Неверное смещение средней точки сигнала и т.п.
	dctCat_SignalInvalid = 0x29,					// Неверный сигнал
	dctCat_SignalErratic = 0x2F,					// Signal is momentarily implausible (not long enough for “signal invalid”) or discontinuous.
	
	// ***************************** System Internal Failures *************************************

	dctCat_GeneralChecksumFailure = 0x41,			// Неверная контрольная сумма без уточнения типа памяти
	dctCat_GeneralMemoryFailure = 0x42,				// Ошибка памяти без уточнения ее типа
	dctCat_SpecialMemoryFailure = 0x43,				// Ошибка дополнительной памяти без уточнения ее типа
	dctCat_DataMemoryFailure = 0x44,				// Ошибка памяти данных (RAM)
	dctCat_ProgramMemoryFailure = 0x45,				// Ошибка программной памяти (ROM/Flash)
	dctCat_CalibrationMemoryFailure = 0x46,			// Ошибка памяти калибровок/параметров (EEPROM)
	dctCat_WatchdogOrSafetyFailure = 0x47,
	dctCat_SupervisionSoftwareFailure = 0x48,
	dctCat_InternalElectronicFailure = 0x49,		// Повреждение электроники ЭБУ
	dctCat_IncorrectComponentInstalled = 0x4A,		// К ЭБУ подключен неверный (неподдерживаемый) компонент
	dctCat_OverTemperature = 0x4B,					// Температура ЭБУ за границами рабочего диапазона
	
		// **************************** System Programming Failures ***********************************
	
	dctCat_NotProgrammed = 0x51,
	dctCat_NotActivated = 0x52,						// Некоторые части программы не включены
	dctCat_Deactivated = 0x53,						// Некоторые части программы отключены
	dctCat_MissingCalibration = 0x54,				// Неверные калибровки для датчиков/актуаторов и т.п.
	dctCat_NotConfigured = 0x55,					// ЭБУ не сконфигурирован
	
	// ****************************** Algorithm Based Failures ************************************
		
	dctCat_SignalCalculationFailure = 0x61,
	dctCat_SignalCompareFailure = 0x62,				// При сравнении 2-х или более сигналов на достоверность
	dctCat_CircuitOrComponentProtectionTimeout = 0x63,	// Если функция управления работает слишком долго
	dctCat_SignalPlausibilityFailure = 0x64,		// Недостоверный сигнал
	dctCat_SignalHasTooFewTransitions = 0x65,
	dctCat_SignalHasTooManyTransitions = 0x66,
	dctCat_SignalIncorrectAfterEvent = 0x67,
	dctCat_EventInformation = 0x68,
	
	// ********************************* Mechanical Failures **************************************
			
	dctCat_ActuatorStuck = 0x71,					// Актуатор/реле/солиноид и т.п. не двигаются
	dctCat_ActuatorStuckOpen = 0x72,				// Актуатор/реле/солиноид и т.п. остается открытым по истечении заданного времени
	dctCat_ActuatorStuckClosed = 0x73,				// Актуатор/реле/солиноид и т.п. остается закрытым по истечении заданного времени
	dctCat_ActuatorSlipping = 0x74,					// Актуатор/реле/солиноид и т.п. перемещаются слишком медленно
	dctCat_EmergencyPositionNotReachable = 0x75,	// 
	dctCat_WrongMountingPosition = 0x76,			// Неправильная установка компонента
	dctCat_CommandedPositionNotReachable = 0x77,	// Заданная позиция не достигнута
	dctCat_AlignmentOrAdjustmentIncorrect = 0x78,	// Неправильная регулировка компонента
	dctCat_MechanicalLinkageFailure = 0x79,			// Актуатор работает, но чем он управляет - не двигается (оторвано)
	dctCat_FluidLeakOrSealFailure = 0x7A,			// 
	dctCat_LowFluidLevel = 0x7B,					// Низкий уровень жидкости
	
} dtcCategory_e;



















#endif
