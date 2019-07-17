/* 
 * File:   LTC6803.h
 */

#ifndef LTC6803_H
#define	LTC6803_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
	
#define VSENS_COUNT 2

typedef enum
{
	LTC6803_FAULTS_NONE,
	LTC6803_F_PEC,
	LTC6803_F_REF_VOLT,
	LTC6803_F_OPEN_CONNECTION,
	LTC6803_F_SIGNAL_INVALID,
} vs_errors_enum;


#define MIN_REF_VOLT_LEVEL_MV	2100
#define MAX_REF_VOLT_LEVEL_MV	2900
#define MAX_PEC_ATTEMPS			5
#define CELL_CODE_ARRAY_SZ		18
#define CELL_GROUP_ARRAY_SZ		6
#define CELL_VOLT_ARRAY_SZ		12
#define TMP_ARRAY_SZ			5
#define CFG_SIZE				6
#define EXT_TMP_SENSOR_NUM		2
#define PEC_INIT				0x41
#define ADDR_CMD				0x80
#define TERM_ARRAY_LEN			21
#define VOLTAGE_JITTER_MV		10
#define DAIG_ARRAY_CZ			2		
#define SZ(x)                 (sizeof(x))
#define PEC_POLY				7

#pragma pack(2)

typedef enum {
    WRCFG = 0x1,
    RDCFG,
    RDCV = 0x4,
    RDCVA = 0x6,
    RDCVB = 0x8,
    RDCVC = 0xA,
    RDFLG = 0xC,
    RDTMP = 0xE,
    STCVAD = 0x10,
	STCLEAR = 0x1D,
    STTEST1 = 0x1E,
	STOWAD	= 0x20,
    STTMPAD = 0x30,
    PLADC = 0x40,
    PLINT = 0x50,
    DAGN = 0x52,
    RDDGNR = 0x54,
    STCVDC = 0x60,
} ltc_cmds_t;


typedef struct
{
	uint8_t CellNumber;
	uint32_t ChipEnableOut;
	uint8_t ChipAddress;
	uint16_t CellTargetVoltage;
	uint16_t BalancingMinVoltage;
	uint32_t BalancingTime_s;
	uint8_t MaxVoltageDiff_mV;
} VoltageSensorParams_t;

typedef struct
{
	VoltageSensorParams_t Parameters;
	
	uint32_t m_timestamp;
	uint32_t m_balanc_timestamp;
	uint8_t m_cfg[CFG_SIZE];// = {0xC2, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t rx_cfg[CFG_SIZE];
	uint8_t m_code_arr[CELL_CODE_ARRAY_SZ];
	uint8_t m_code_group_array[CELL_GROUP_ARRAY_SZ];
	int16_t m_voltage_array[CELL_VOLT_ARRAY_SZ];
	uint8_t m_diag_reg[DAIG_ARRAY_CZ];
	uint8_t m_tmp_code_arr[TMP_ARRAY_SZ];
	int16_t m_tmp_arr[EXT_TMP_SENSOR_NUM];
	
	uint8_t open_conncetion_cell;
	uint16_t m_ref_volt;
	uint32_t discharge_cell_mask;
	uint8_t m_max_pec_attemps;
	uint8_t volt_fault;
	uint8_t IsFault;
	uint8_t FaultCode;
	uint8_t BanBalancing;
	
	ltc_cmds_t m_cmd_table[11];// = {STCVAD, RDCFG, WRCFG, DAGN, STTMPAD, RDDGNR, RDTMP, RDCV, STTEST1, STCVDC, STCLEAR};
	uint8_t * m_dst_ptr_table[11];// = {NULL, rx_cfg, NULL, NULL, NULL, m_diag_reg, m_tmp_code_arr, m_code_arr, NULL, NULL, NULL};
	uint8_t * m_src_ptr_table[11];// = {NULL, NULL, m_cfg, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	uint8_t m_size_ptr_table[11];// = {NULL, SZ(rx_cfg), SZ(m_cfg), NULL, NULL, SZ(m_diag_reg), SZ(m_tmp_code_arr), SZ(m_code_arr), NULL, NULL, NULL};
	
	
} ltc6803_Sensor_t;

#pragma pack(4)


uint16_t* vs_init(uint8_t cs_num, VoltageSensorParams_t *p);
void vs_deinit(uint8_t vs_num);
void vs_set_min_dis_chars(uint16_t dis_start_volt);
bool vs_thread(int16_t *cell_voltage_array, int16_t *cell_temp_array);
int16_t* vs_temp_ptr_get(uint8_t vs_num);
int16_t* vs_cell_volt_get(uint8_t vs_num);
uint8_t ltc6803_GetError(uint8_t vs_num);
uint32_t ltc6803_GetDischargingMask(uint8_t num_cs);

void vs_ban_balancing(uint8_t cmd);
uint16_t find_cell_for_discharge(const int16_t * cell_voltage_array, uint8_t cell_num, const int16_t discharge_volt_target, const int16_t discharge_volt_min);
#ifdef	__cplusplus
}
#endif

#endif	/* LTC6803_H */

