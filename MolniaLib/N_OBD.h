/* 
 * File:   OBD.h
 * Author: Ag
 *
 * Created on 6 ������� 2015 �., 14:33
 */

#ifndef OBD_H
#define	OBD_H


enum {
			OK_UPDATE_PROFILE = 0,         // �� ��, ��� ��������� ������
			UPDATE_PROFILE_FINISIHED,      // ���������� ���������
			CRC_MISTMATCH_ERROR,           // �� ������� ����������� �����
			INDEX_OUTSIDE_BOUNDSE_ERROR,   // ������ �� ��������� ���������� ������
			SERIES_ERROR,                  // ������ ������������������
			END_OF_COMMUNICATION,          // ����� ����������
			GENERAL_PROGRAMMING_FAILURE,
        };

#define ECAN_NTYPE_NONE			0
#define ECAN_PROFILE_FUN_READ   1
#define ECAN_PROFILE_FUN_WRITE  2
#define ECAN_GET_ARRAY_FUN      3
#define ECAN_DIAG_VALUE_READ	4
		
#define SET_SYSTEM_TIME         0xFE
#define KEEP_ALIVE_FUN          0xFF
/*******************************************************************************
 *                           ��������� �������
*******************************************************************************/
uint8_t ObdThread(CanMsg * msg );
void ObdSendMes(void);
	  
	  
#endif	/* OBD_H */

