#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "LTC6803.h"
#include "TimerFunc.h"
#include "../BoardDefinitions/BmsCombi_Board.h"
#include "SpiFunc.h"
#include "../MolniaLib/sys_utils.h"
#include "../MolniaLib/MF_Tools.h"

#define MAX_MEASURING_mV	5375

//static uint32_t m_timestamp;
//static uint32_t m_balanc_timestamp = 0;
//static uint8_t m_cfg[CFG_SIZE] = {0xC2, 0x00, 0x00, 0x00, 0x00, 0x00};
//static uint8_t rx_cfg[CFG_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//static uint8_t m_code_arr[CELL_CODE_ARRAY_SZ];
//static uint8_t m_code_group_array[CELL_GROUP_ARRAY_SZ];
//static uint16_t m_voltage_array[CELL_VOLT_ARRAY_SZ];
//static uint8_t m_diag_reg[DAIG_ARRAY_CZ];
//static uint8_t m_tmp_code_arr[TMP_ARRAY_SZ];
//static int16_t m_tmp_arr[EXT_TMP_SENSOR_NUM];
//static uint16_t CellTargetVoltage = INT16_MAX;
//static uint16_t BalancingMinVoltage = INT16_MAX;
//static uint8_t m_oper_cell_number;
//static uint8_t m_max_pec_attemps = 0;
//static uint16_t m_ref_volt;
//static uint32_t discharge_cell_mask = 0;
//static uint32_t discharge_timestamp = 0;


int16_t first_meas_cells[VSENS_COUNT][CELL_VOLT_ARRAY_SZ];
int16_t second_meas_cells[VSENS_COUNT][CELL_VOLT_ARRAY_SZ];


int16_t rt_t0_r25_table[2][TERM_ARRAY_LEN] ={
    {17, 20, 24, 29, 36, 43, 53, 65, 80, 100, 124, 157, 199, 253, 326, 423, 553, 729, 970, 1304, 1770},
    {70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0, -5, -10, -15, -20, -25, -30}
};

static ltc6803_Sensor_t vs[VSENS_COUNT];

uint8_t pec8_calc(uint8_t len, uint8_t *data);
uint8_t OpenConnectionDetectin(int16_t *first_meas, int16_t *second_meas, uint8_t cells_num);
uint8_t vs_faults_check(uint8_t vs_num);

static uint8_t pec(uint8_t * const data, uint8_t len) {
    uint8_t pec_val = PEC_INIT;
    uint8_t bit_mask;
    uint8_t data_inx;
    int8_t bit_inx;
    uint8_t in;
    bool din;

    for (data_inx = 0; data_inx < len; data_inx++) {
        for (bit_inx = 7; bit_inx >= 0; bit_inx--) {
            in = 0;
            bit_mask = 1 << bit_inx;
            din = (data[data_inx] & bit_mask) != 0;

            in |= din ^ ((pec_val & 0x80) != 0);
            in |= (((in & 0x1) != 0) ^ ((pec_val & 0x1) != 0)) << 1;
            in |= (((in & 0x1) != 0) ^ ((pec_val & 0x2) != 0)) << 2;

            pec_val <<= 1;
            pec_val &= ~0x7;
            pec_val |= in & 0x7;
        }
    }
    return pec_val;
}

static void lts_cmd_send(uint8_t vs_num, ltc_cmds_t cmd) {
    uint8_t cmd_i;
    bool cmd_found = false;
	uint8_t m_max_pec_attemps = 0;

    for (cmd_i = 0; cmd_i < ARRAY_SIZE(vs[vs_num].m_cmd_table); cmd_i++) {
        if (cmd == vs[vs_num].m_cmd_table[cmd_i]) {
            cmd_found = true;
            break;
        }
    }
    if (cmd_found == false) {
        while (1);
    }

//    uint8_t cmd_buffer[2];
//    cmd_buffer[0] = cmd;
//    cmd_buffer[1] = pec(&cmd_buffer[0], 1);
	
	uint8_t cmd_buffer[4];
	cmd_buffer[0] = ADDR_CMD + vs[vs_num].Parameters.ChipAddress;
	cmd_buffer[1] = pec(&cmd_buffer[0], 1);
    cmd_buffer[2] = cmd;
    cmd_buffer[3] = pec(&cmd_buffer[2], 1);

    uint8_t my_pec = 0xAA;
    uint8_t device_pec = 0xBB;
    
    m_max_pec_attemps = 0;
    if (vs[vs_num].m_dst_ptr_table[cmd_i] != NULL) 
	{        
        while (my_pec != device_pec && m_max_pec_attemps < MAX_PEC_ATTEMPS) 
		{
            gpio_ltc6804_cs_set(vs[vs_num].Parameters.ChipEnableOut, GPIO_LOW);
            uint8_t byte_cnt = SpiReadWrite(cmd_buffer, 4/*sizeof (cmd_buffer)*/, NULL);
            byte_cnt = SpiReadWrite(NULL, vs[vs_num].m_size_ptr_table[cmd_i], vs[vs_num].m_dst_ptr_table[cmd_i]);
            byte_cnt = SpiReadWrite(NULL, sizeof (device_pec), &device_pec);
            gpio_ltc6804_cs_set(vs[vs_num].Parameters.ChipEnableOut, GPIO_HIGH);
            my_pec = pec(vs[vs_num].m_dst_ptr_table[cmd_i], vs[vs_num].m_size_ptr_table[cmd_i]);
            m_max_pec_attemps++;
        }
    } else if (vs[vs_num].m_src_ptr_table[cmd_i] != NULL) 
	{
        my_pec = pec(vs[vs_num].m_src_ptr_table[cmd_i], vs[vs_num].m_size_ptr_table[cmd_i]);
        gpio_ltc6804_cs_set(vs[vs_num].Parameters.ChipEnableOut, GPIO_LOW);
        SpiReadWrite(cmd_buffer, sizeof (cmd_buffer), NULL);
        SpiReadWrite(vs[vs_num].m_src_ptr_table[cmd_i], vs[vs_num].m_size_ptr_table[cmd_i], NULL);
        SpiReadWrite(&my_pec, sizeof (my_pec), NULL);
        gpio_ltc6804_cs_set(vs[vs_num].Parameters.ChipEnableOut, GPIO_HIGH);
    } else 
	{
        gpio_ltc6804_cs_set(vs[vs_num].Parameters.ChipEnableOut, GPIO_LOW);
        SpiReadWrite(cmd_buffer, sizeof (cmd_buffer), NULL);
        gpio_ltc6804_cs_set(vs[vs_num].Parameters.ChipEnableOut, GPIO_HIGH);
    }
}

static uint16_t code_to_voltage(uint16_t code) {
    if (code == 0x0fff) {
        return 0xffff;
    } else {
        //return (3 * (code - 512)) / 2;
		return (3 * (code - 512)) >> 1;		// (code - 512) / 1.5mV
    }
}

static void discharge_mask_set(uint8_t vs_num, uint16_t mask) {
    uint8_t dummy;

    vs[vs_num].m_cfg[1] = mask & 0xFF;
    dummy = 0xF0 & vs[vs_num].m_cfg[2];
    vs[vs_num].m_cfg[2] = dummy | (0xF & (mask >> 8));
}

static void cdc_mask_set(uint8_t vs_num, uint8_t mask) {
    vs[vs_num].m_cfg[0] = mask & 0x7;
}

static uint8_t volt_code_parse(uint8_t sensor_num, uint8_t cell_number, uint8_t *code_arr, int16_t *res_voltage_array) {
    uint8_t cnt;
    uint16_t code = 0;   

	int data_counter = 0;
    uint16_t temp,temp2;
    uint8_t result = 0;
	
    for (cnt = 0; cnt < cell_number; cnt = cnt + 2) {
		
		temp = code_arr[data_counter++];
		temp2 = (uint16_t)(code_arr[data_counter] & 0x0F) << 8;
		code = temp + temp2;

		if(code < 0x0fff)
			res_voltage_array[cnt] = (3 * (code - 512)) >> 1;
		else
		{
			res_voltage_array[cnt] = 0xffff;
			result = 1;
		}

		
		temp2 = (code_arr[data_counter++]) >> 4;
		temp =  (code_arr[data_counter++]) << 4;
		code = temp + temp2;
		//res_voltage_array[cnt+1] = (code < 0x0fff)? (3 * (code - 512)) >> 1 : 0xffff;

		if(code < 0x0fff)
			res_voltage_array[cnt+1] = (3 * (code - 512)) >> 1;
		else
		{
			res_voltage_array[cnt+1] = 0xffff;
			result = 1;
		}		
    }
	return result;
}

uint16_t find_cell_for_discharge(const int16_t * cell_voltage_array, uint8_t cell_num, const int16_t discharge_volt_target, const int16_t discharge_volt_min)
{    
	uint16_t discharge_mask = 0;
    for(int cnt = 0; cnt < cell_num; cnt++)
    {
        if ((cell_voltage_array[cnt] > discharge_volt_target + vs[0].Parameters.MaxVoltageDiff_mV) 
			&& cell_voltage_array[cnt] > discharge_volt_min) {
            discharge_mask |= 1UL << cnt;
        } else {
            discharge_mask &= ~(1UL << cnt);
        }
    }	
    
	return discharge_mask;
}

static int16_t code_to_tmp(uint16_t code, uint16_t ref_volt) {
    uint32_t tmp_volt = code_to_voltage(code);
    //uint16_t ref_code = ((uint16_t)(m_diag_reg[1] & 0xF) << 8) | m_diag_reg[0];
    int16_t tmp;

    //m_ref_volt = code_to_voltage(ref_code);
    tmp_volt *= 100;
    tmp_volt /= ref_volt / 2;
    tmp = interpol(rt_t0_r25_table[0], TERM_ARRAY_LEN, (int16_t) tmp_volt);

    return tmp;
}

static void temp_code_parse(uint8_t tmp_number, uint8_t *code_arr, uint16_t ref_volt, int16_t *res_temp_array) {
    uint8_t cnt;
    uint8_t bait_number;
    uint16_t code;
    for (cnt = 0; cnt < tmp_number; cnt++) {
        bait_number = (3 * cnt) / 2;

        if (cnt & 0x1) {
            code = code_arr[bait_number + 1];
            code <<= 4;
            code |= code_arr[bait_number] & 0xF;
        } else {
            code = code_arr[bait_number + 1] & 0xF;
            code <<= 8;
            code |= code_arr[bait_number];
        }
        res_temp_array[cnt] = code_to_tmp(code, ref_volt);
    }
}

uint16_t * vs_init(uint8_t vs_num, VoltageSensorParams_t *p) 
{
	vs[vs_num].Parameters = *p;	
	
	// Start cmds
	vs[vs_num].m_cmd_table[0] = STCVAD;
	vs[vs_num].m_dst_ptr_table[0] = NULL;
	vs[vs_num].m_src_ptr_table[0] = NULL;
	vs[vs_num].m_size_ptr_table[0] = NULL;
	
	vs[vs_num].m_cmd_table[1] = STTMPAD;
	vs[vs_num].m_dst_ptr_table[1] = NULL;
	vs[vs_num].m_src_ptr_table[1] = NULL;
	vs[vs_num].m_size_ptr_table[1] = NULL;
	
	vs[vs_num].m_cmd_table[2] = DAGN;
	vs[vs_num].m_dst_ptr_table[2] = NULL;
	vs[vs_num].m_src_ptr_table[2] = NULL;
	vs[vs_num].m_size_ptr_table[2] = NULL;
	
	vs[vs_num].m_cmd_table[3] = STCVDC;
	vs[vs_num].m_dst_ptr_table[3] = NULL;
	vs[vs_num].m_src_ptr_table[3] = NULL;
	vs[vs_num].m_size_ptr_table[3] = NULL;
	
	vs[vs_num].m_cmd_table[4] = STCLEAR;
	vs[vs_num].m_dst_ptr_table[4] = NULL;
	vs[vs_num].m_src_ptr_table[4] = NULL;
	vs[vs_num].m_size_ptr_table[4] = NULL;

	vs[vs_num].m_cmd_table[5] = STOWAD;
	vs[vs_num].m_dst_ptr_table[5] = NULL;
	vs[vs_num].m_src_ptr_table[5] = NULL;
	vs[vs_num].m_size_ptr_table[5] = NULL;
	
	// Read cmds
	vs[vs_num].m_cmd_table[6] = RDCFG;
	vs[vs_num].m_dst_ptr_table[6] = vs[vs_num].rx_cfg;
	vs[vs_num].m_src_ptr_table[6] = NULL;
	vs[vs_num].m_size_ptr_table[6] = SZ(vs[vs_num].rx_cfg);
	
	vs[vs_num].m_cmd_table[7] = RDDGNR;
	vs[vs_num].m_dst_ptr_table[7] = vs[vs_num].m_diag_reg;
	vs[vs_num].m_src_ptr_table[7] = NULL;
	vs[vs_num].m_size_ptr_table[7] = SZ(vs[vs_num].m_diag_reg);
	
	vs[vs_num].m_cmd_table[8] = RDTMP;
	vs[vs_num].m_dst_ptr_table[8] = vs[vs_num].m_tmp_code_arr;
	vs[vs_num].m_src_ptr_table[8] = NULL;
	vs[vs_num].m_size_ptr_table[8] = SZ(vs[vs_num].m_tmp_code_arr);
	
	vs[vs_num].m_cmd_table[9] = RDCV;
	vs[vs_num].m_dst_ptr_table[9] = vs[vs_num].m_code_arr;
	vs[vs_num].m_src_ptr_table[9] = NULL;
	vs[vs_num].m_size_ptr_table[9] = SZ(vs[vs_num].m_code_arr);	
	
	// Write cmds
	vs[vs_num].m_cmd_table[10] = WRCFG;
	vs[vs_num].m_dst_ptr_table[10] = NULL;
	vs[vs_num].m_src_ptr_table[10] = vs[vs_num].m_cfg;
	vs[vs_num].m_size_ptr_table[10] = SZ(vs[vs_num].m_cfg);
	
	
	vs[vs_num].m_cfg[0] = 0x00;
	vs[vs_num].m_cfg[1] = 0x0;
	vs[vs_num].m_cfg[2] = 0x0;
	vs[vs_num].m_cfg[3] = 0x0;
	vs[vs_num].m_cfg[4] = 0x0;
	vs[vs_num].m_cfg[5] = 0x0;
	
	vs[vs_num].FaultCode = LTC6803_FAULTS_NONE;
	vs[vs_num].IsFault = 0;
	
	vs[vs_num].BanBalancing = 0;
	
	//lts_cmd_send(vs_num, WRCFG);
	cdc_mask_set(vs_num, 0x4);
	discharge_mask_set(vs_num, 0x00);
	lts_cmd_send(vs_num, WRCFG);
	
	
//		ltc_cmds_t m_cmd_table[11] = {STCVAD, RDCFG, WRCFG, DAGN, STTMPAD, RDDGNR, RDTMP, RDCV, STTEST1, STCVDC, STCLEAR};
//	uint8_t * m_dst_ptr_table[11] = {NULL, rx_cfg, NULL, NULL, NULL, m_diag_reg, m_tmp_code_arr, m_code_arr, NULL, NULL, NULL};
//	uint8_t * m_src_ptr_table[11] = {NULL, NULL, m_cfg, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
//	uint8_t m_size_ptr_table[11] = {NULL, SZ(rx_cfg), SZ(m_cfg), NULL, NULL, SZ(m_diag_reg), SZ(m_tmp_code_arr), SZ(m_code_arr), NULL, NULL, NULL};
    return 0;//m_voltage_array;
}

void vs_deinit(uint8_t vs_num) {
	cdc_mask_set(vs_num, 0x0);
	discharge_mask_set(vs_num, 0);
	lts_cmd_send(vs_num, WRCFG);
}



int16_t * vs_temp_ptr_get(uint8_t vs_num) {
    return vs[vs_num].m_tmp_arr;
}

int16_t* vs_cell_volt_get(uint8_t vs_num)
{
	return vs[vs_num].m_voltage_array;
}

bool vs_thread(int16_t *cell_voltage_array, int16_t *cell_temp_array) 
{
    static uint8_t step = 0;
    static uint32_t m_timestamp ;
	static uint32_t discharge_timestamp = 0;
	static uint32_t discharge_timer = 0;
	static uint32_t open_connect_timestamp = 0;
	static uint8_t vs_num = 0;
	static uint8_t meas_step = 0;
	static uint8_t flag = 0;
	static uint8_t balanc_flag = 0;
	
	// Active current sensor count: if second sensor doesn't have any active cells active sensor will equal 1
	uint8_t sens_cnt = (vs[1].Parameters.CellNumber > 0)? VSENS_COUNT : 1;
	
	switch (step)
	{
	// Open Connection Detection
		case 0:
		{
			m_timestamp = GetTimeStamp();

			for(vs_num = 0; vs_num < sens_cnt; vs_num++)
				lts_cmd_send(vs_num, STOWAD);

			meas_step++;
			step = 1;
		}
		break;
		case 1:
		if (GetTimeFrom(m_timestamp) > 20)
		{
			m_timestamp = GetTimeStamp();
			for(vs_num = 0; vs_num < sens_cnt; vs_num++)
			{
				int16_t *target_array = (int16_t *)((meas_step == 1)? first_meas_cells[vs_num] : second_meas_cells[vs_num]);
				lts_cmd_send(vs_num, RDCV);
				volt_code_parse(vs_num, vs[vs_num].Parameters.CellNumber, vs[vs_num].m_code_arr, target_array);

				if(meas_step == 1)
				{
					vs[vs_num].open_conncetion_cell = OpenConnectionDetectin(first_meas_cells[vs_num], second_meas_cells[vs_num], vs[vs_num].Parameters.CellNumber);
					step = 2;
				}
				else
					step = 	0;
			}

			open_connect_timestamp = GetTimeStamp();
		}
		break;

		// Get Cells Voltage
		case 2:
			m_timestamp = GetTimeStamp();
			for(vs_num = 0; vs_num < sens_cnt; vs_num++)
			{
				vs[vs_num].volt_fault = 0;

				if(ltc6803_GetError(vs_num) != LTC6803_FAULTS_NONE)
					vs[vs_num].IsFault = 1;

				// start cell voltage measuring
				lts_cmd_send(vs_num, STCVDC);
			}


			step = 3;
			break;

		// Read cell voltage
		case 3:
			if (GetTimeFrom(m_timestamp) > 20)
			{
				m_timestamp = GetTimeStamp();
				for(vs_num = 0; vs_num < sens_cnt; vs_num++)
				{
					lts_cmd_send(vs_num, RDCV);
					vs[vs_num].volt_fault = volt_code_parse(vs_num, vs[vs_num].Parameters.CellNumber, vs[vs_num].m_code_arr, vs[vs_num].m_voltage_array);
					memcpy((void*)(cell_voltage_array + vs_num * CELL_VOLT_ARRAY_SZ), (void*)vs[vs_num].m_voltage_array, sizeof(vs[vs_num].m_voltage_array));
					vs[vs_num].results_is_available = 1;
				}
				step = 4;
			}
			break;

		case 4:
			for(vs_num = 0; vs_num < sens_cnt; vs_num++)
				lts_cmd_send(vs_num, DAGN);

			step = 5;
		break;
			// Temperature measuring
		case 5:
			if(GetTimeFrom(m_timestamp) > 20)
			{
				for(vs_num = 0; vs_num < sens_cnt; vs_num++)
				{
					lts_cmd_send(vs_num, RDDGNR);
					lts_cmd_send(vs_num, STTMPAD);
					uint16_t code = vs[vs_num].m_diag_reg[0] + ((uint16_t)(vs[vs_num].m_diag_reg[1] & 0x0f) << 8);
					vs[vs_num].m_ref_volt = code_to_voltage(code);
				}
				
				m_timestamp = GetTimeStamp();
				step = 6;
			}
			break;
		case 6:
			if (GetTimeFrom(m_timestamp) > 10)
			{
				for(vs_num = 0; vs_num < sens_cnt; vs_num++)
				{
					// read temperature
					lts_cmd_send(vs_num, RDTMP);
					temp_code_parse(2, vs[vs_num].m_tmp_code_arr, vs[vs_num].m_ref_volt, vs[vs_num].m_tmp_arr);
					int16_t *dest = cell_temp_array + vs_num * EXT_TMP_SENSOR_NUM;
					memcpy((void*)dest, (void*)vs[vs_num].m_tmp_arr, sizeof(vs[vs_num].m_tmp_arr));
					lts_cmd_send(vs_num, STCLEAR);

					m_timestamp = GetTimeStamp();
				}
				step = 7;
			}
			break;

			// Get CFG register to find out discharging cell mask
			// Find out cell number for discharge
		case 7:
			{
				m_timestamp = GetTimeStamp();

				static uint16_t dummy1 = 0;
				
				for(vs_num = 0; vs_num < sens_cnt; vs_num++)
				{
					lts_cmd_send(vs_num, RDCFG);

					vs[vs_num].discharge_cell_mask = (uint16_t)vs[vs_num].rx_cfg[1] + ((uint16_t)(vs[vs_num].rx_cfg[2] & 0x0f) << 8);
					/*if(vs_num == 0)
						dummy1 |= (uint16_t)vs[vs_num].rx_cfg[1] + ((uint16_t)(vs[vs_num].rx_cfg[2] & 0x0f) << 8);
					else
						vs[vs_num].discharge_cell_mask = dummy1 + (uint32_t)(((uint16_t)vs[vs_num].rx_cfg[1] + ((uint16_t)(vs[vs_num].rx_cfg[2] & 0x0f) << 8)) << 12);
					*/
					vs[vs_num].IsFault = vs_faults_check(vs_num);
				}
				step = 8;
			}
			break;
		case 8:
		{
				// Turn on or turn off balancing proccess
				if(GetTimeFrom(discharge_timestamp) > discharge_timer)
				{
					uint16_t discharge_mask = 0;
					uint16_t balanc_limit = 0;
					balanc_flag = 0;
					for(vs_num = 0; vs_num < sens_cnt; vs_num++)
					{
						if((flag == 0) && !vs[0].IsFault && !vs[1].IsFault && !vs[0].BanBalancing && !vs[1].BanBalancing)
							balanc_limit = vs[vs_num].Parameters.BalancingMinVoltage;
						else
							balanc_limit = INT16_MAX;

						discharge_mask = find_cell_for_discharge( (int16_t*)vs[vs_num].m_voltage_array, vs[vs_num].Parameters.CellNumber, vs[vs_num].Parameters.CellTargetVoltage, balanc_limit);
						discharge_mask_set(vs_num, discharge_mask);
						balanc_flag |= (discharge_mask > 0)? 1 : 0;
					}

					discharge_timestamp = GetTimeStamp();
					if(flag == 0)
					{
						flag = 	1;
						discharge_timer = vs[0].Parameters.BalancingTime_s * 1000;
					}
					else
					{
						discharge_timer = 2000;
						flag = 0;
					}
				}

				step = 9;
			}
			break;
		case 9:
				m_timestamp = GetTimeStamp();
				for(vs_num = 0; vs_num < sens_cnt; vs_num++)
				{
					lts_cmd_send(vs_num, WRCFG);
					vs[vs_num].is_ready = 1;
				}

				meas_step = 0;
				// Open connection detection produce every 1 sec
				step = (GetTimeFrom(open_connect_timestamp) >= 1000)? 0 : 2;
				// If balancing is active don't measure cell voltage
				step = (balanc_flag)? 4 : step;					
		break;
	}
		
    return false;
}

/*
* params: *first_meas - point to measuring array
*			cell_num - number of cells 		
* return value: cell number, 0xff if open cells didn't detect
*/
uint8_t OpenConnectionDetectin(int16_t *first_meas, int16_t *second_meas, uint8_t cells_num)
{
	// дл€ первой €чейки условие CELLA(1) < 0 or CELLB(1) < 0
//	if(first_meas[0] < 0 || second_meas[0] < 0)
//		return 1;

//	// дл€ последней €чейки условие CELLA(12) < 0 or CELLB(12) < 0
//	if(first_meas[cells_num - 1] < 0 || second_meas[cells_num - 1] < 0)
//	{
//		return cells_num;
//	}

//	// дл€ остальных €чеек CELLB(n+1) Ц CELLA(n+1) > 200mV
//	for(uint8_t i = 1; i < cells_num - 1; i++)
//	{
//		if((first_meas[i] + 200 < second_meas[i]) || (second_meas[i] == MAX_MEASURING_mV) || (second_meas[i] < 0))
//			return i;
//	}
	
	for(uint8_t i = 0; i < cells_num - 1; i++)
	{
		if(first_meas[i] < 0)
			return i;
	}

	// fault isn't finded
	return 0xff;
}

uint32_t ltc6803_GetDischargingMask(uint8_t vs_num)
{	
	uint32_t discharge_mask = 0;
	
	discharge_mask = (uint16_t)vs[vs_num].rx_cfg[1] + ((uint16_t)(vs[vs_num].rx_cfg[2] & 0x0f) << 8);	
	
	return vs[vs_num].discharge_cell_mask;//discharge_mask;
}

uint8_t ltc6803_GetError(uint8_t vs_num)
{
    return vs[vs_num].FaultCode;
}

//!Function that calculates PEC byte

uint8_t pec8_calc(uint8_t len, uint8_t *data)
{

  uint8_t  remainder = 0x41;//PEC_SEED;


  /*
   * Perform modulo-2 division, a byte at a time.
   */
  for (int byte = 0; byte < len; ++byte)
  {
    /*
     * Bring the next byte into the remainder.
     */
    remainder ^= data[byte];

    /*
     * Perform modulo-2 division, a bit at a time.
     */
    for (uint8_t bit = 8; bit > 0; --bit)
    {
      /*
       * Try to divide the current data bit.
       */
      if (remainder & 128)
      {
        remainder = (remainder << 1) ^ PEC_POLY;
      }
      else
      {
        remainder = (remainder << 1);
      }
    }
  }

  /*
   * The final remainder is the CRC result.
   */
  return (remainder);

}

uint8_t vs_faults_check(uint8_t vs_num)
{
	uint8_t result = 0;
	if(!vs[vs_num].is_ready)
		return result;
	
	// Faults check
	if(vs[vs_num].open_conncetion_cell < 0xff)
	{
		result = 1;
		vs[vs_num].FaultCode = LTC6803_F_OPEN_CONNECTION;
	}
	else if(vs[vs_num].m_ref_volt < MIN_REF_VOLT_LEVEL_MV || vs[vs_num].m_ref_volt > MAX_REF_VOLT_LEVEL_MV)
	{
		result = 1;
		vs[vs_num].FaultCode = LTC6803_F_REF_VOLT;
	}
	else if(vs[vs_num].m_max_pec_attemps >= MAX_PEC_ATTEMPS)
	{
		result = 1;
		vs[vs_num].FaultCode = LTC6803_F_PEC;
	}
	else if(vs[vs_num].volt_fault != 0)
	{
		result = 1;
		vs[vs_num].FaultCode = LTC6803_F_SIGNAL_INVALID;
	}		
	
	return result;
}

void vs_set_min_dis_chars(uint16_t dis_start_volt)
{
	for(uint8_t i = 0; i < VSENS_COUNT; i++)
		vs[i].Parameters.CellTargetVoltage = dis_start_volt;
}

void vs_ban_balancing(uint8_t cmd)
{
	for(uint8_t i = 0; i < VSENS_COUNT; i++)
		vs[i].BanBalancing = cmd;	
}

uint8_t vs_is_available(uint8_t num)
{
	return vs[num].results_is_available;
}
