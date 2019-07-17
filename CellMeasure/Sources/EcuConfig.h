#ifndef _ECU_CONFIG_H_
#define _ECU_CONFIG_H_

#include <stdint.h>

// Колличесво точек по которым определяются токи заряда и разряда от различных факторов
#define CCL_DCL_POINTS_NUM 6
// Количество строк в том же самом
#define CCL_DCL_LINES_NUM 3

#pragma anon_unions

#pragma pack(4)

// Струкура настроек (конфигурация)
typedef struct
{
	// Общие
    uint8_t DiagnosticID;					// ODB Id
	uint8_t BatteryIndex;					// Порядковый номер батареи в системе (0 - это мастер).
    uint8_t ModuleIndex;                    // Порядковый номер слейва в сети мастера
	
	struct {
		uint8_t
		IsMaster			: 1,			// Если 0, то Master. Иначе Slave.
		CheckContactor		: 1,			// Проверять залипание контакторов или нет
		CheckInterlock		: 1,			// Проверять интерлок или нет
		IsTimeServer		: 1,			// Является ли ЭБУ сервером времени
		IsAutonomic			: 1,			// Управляется по команде с VCU или включается автоматически
		Offline				: 1,			// Выключен. Работает только диагностический протокол
		IsPowerManager		: 1,			// Режим диагностики. Выводится напряжения каждой ячейки
		dummy5				: 1;
	};
	uint32_t BaseID;						// Базовый CAN ID
	
	uint8_t CurrentSensType;				// Тип датчика тока
	uint8_t CurrentSensDirection;			// Положение установки датчика: 0 обычное, 1 противоположное
			
	// Мастер
	uint8_t Sys_MaxVoltageDisbalanceS;			// Максимально-допустимая разбалансировка модулей по напряжению в вольтах
	uint16_t ModuleCapacity;					// Суммарная емкость аккумулятора в Ач
	uint16_t Sys_MaxVoltageDisbalanceP;			// Максимально-допустимая разбалансировка батарей по напряжению в вольтах
	
	
		// DCL/CCL
	int16_t Sys_MaxDCL;							// Максимальный DCL, А
	int16_t Sys_MaxCCL;							// Максимальный CCL, А
	
	uint8_t Sys_ModulesCountS;						// Количество модулей в накопителе, соединенных последовательно
	uint8_t Sys_ModulesCountP;						// Количество модулей в накопителе, соединенных параллельно	

	uint8_t CellNumber;                     	// Количество ячеек в модуле
	uint16_t Sys_MaxCellVoltage_mV;				// Максимальное напряжение на ячейке в мВ
	uint16_t Sys_MinCellVoltage_mV;				// Минимальное напряжение на ячейке в мВ
	
	// Балансировка
    uint16_t MinVoltageForBalancing;			// Минимальный порог напряжения для разрешения балансировки
	
    // Опорные точки зависимости максимального тока разряда от напряжения наиболее заряженного аккумулятора
    int16_t VoltageCCLpoint[CCL_DCL_LINES_NUM][CCL_DCL_POINTS_NUM];
    // Опорные точки зависимости максимального тока заряда от температуры самого нагретого аккумулятора
    int16_t TemperatureCCLpoint[CCL_DCL_LINES_NUM-1][CCL_DCL_POINTS_NUM];		
	
	// Предзаряд
	uint16_t PreMaxDuration;				// Максимальное время предзаряда
    uint16_t PreZeroCurrentDuration;		// Длительность нулевого тока в мс после которого считаем что предзаряд выполнен
	uint16_t PreZeroCurrent;				// Нулевой ток в амперах (когда считается, что предзаряд прошел)
    int16_t PreMaxCurrent;					// Максимальный ток предзаряда в амперах	
	
	uint8_t BalancingTime_s;	
	uint8_t MaxVoltageDiff;					// Разница между ячейками для включения балансировки
	
	// OCV характеристика
    int16_t OCVpoint[CCL_DCL_LINES_NUM-1][CCL_DCL_POINTS_NUM*2];	
	
	uint16_t PowerOffDelay_ms;
	uint16_t KeyOffTime_ms;
	uint16_t addition_5;
	uint16_t addition_6;
	uint16_t addition_7;
	

// Контрольная сумма
    uint16_t CRC;

} EcuConfig_t;



void _cfgSetDefaultParams(void);
void cfgApply(void);
EcuConfig_t GetConfigInstance(void);

#endif

