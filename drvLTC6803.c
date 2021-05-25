
static void lts_cmd_send(const ltc6803_Sensor_t *Handle, ltc_cmds_t cmd);
static uint8_t pec(const uint8_t * data, uint8_t len);
static uint16_t code_to_voltage(uint16_t code);

/* **************************** New IFace *********************************** */

int8_t CellsSensorInit(ltc6803_Sensor_t *Handle, VoltageSensorParams_t *p, int16_t *VoltageArray, int16_t *TempArray)
{
	if(Handle == 0 || p == 0 || VoltageArray == 0 || TempArray == 0)
		return 1;

	Handle.Parameters = *p;

	// Start cmds
	Handle->m_cmd_table[0] = STCVAD;
	Handle->m_dst_ptr_table[0] = NULL;
	Handle->m_src_ptr_table[0] = NULL;
	Handle->m_size_ptr_table[0] = NULL;

	Handle->m_cmd_table[1] = STTMPAD;
	Handle->m_dst_ptr_table[1] = NULL;
	Handle->m_src_ptr_table[1] = NULL;
	Handle->m_size_ptr_table[1] = NULL;

	Handle->m_cmd_table[2] = DAGN;
	Handle->m_dst_ptr_table[2] = NULL;
	Handle->m_src_ptr_table[2] = NULL;
	Handle->m_size_ptr_table[2] = NULL;

	Handle->m_cmd_table[3] = STCVDC;
	Handle->m_dst_ptr_table[3] = NULL;
	Handle->m_src_ptr_table[3] = NULL;
	Handle->m_size_ptr_table[3] = NULL;

	Handle->m_cmd_table[4] = STCLEAR;
	Handle->m_dst_ptr_table[4] = NULL;
	Handle->m_src_ptr_table[4] = NULL;
	Handle->m_size_ptr_table[4] = NULL;

	Handle->m_cmd_table[5] = STOWAD;
	Handle->m_dst_ptr_table[5] = NULL;
	Handle->m_src_ptr_table[5] = NULL;
	Handle->m_size_ptr_table[5] = NULL;

	// Read cmds
	Handle->m_cmd_table[6] = RDCFG;
	Handle->m_dst_ptr_table[6] = Handle->rx_cfg;
	Handle->m_src_ptr_table[6] = NULL;
	Handle->m_size_ptr_table[6] = SZ(Handle->rx_cfg);

	Handle->m_cmd_table[7] = RDDGNR;
	Handle->m_dst_ptr_table[7] = Handle->m_diag_reg;
	Handle->m_src_ptr_table[7] = NULL;
	Handle->m_size_ptr_table[7] = SZ(Handle->m_diag_reg);

	Handle->m_cmd_table[8] = RDTMP;
	Handle->m_dst_ptr_table[8] = Handle->m_tmp_code_arr;
	Handle->m_src_ptr_table[8] = NULL;
	Handle->m_size_ptr_table[8] = SZ(Handle->m_tmp_code_arr);

	Handle->m_cmd_table[9] = RDCV;
	Handle->m_dst_ptr_table[9] = Handle->m_code_arr;
	Handle->m_src_ptr_table[9] = NULL;
	Handle->m_size_ptr_table[9] = SZ(Handle->m_code_arr);

	// Write cmds
	Handle->m_cmd_table[10] = WRCFG;
	Handle->m_dst_ptr_table[10] = NULL;
	Handle->m_src_ptr_table[10] = Handle->m_cfg;
	Handle->m_size_ptr_table[10] = SZ(Handle->m_cfg);

	Handle->v_values = VoltageArray;
	Handle->t_values = TempArray;


	Handle->m_cfg[0] = 0x00;
	Handle->m_cfg[1] = 0x0;
	Handle->m_cfg[2] = 0x0;
	Handle->m_cfg[3] = 0x0;
	Handle->m_cfg[4] = 0x0;
	Handle->m_cfg[5] = 0x0;

	Handle->FaultCode = LTC6803_FAULTS_NONE;
	Handle->IsFault = 0;

	Handle->BanBalancing = 0;

	cdc_mask_set(vs_num, 0x4);
	discharge_mask_set(vs_num, 0x00);
	lts_cmd_send(vs_num, WRCFG);

	return 0;
}

int8_t CellsSensorThread(ltc6803_Sensor_t *Handle)
{
	static uint32_t discharge_timestamp = 0;
	static uint32_t discharge_timer = 0;
	static uint32_t open_connect_timestamp = 0;
	static uint8_t flag = 0;

	switch (Handle->algStep)
	{
	// Open Connection Detection
		case 0:
		{
			Handle->algTimeStamp = GetTimeStamp();

			lts_cmd_send(vs_num, STOWAD);

			Handle->algMsrCnt++;
			Handle->algStep++;
		}
		break;
		case 1:
		if (GetTimeFrom(Handle->algTimeStamp) > 20)
		{
			Handle->algTimeStamp = GetTimeStamp();
			int16_t *target_array = (int16_t *)((Handle->algMsrCnt == 1)? Handle->buffer1 : Handle->buffer1);
			lts_cmd_send(vs_num, RDCV);
			volt_code_parse(null, Handle->Parameters.CellNumber, Handle->m_code_arr, target_array);

			if(Handle->algMsrCnt == 1)
			{
				Handle->open_conncetion_cell = OpenConnectionDetectin(Handle->buffer1, Handle->buffer2, Handle->Parameters.CellNumber);
				Handle->algStep++;
			}
			else
				Handle->algStep = 0;


			open_connect_timestamp = GetTimeStamp();
		}
		break;

		// Get Cells Voltage
		case 2:
			Handle->algTimeStamp = GetTimeStamp();

			Handle->volt_fault = 0;

			if(ltc6803_GetError(vs_num) != LTC6803_FAULTS_NONE)
				Handle->IsFault = 1;

			// start cell voltage measuring
			lts_cmd_send(vs_num, STCVDC);

			Handle->algStep++;
			break;

		// Read cell voltage
		case 3:
			if (GetTimeFrom(Handle->algTimeStamp) > 20)
			{
				Handle->algTimeStamp = GetTimeStamp();
				lts_cmd_send(vs_num, RDCV);
				Handle->volt_fault = volt_code_parse(vs_num, Handle->Parameters.CellNumber, Handle->m_code_arr, Handle->m_voltage_array);

				Handle->algStep++;
			}
			break;
		case 4:
			lts_cmd_send(vs_num, DAGN);

			Handle->algStep++;
		break;
			// Temperature measuring
		case 5:
			if(GetTimeFrom(Handle->algTimeStamp) > 20)
			{
				lts_cmd_send(vs_num, RDDGNR);
				lts_cmd_send(vs_num, STTMPAD);
				uint16_t code = Handle->m_diag_reg[0] + ((uint16_t)(Handle->m_diag_reg[1] & 0x0f) << 8);
				vs[vs_num].m_ref_volt = code_to_voltage(code);

				Handle->algTimeStamp = GetTimeStamp();
				Handle->algStep++;
			}
			break;
		case 6:
			if (GetTimeFrom(Handle->algTimeStamp) > 10)
			{
				// read temperature
				lts_cmd_send(vs_num, RDTMP);
				temp_code_parse(2, vs[vs_num].m_tmp_code_arr, vs[vs_num].m_ref_volt, vs[vs_num].m_tmp_arr);
				lts_cmd_send(vs_num, STCLEAR);

				Handle->algTimeStamp = GetTimeStamp();

				Handle->algStep++;
			}
			break;

			// Get CFG register to find out discharging cell mask
			// Find out cell number for discharge
		case 7:
			{
				Handle->algTimeStamp = GetTimeStamp();

				static uint16_t dummy1 = 0;
				lts_cmd_send(vs_num, RDCFG);

				if(vs_num == 0)
					dummy1 |= (uint16_t)Handle->rx_cfg[1] + ((uint16_t)(Handle->rx_cfg[2] & 0x0f) << 8);
				else
					Handle->discharge_cell_mask = dummy1 + (uint32_t)(((uint16_t)Handle->rx_cfg[1] + ((uint16_t)(Handle->rx_cfg[2] & 0x0f) << 8)) << 12);

				Handle->IsFault = vs_faults_check(vs_num);

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
				for(vs_num = 0; vs_num < VSENS_COUNT; vs_num++)
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
			Handle->algTimeStamp = GetTimeStamp();
			for(vs_num = 0; vs_num < VSENS_COUNT; vs_num++)
				lts_cmd_send(vs_num, WRCFG);

			Handle->algMsrCnt = 0;
			// Open connection detection produce every 1 sec
			step = (GetTimeFrom(open_connect_timestamp) >= 1000)? 0 : 2;
			// If balancing is active don't measure cell voltage
			step = (balanc_flag)? 4 : step;
		break;
	}

	return false;
}


// Private Function

static void lts_cmd_send(const ltc6803_Sensor_t *Handle, ltc_cmds_t cmd) {
    uint8_t cmd_i;
    bool cmd_found = false;
	uint8_t m_max_pec_attemps = 0;

    for (cmd_i = 0; cmd_i < ARRAY_SIZE(Handle->m_cmd_table); cmd_i++) {
        if (cmd == Handle->m_cmd_table[cmd_i]) {
            cmd_found = true;
            break;
        }
    }
    if (cmd_found == false) {
        while (1);
    }

	uint8_t cmd_buffer[4];
	cmd_buffer[0] = ADDR_CMD + Handle->Parameters.ChipAddress;
	cmd_buffer[1] = pec(&cmd_buffer[0], 1);
    cmd_buffer[2] = cmd;
    cmd_buffer[3] = pec(&cmd_buffer[2], 1);

    uint8_t my_pec = 0xAA;
    uint8_t device_pec = 0xBB;

    m_max_pec_attemps = 0;
    if (Handle->m_dst_ptr_table[cmd_i] != NULL)
	{
        while (my_pec != device_pec && m_max_pec_attemps < MAX_PEC_ATTEMPS)
		{
            gpio_ltc6804_cs_set(Handle->Parameters.ChipEnableOut, GPIO_LOW);
            uint8_t byte_cnt = SpiReadWrite(cmd_buffer, 4/*sizeof (cmd_buffer)*/, NULL);
            byte_cnt = SpiReadWrite(NULL, Handle->m_size_ptr_table[cmd_i], Handle->m_dst_ptr_table[cmd_i]);
            byte_cnt = SpiReadWrite(NULL, sizeof (device_pec), &device_pec);
            gpio_ltc6804_cs_set(Handle->Parameters.ChipEnableOut, GPIO_HIGH);
            my_pec = pec(Handle->m_dst_ptr_table[cmd_i], Handle->m_size_ptr_table[cmd_i]);
            m_max_pec_attemps++;
        }
    } else if (Handle->m_src_ptr_table[cmd_i] != NULL)
	{
        my_pec = pec(Handle->m_src_ptr_table[cmd_i], Handle->m_size_ptr_table[cmd_i]);
        gpio_ltc6804_cs_set(Handle->Parameters.ChipEnableOut, GPIO_LOW);
        SpiReadWrite(cmd_buffer, sizeof (cmd_buffer), NULL);
        SpiReadWrite(Handle->m_src_ptr_table[cmd_i], Handle->m_size_ptr_table[cmd_i], NULL);
        SpiReadWrite(&my_pec, sizeof (my_pec), NULL);
        gpio_ltc6804_cs_set(Handle->Parameters.ChipEnableOut, GPIO_HIGH);
    } else
	{
        gpio_ltc6804_cs_set(Handle->Parameters.ChipEnableOut, GPIO_LOW);
        SpiReadWrite(cmd_buffer, sizeof (cmd_buffer), NULL);
        gpio_ltc6804_cs_set(Handle->Parameters.ChipEnableOut, GPIO_HIGH);
    }
}

static uint8_t pec(const uint8_t * data, uint8_t len) {
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

static uint16_t code_to_voltage(uint16_t code) {
    if (code == 0x0fff) {
        return 0xffff;
    } else
    {
		return (3 * (code - 512)) >> 1;		// (code - 512) / 1.5mV
    }
}

static int16_t code_to_tmp(uint16_t code, uint16_t ref_volt)
{
	int16_t tmp;
    uint32_t tmp_volt = code_to_voltage(code);

    tmp_volt *= 100;
    tmp_volt /= ref_volt / 2;
    tmp = interpol(rt_t0_r25_table[0], TERM_ARRAY_LEN, (int16_t) tmp_volt);

    return tmp;
}

static uint8_t volt_code_parse(ltc6803_Sensor_t *Handle, int16_t *res_voltage_array) {
    uint8_t cnt;
    uint16_t code = 0;

	int data_counter = 0;
    uint16_t temp,temp2;
    uint8_t result = 0;

    for (cnt = 0; cnt < Handle->Parameters.CellNumber; cnt = cnt + 2) {

		temp = Handle->m_code_arr[data_counter++];
		temp2 = (uint16_t)(Handle->m_code_arr[data_counter] & 0x0F) << 8;
		code = temp + temp2;

		if(code < 0x0fff)
			Handle->m_voltage_array[cnt] = (3 * (code - 512)) >> 1;
		else
		{
			Handle->m_voltage_array[cnt] = 0xffff;
			result = 1;
		}


		temp2 = (Handle->m_code_arr[data_counter++]) >> 4;
		temp =  (Handle->m_code_arr[data_counter++]) << 4;
		code = temp + temp2;
		//res_voltage_array[cnt+1] = (code < 0x0fff)? (3 * (code - 512)) >> 1 : 0xffff;

		if(code < 0x0fff)
			Handle->m_voltage_array[cnt+1] = (3 * (code - 512)) >> 1;
		else
		{
			Handle->m_voltage_array[cnt+1] = 0xffff;
			result = 1;
		}
    }
	return result;
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
