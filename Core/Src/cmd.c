//cmd.c

#include "cmd.h"

UART_T m_uart1;
UART_T m_uart2;
UART_T m_uart3;


uint8_t Rx_data3[1];
uint8_t Rx_data2[1];
uint8_t Rx_data1[1];
uint32_t xxx = 1234567890;
void Uart_Init()
{
	HAL_UART_Receive_IT(&huart3, Rx_data3, 1);
	HAL_UART_Receive_IT(&huart2, Rx_data2, 1);
	HAL_UART_Receive_IT(&huart1, Rx_data1, 1);

}

int putchar(int ch)
{
    while(HAL_OK != HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 100))
    {}
    return ch;

}



void Uart_Clear_Rx(UART_T* uart, uint8_t idx)
{
	uart->rxRingBuff[idx][IDX_RX_CMD] = 0;
	uart->rxRingBuff[idx][IDX_RX_DATA] = 0;
	uart->rxCmdChk &= ~(1<<idx);
}


void Uart1_Passing_Pop(int cmd, int data)
{
	uint8_t str[30] = {0,};
	uint8_t len;
	len = sprintf((char *)str,"u1 re %d %d\r\n",cmd, data);
	HAL_UART_Transmit(&huart1,str,len,100);
}
void Uart2_Passing_Pop(int cmd, int data)
{
	uint8_t str[30] = {0,};
	uint8_t len;
	len = sprintf((char *)str,"u2 re %d %d\r\n",cmd, data);
	HAL_UART_Transmit(&huart2,str,len,100);
}
void Uart3_Passing_Pop(int cmd, int data)
{
	uint8_t str[30] = {0,};
	uint8_t len;
	len = sprintf((char *)str,"u3 re %d %d\r\n",cmd, data);
	HAL_UART_Transmit(&huart3,str,len,100);
}

//
void UartRx1DataProcess()
{
	int cmd;
	int data;
	if(m_uart1.rxCmdChk == 0x0000)return;

	for(int i =0 ;i < 10;i++)
	{
		if(m_uart1.rxRingBuff[i][IDX_RX_CMD] !=0)
		{
			cmd = m_uart1.rxRingBuff[i][IDX_RX_CMD];
			data = m_uart1.rxRingBuff[i][IDX_RX_DATA];
			Uart1_Passing_Pop(cmd, data);
			Uart_Clear_Rx(&m_uart1, i);
		}

	}
}




uint8_t chNum = 0;
uint8_t chNumMax = 1;

CHIMNEY_T m_ch[4];
CMD_T m_Gcmd;

TIME_T m_time;


uint32_t workPlaceCode;//사업장코드[공통]










uint8_t txAllBuff[200];
uint8_t TFDH_4_Buff[3][80]; // item 세개 까지
uint8_t TFDH_4Cnt;
uint8_t TFDH_4Num;

uint8_t TDUH_5step = STEP0;

uint8_t TDUH_5_5_Buff[3][80]; // item 세개 까지
uint8_t TDUH_5_5_Cnt;
uint8_t TDUH_5_5_Num;

uint8_t TDUH_5_6_Buff[3][80]; // item 세개 까지
uint8_t TDUH_5_6_Cnt;
uint8_t TDUH_5_6_Num;

uint8_t TDUH_5_2_Buff[3][80]; // item 세개 까지
uint8_t TDUH_5_2_Cnt;
uint8_t TDUH_5_2_Num;

uint8_t TDUH_5_3_Buff[3][80]; // item 세개 까지
uint8_t TDUH_5_3_Cnt;
uint8_t TDUH_5_3_Num;


#define CRC16_INIT_VALUE 0xffff
#define CRC16_XOR_VALUE 0x0000

static unsigned short crctable[256] = {
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};



void Save_30Day_1_TDAH_Fail()
{

}

void Save_30Day_1_TDAH()
{

}
void Save_30Day_2_TOFH()
{

}
void Save_30Day_3_TDDH()
{

}
void Save_30Day_6_TNOH()
{

}

void TX_ACK(uint8_t cmd)
{
    uint8_t msg[1] = MSG_ACK;
    HAL_UART_Transmit(&huart1,msg,1,100);
    printf("ACK\r\n");

	m_Gcmd.ID = cmd;
    m_Gcmd.txAckTimeStamp = HAL_GetTick();
    m_Gcmd.txAckFlag = 1;
}
void ACK_ReSend()
{
	uint8_t msg[1] = MSG_ACK;
	if(m_Gcmd.txAckFlag)
	{
		if(HAL_GetTick() - m_Gcmd.txAckTimeStamp>30000 )
		{

			switch (m_Gcmd.stepNoneAck)
			{
				case STEP0:
					HAL_UART_Transmit(&huart1,msg,1,100);
					printf("ACK\r\n");
					m_Gcmd.stepNoneAck = STEP1;
				break;

				case STEP1://미송신
					m_Gcmd.txAckFlag = 0;
					m_Gcmd.ID = 0;
					m_Gcmd.stepNoneAck = STEP0;
				break;
			}
			m_Gcmd.txAckTimeStamp = HAL_GetTick();
		}
	}


}
void TxMsg_ReSend()
{
	if(m_Gcmd.txMsgFlag)
	{
		if(HAL_GetTick() - m_Gcmd.txMsgTimeStamp>30000 )
		{

			switch (m_Gcmd.stepNoneMsg)
			{
				case STEP0:
					m_Gcmd.txCmd = m_Gcmd.ID;
					m_Gcmd.stepNoneMsg = STEP1;
				break;

				case STEP1://미송신
					if(m_Gcmd.ID == ID_TDAH_1)
					{
						//미송신 데이터 SD CARD 저장
						Save_30Day_1_TDAH_Fail();
					}
					m_Gcmd.txMsgFlag = 0;
					m_Gcmd.ID = 0;
					m_Gcmd.stepNoneMsg = STEP0;
				break;
			}
			m_Gcmd.txMsgTimeStamp = HAL_GetTick();
		}
	}
}
void TX_NAK()
{
    uint8_t msg[1] = MSG_NAK;
    HAL_UART_Transmit(&huart1,msg,1,100);
    printf("NAK\r\n");
}
void TX_EOT()
{
    uint8_t msg[1] = MSG_EOT;
    HAL_UART_Transmit(&huart1,msg,1,100);
    printf("EOT\r\n");
}

void TX_Memo(uint8_t cmd)
{
	m_Gcmd.ID = cmd;
	m_Gcmd.txMsgTimeStamp = HAL_GetTick();
	m_Gcmd.txMsgFlag = 1;
}

void TDUH_5_ACK()
{
	switch (TDUH_5step)
	{
		case STEP0:
			if(TDUH_5_5_Num -1 > TDUH_5_5_Cnt)
			{
				TDUH_5_5_Cnt++;
				m_Gcmd.txCmd = ID_TDUH_5;
			}
			else
			{
				TX_EOT();
				m_Gcmd.txCmd = ID_TDUH_5;
				if(TDUH_5_6_Num) TDUH_5step = STEP1;
				else if(TDUH_5_2_Num) TDUH_5step = STEP2;
				else TDUH_5step = STEP3;
			}
		break;

		case STEP1:
			if(TDUH_5_6_Num -1 > TDUH_5_6_Cnt)
			{
				TDUH_5_6_Cnt++;
				m_Gcmd.txCmd = ID_TDUH_5;
			}
			else
			{
				TX_EOT();
				m_Gcmd.txCmd = ID_TDUH_5;
				if(TDUH_5_2_Num) TDUH_5step = STEP2;
				else TDUH_5step = STEP3;
			}
		break;

		case STEP2:
			if(TDUH_5_2_Num -1 > TDUH_5_2_Cnt)
			{
				TDUH_5_2_Cnt++;
				m_Gcmd.txCmd = ID_TDUH_5;
			}
			else
			{
				TX_EOT();
				m_Gcmd.txCmd = ID_TDUH_5;
				TDUH_5step = STEP3;
			}
		break;

		case STEP3:
			if(TDUH_5_3_Num -1 > TDUH_5_3_Cnt)
			{
				TDUH_5_3_Cnt++;
				m_Gcmd.txCmd = ID_TDUH_5;
			}
			else
			{
				TX_EOT();
				TDUH_5step = STEP0;
			}
		break;
	}

}
void Rx_Passing_ACK()
{
	printf("RX ACK \r\n");
	switch (m_Gcmd.ID)
	{
		case ID_TDAH_1 :
			if(m_ch[0].transferMode == TXMODE_HAF_NUM)
			{
				TX_EOT();
			}
			else if(m_ch[0].transferMode == TXMODE_HAF_NUM)
			{
				TX_EOT();
				for(int i =0 ;i < m_ch[0].itemNum; i++)
				{
					if((m_ch[0].item[i].protectStatus[HAF_IDX] != NORMAL) ||m_ch[0].item[i].operStatus[HAF_IDX])
					{
						m_Gcmd.txCmd = ID_TNOH_6;
					}
				}
			}
			else if(m_ch[0].transferMode == TXMODE_ALL_NUM)
			{
				if(m_ch[0].itemMode == HAF_IDX && m_ch[0].itemAllFlag)
				{
					m_ch[0].itemAllFlag = 0;
					m_ch[0].itemMode = FIV_IDX;
					m_Gcmd.txCmd = ID_TDAH_1;
				}
				else
				{
					TX_EOT();
				}

			}

		break;

		case ID_TOFH_2:

			if(m_ch[0].tohTotalDay > m_ch[0].tohfDayCnt)
			{
				m_ch[0].tohfDayCnt++;
				m_Gcmd.txCmd = ID_TOFH_2;
			}
			else
			{
				m_Gcmd.tofhDone = 1;
				TX_EOT();
			}

		break;

		case ID_TDDH_3:
			if(m_ch[0].transferMode == TXMODE_ALL_NUM)
			{
				if(m_ch[0].itemMode == HAF_IDX && m_ch[0].itemAllFlag)
				{
					m_ch[0].itemAllFlag = 0;
					m_ch[0].itemMode = FIV_IDX;
					m_Gcmd.txCmd = ID_TDDH_3;
				}
				else
				{
					TX_EOT();
				}
			}
		break;

		case ID_TFDH_4:
			if(TFDH_4Num -1 > TFDH_4Cnt)
			{
				TFDH_4Cnt++;
				m_Gcmd.txCmd = ID_TFDH_4;
			}
			else
			{
				TX_EOT();
			}

		break;

		case ID_TDUH_5:
			TDUH_5_ACK();
		break;



		case ID_TNOH_6 :
		case ID_TUPG_10:
		case ID_TVER_11:
		case ID_TFCR_15:
		case ID_TFCR_16:
		case ID_TCN2_20:
			TX_EOT();
		break;


	}
	m_Gcmd.ID = 0;
	m_Gcmd.txMsgFlag = 0;


	m_Gcmd.stepNoneAck = STEP0;
	m_Gcmd.stepNoneMsg = STEP0;
	m_Gcmd.stepNoneNck = STEP0;

}
void Rx_Passing_NAK()
{
	printf("RX NAK \r\n");
	if(m_Gcmd.txMsgFlag)
	{
		switch (m_Gcmd.stepNoneNck)
		{
			case STEP0:
				m_Gcmd.txCmd = m_Gcmd.ID;
				m_Gcmd.stepNoneNck = STEP1;
			break;

			case STEP1://미송신
				if(m_Gcmd.ID == ID_TDAH_1)
				{
					//미송신 데이터 SD CARD 저장
					Save_30Day_1_TDAH_Fail();
				}
				m_Gcmd.txMsgFlag = 0;
				m_Gcmd.ID = 0;
				m_Gcmd.stepNoneNck = STEP0;
			break;
		}
	}

	m_Gcmd.txCmd = m_Gcmd.ID;
}
void Rx_Passing_EOT()// 내가 서버로부터 바디가 있는 데이터를 받았을때만 해당
{
	printf("RX EOT \r\n");
	m_Gcmd.txAckFlag = 0;
	switch (m_Gcmd.ID)
	{
	    case ID_PFST_7 ://TCN2
	    case ID_PSEP_8 ://TCN2
	    case ID_PFCC_13://TCN2
	    case ID_PAST_14://TCN2
	    case ID_PRSI_17://TCN2
	    case ID_PDAT_18://TCN2
	    case ID_PODT_19://TCN2
	    case ID_PCN2_20://TCN2
	    case ID_PRBT_22://TCN2
			m_Gcmd.txCmd = ID_TCN2_20;
		break;

	    case ID_PTIM_9 ://END
	    case ID_PSET_12://END
			//notting
		break;

	    case ID_PUPG_10://PUPG
			m_Gcmd.txCmd = ID_TUPG_10;
		break;
	    case ID_PFRS_16://TFCR
			m_Gcmd.txCmd = ID_TFCR_16;
		break;

	}
	m_Gcmd.ID = 0;
}


void append_crc16(uint8_t *buff, uint16_t idx)
{
    uint16_t crc = CRC16_INIT_VALUE; // 0xFFFF
    uint16_t length = idx;

    for (uint16_t i = 0; i < length; i++) {
        uint8_t index = (crc >> 8) ^ buff[i];
        crc = (crc << 8) ^ crctable[index];
    }
    crc = crc ^ CRC16_XOR_VALUE;

    // [핵심 Fix] 문자열 변환 없이 순수 바이너리 데이터를 직접 버퍼에 삽입합니다.
    // (보통 상위 바이트를 먼저 보내는 Big-Endian 방식을 많이 씁니다)
    buff[idx]     = (crc >> 8) & 0xFF; // CRC 상위 1바이트
    buff[idx + 1] = crc & 0xFF;        // CRC 하위 1바이트
}
uint8_t Check_crc16(uint8_t *buff, uint16_t idx)
{
    uint16_t crc = CRC16_INIT_VALUE; // 0xFFFF
    uint16_t length = idx;

    for (uint16_t i = 0; i < length; i++) {
        uint8_t index = (crc >> 8) ^ buff[i];
        crc = (crc << 8) ^ crctable[index];
    }
    crc = crc ^ CRC16_XOR_VALUE;

    uint16_t crcTail = (buff[idx]<<8)|(buff[idx+1]);
    if(crc == crcTail)
    {
        return 0;
    }
    else
    {
        printf("CRC ERR\r\n");
        return 1;
    }
}



void TxAllBuff_Clear()
{
    memset(txAllBuff, 0, sizeof(txAllBuff));
    m_Gcmd.txCnt = 0;
    m_Gcmd.eotTx = 0;
}

void TxStr_Faci_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data)
{
    char str[16] = {0};
    int len = 0;
    printf("%s ->",debugStr);

    if      (data / 10000 == 1) str[0] = 'E';
    else if (data / 10000 == 2) str[0] = 'P';
    else if (data / 10000 == 3) str[0] = 'F';
    else return;

    // 나머지 4자리를 0패딩 (예: 14221 → "4221", 10005 → "0005")
    len = snprintf(str + 1, sizeof(str) - 1, "%04u", (unsigned)(data % 10000));
    if (len < 0) return;

    memcpy(txAllBuff + idx, str, fixLen);  // +1: str[0] 접두 문자 포함
    printf("%s\r\n",str);
}

void TxStr_chimCode_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data)
{
    char str[5] = {0};
    printf("%s ->",debugStr);

    snprintf(str, sizeof(str), "%03u", data);

    memcpy(txAllBuff + idx, str, 3);
    printf("%s\r\n",str);
}
void TxStr_Item_Code_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data)
{
    char str[3] = {0};
    printf("%s ->",debugStr);

    switch (data)
    {
        case 1: str[0] = 'A'; break;
        case 2: str[0] = 'D'; break;
        case 3: str[0] = 'T'; break;
        case 4: str[0] = 'H'; break;
        case 5: str[0] = 'a'; break;
        case 6: str[0] = 'b'; break;
        default:
            printf("error \r\n");
        break;
    }
    memcpy(txAllBuff + idx, str, 1);
    printf("%s\r\n",str);
}
void TxStr_IP_Input_org(char* debugStr, uint16_t idx, uint16_t fixLen, uint8_t* buff)
{
    char str[16] = {0};
    printf("%s ->",debugStr);
    snprintf(str, sizeof(str), "%03u.%03u.%03u.%03u",
             buff[0], buff[1], buff[2], buff[3]);

    memcpy(txAllBuff + idx, str, fixLen);  // 점 포함 15바이트 (널 제외)
    printf("%s\r\n",str);
}

void TxStr_IP_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint8_t* buff)
{
    char str[16] = {0};
    uint8_t encryBuff[16] = {0,};
    printf("%s ->",debugStr);
    snprintf(str, sizeof(str), "%03u.%03u.%03u.%03u",
             buff[0], buff[1], buff[2], buff[3]);

    Greenlink_Encrypt((uint8_t*)str, 15, encryBuff);

    memcpy(txAllBuff + idx, encryBuff, fixLen);  // 점 포함 15바이트 (널 제외)
    printf("%s\r\n",str);
}
void TxStr_PW_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data)
{
    char str[16] = {0};
    uint8_t encryBuff[16] = {0,};
	int len = 0, termLen = 0;

    printf("%s ->",debugStr);

	len = snprintf(str, sizeof(str), "%d",data);


    Greenlink_Encrypt((uint8_t*)str, LEN_TCN2_21_11_10, encryBuff);

    memcpy(txAllBuff + idx, encryBuff, fixLen);  // 점 포함 15바이트 (널 제외)
    printf("%s\r\n",str);
}

void TxStr_TxMode_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint8_t data)
{
    char str[5] ={0,};
    printf("%s ->",debugStr);
    switch (data)
    {
        case TXMODE_HAF_NUM:
            memcpy(str, TXMODE_HAF, fixLen);
        break;

        case TXMODE_FIV_NUM:
            memcpy(str, TXMODE_FIV, fixLen);
        break;

        case TXMODE_ALL_NUM:
            memcpy(str, TXMODE_ALL, fixLen);
        break;
    }


    memcpy(txAllBuff + idx, str, fixLen);
    printf("%s\r\n",str);
}

void TxStr_Int_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data )
{
	char str[16] = {0,};
	int len = 0, termLen = 0;

    printf("%s ->",debugStr);
	len = snprintf(str, sizeof(str), "%d",data);
//	printf("%s : idx:%d, fixLen:%d, data:%d\r\n",debugStr, idx, fixLen, data);
	if(len > fixLen)
	{
		//error
		printf("error: data too long for fixLen\r\n");
		return;
	}
	else if(len < fixLen)
	{
		termLen = fixLen - len;

		for(int i =0 ;i < termLen;i++)
		{
			txAllBuff[idx++] = ' ';
		}
		memcpy(txAllBuff+idx, str, len);

	}
	else if(len == fixLen)
	{
		memcpy(txAllBuff+idx, str, fixLen);
	}
    printf("%s\r\n",str);

}
void TxStr_float_Input(char* debugStr, uint16_t idx, uint16_t fixLen, float data)
{
	char str[24] = {0,};
	int len = 0, termLen = 0;
	float roundedData = roundf(data * 100.0f) / 100.0f;

    printf("%s ->",debugStr);
	len = snprintf(str, sizeof(str), "%.2f", roundedData);


	if (len > fixLen)
	{
		printf("error: data too long for fixLen\r\n");
		return;
	}
	else if (len < fixLen)
	{
		termLen = fixLen - len;

		for (int i = 0; i < termLen; i++)
		{
			txAllBuff[idx++] = ' ';
		}
		memcpy(txAllBuff + idx, str, len);
	}
	else if (len == fixLen)
	{
		memcpy(txAllBuff + idx, str, fixLen);
	}
	printf("%s\r\n",str);
}

void TxStr_Str_Input(char* debugStr, uint16_t idx, uint16_t fixLen, char* str )
{
    printf("%s ->",debugStr);
    memcpy(txAllBuff+idx, str, fixLen);
    printf("%s\r\n",str);
}




// ========================================================================================
// [1] 측정자료 전송 (TDAH) - 가변 구조
// ========================================================================================
void Tx_1_TDAH(uint8_t ch)
{

	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,      LEN_COMM_1_4,      CMD_TDAH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      ch);
	TxStr_Int_Input("allLan",     IDX_COMM_4,        LEN_COMM_4_4,      TOTAL_LEN_TDAH1(m_ch[ch].itemNum));
	TxStr_TxMode_Input("transferMode",     IDX_COMM_5,    LEN_COMM_5_3,      m_ch[ch].transferMode);

	// 바디 (고정)
	TxStr_Int_Input("measureTime",  IDX_TDAH1_6,   LEN_TDAH1_6_10,    m_ch[ch].measureTime );
	TxStr_Int_Input("measureQty",    IDX_TDAH1_7,   LEN_TDAH1_7_2,     m_ch[ch].itemNum);

	// 바디 (가변)
	int commIdx;
	uint8_t tf_idx = m_ch[ch].itemMode;
	for(int i = 0; i < m_ch[ch].itemNum; i++)
	{
		commIdx = i * IDX_TDAH1_CYCLE;

		TxStr_Faci_Input("facCode",     commIdx + IDX_TDAH1_8n,  LEN_TDAH1_8n_5,  m_ch[ch].item[i].facCode);
		TxStr_Item_Code_Input("itemCode", commIdx + IDX_TDAH1_9n,  LEN_TDAH1_9n_1,  m_ch[ch].item[i].itemCode);
		TxStr_float_Input("measureValue", commIdx + IDX_TDAH1_10n,  LEN_TDAH1_10n_6, m_ch[ch].item[i].value[tf_idx]);
		TxStr_Int_Input("measureStatus",  commIdx + IDX_TDAH1_11n,  LEN_TDAH1_11n_1, m_ch[ch].item[i].status[tf_idx]);
		TxStr_Int_Input("operStatus",     commIdx + IDX_TDAH1_12n,  LEN_TDAH1_12n_1, m_ch[ch].item[i].operStatus[tf_idx]);
		TxStr_Int_Input("protectStatus",    commIdx + IDX_TDAH1_13n,  LEN_TDAH1_13n_1, m_ch[ch].item[i].protectStatus[tf_idx]);
	}

	// 테일러 (CRC)
	uint16_t crcidx = m_ch[ch].itemNum* IDX_TDAH1_CYCLE + IDX_TDAH1_8n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	HAL_UART_Transmit(&huart1, (uint8_t*)txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TDAH_1);
	Save_30Day_1_TDAH();

}

// ========================================================================================
// [2] 전원단절구간자료 전송 (TOFH) - 가변 구조
// ========================================================================================
void Tx_2_TOFH(uint8_t ch)
{
	if(m_Gcmd.tofhDone) return; // 처음 한번만 사용하고 다음엔 사용안함

	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TOFH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      ch);
	TxStr_TxMode_Input("transferMode", IDX_COMM_5,    LEN_COMM_5_3,      m_ch[ch].transferMode);

	uint8_t dayCnt = m_ch[ch].tohfDayCnt;
	uint8_t tf_idx = m_ch[ch].itemMode;

	// 바디 (고정)
	TxStr_Int_Input("powerOffDay",  IDX_TOFH2_6,   LEN_TOFH2_6_8,     m_ch[ch].tohfDayBuff[dayCnt]);
	// 바디 (가변)
	int commIdx = 0;
	uint16_t idxCnt = 0;
	uint8_t tfMode;
	if(m_ch[ch].itemMode == TXMODE_FIV_NUM) tfMode = 5;
	else if(m_ch[ch].itemMode == TXMODE_HAF_NUM) tfMode = 30;
	uint16_t startTime = m_ch[ch].tohfTimeSE[dayCnt][0];
	uint16_t endTime = m_ch[ch].tohfTimeSE[dayCnt][1];

	for(int i = startTime; i <= endTime; i =i+tfMode)
	{
		if((i % 100) >= 60)
		{
			i += 40;
		}


		if(tf_idx == HAF_IDX && (i%100))
		{

		}
		else if(tf_idx == FIV_IDX)
		{

		}
		commIdx = idxCnt * IDX_TOFH2_CYCLE;
		TxStr_Int_Input("powerOffTime", commIdx + IDX_TOFH2_8n, LEN_TOFH2_8n_4, i);
		idxCnt++;
	}
	TxStr_Int_Input("powerOffCnt",  IDX_TOFH2_7,   LEN_TOFH2_7_3,     idxCnt);
	TxStr_Int_Input("allLan",		  IDX_COMM_4,	 LEN_COMM_4_4,		TOTAL_LEN_TOFH2(idxCnt));


	// 테일러 (CRC)
	uint16_t crcidx = idxCnt* IDX_TOFH2_CYCLE + IDX_TOFH2_8n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	HAL_UART_Transmit(&huart1, (uint8_t*)txAllBuff, m_Gcmd.txCnt, 100);
	Save_30Day_2_TOFH();
	TX_Memo(ID_TOFH_2);
	printf("> END \r\n");
}


// ========================================================================================
// [3] 일일 마감자료 전송 (TDDH) - 가변 구조
// ========================================================================================
void Tx_3_TDDH(uint8_t ch)
{

	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TDDH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      ch);
	TxStr_Int_Input("allLan",         IDX_COMM_4,    LEN_COMM_4_4,      TOTAL_LEN_TDDH3(m_ch[ch].itemNum));
	TxStr_TxMode_Input("transferMode",  IDX_COMM_5,    LEN_COMM_5_3,      m_ch[ch].transferMode);

	// 바디 (고정)
	TxStr_Int_Input("closeDate",    IDX_TDDH3_6,   LEN_TDDH3_6_8,     m_ch[ch].closeDate);
	TxStr_Int_Input("dayCnt",       IDX_TDDH3_7,   LEN_TDDH3_7_3,     m_ch[ch].dayCnt);
	TxStr_Int_Input("TDAHcnt",      IDX_TDDH3_8,   LEN_TDDH3_8_3,     m_ch[ch].TDAHcnt);
	TxStr_Int_Input("TOFHcnt",      IDX_TDDH3_9,   LEN_TDDH3_9_3,     m_ch[ch].TOFHcnt);
	TxStr_Int_Input("measureQty",   IDX_TDDH3_10,  LEN_TDDH3_10_2,    m_ch[ch].itemNum );

	// 바디 (가변)
	int commIdx;
	uint8_t tf_idx = m_ch[ch].itemMode;
	for(int i = 0; i < m_ch[ch].itemNum; i++)
	{
		commIdx = i * IDX_TDDH3_CYCLE;

		TxStr_Faci_Input("facCode",     commIdx + IDX_TDDH3_11n,  LEN_TDDH3_11n_5, m_ch[ch].item[i].facCode);
		TxStr_Item_Code_Input("itemCode", commIdx + IDX_TDDH3_12n,  LEN_TDDH3_12n_1, m_ch[ch].item[i].itemCode);
		TxStr_Int_Input("nomalCnt",     commIdx + IDX_TDDH3_13n,  LEN_TDDH3_13n_3, m_ch[ch].item[i].nomalCnt[tf_idx] );
		TxStr_Int_Input("FultCnt",      commIdx + IDX_TDDH3_14n,  LEN_TDDH3_14n_3, m_ch[ch].item[i].FultCnt[tf_idx]);
		TxStr_Int_Input("commuErrCnt",  commIdx + IDX_TDDH3_15n,  LEN_TDDH3_15n_3, m_ch[ch].item[i].commuErrCnt[tf_idx]);
		TxStr_Int_Input("powerOffCnt",  commIdx + IDX_TDDH3_16n,  LEN_TDDH3_16n_3, m_ch[ch].item[i].powerOffCnt[tf_idx]);
		TxStr_Int_Input("fixCnt",       commIdx + IDX_TDDH3_17n,  LEN_TDDH3_17n_3, m_ch[ch].item[i].fixCnt[tf_idx]);
	}

	// 테일러 (CRC)
	uint16_t crcidx = m_ch[ch].itemNum * IDX_TDDH3_CYCLE + IDX_TDDH3_11n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TDDH_3);
	Save_30Day_3_TDDH();
}

// ========================================================================================
// [4] 미전송자료 전송 (TFDH) - 가변 구조
// ========================================================================================
void Tx_4_TFDH(uint8_t ch)
{

	uint8_t len = TOTAL_LEN_TFDH4(m_ch[ch].itemNum);
	memcpy(txAllBuff, TFDH_4_Buff[TFDH_4Cnt], len);
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TFDH);

	uint16_t crcidx = m_ch[ch].itemNum* IDX_TFDH4_CYCLE + IDX_TFDH4_8n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;

	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TFDH_4);
}

// ========================================================================================
// [5] 저장자료 응답 (TDUH) - 가변 구조
// ========================================================================================
void Tx_5_TDUH(uint8_t ch)
{
	uint32_t len;
	uint16_t crcidx;
	switch (TDUH_5step)
	{
		case STEP0:
			 len = TOTAL_LEN_TDUH5(m_ch[ch].itemNum);
			memcpy(txAllBuff, TDUH_5_5_Buff[TDUH_5_5_Cnt], len);
			TxStr_Str_Input("cmd",			  IDX_COMM_1,	 LEN_COMM_1_4,		CMD_TDUH);

			 crcidx = m_ch[ch].itemNum* IDX_TDUH5_CYCLE + IDX_TDUH5_8n;
		    append_crc16(txAllBuff, crcidx);
		    m_Gcmd.txCnt = crcidx+LEN_CRC;
			HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
			printf("> END \r\n");
			TX_Memo(ID_TDUH_5);
		break;

		case STEP1:
			 len = TOTAL_LEN_TNOH6(m_ch[ch].itemNum);
			memcpy(txAllBuff, TDUH_5_6_Buff[TDUH_5_6_Cnt], len);
			TxStr_Str_Input("cmd",			  IDX_COMM_1,	 LEN_COMM_1_4,		CMD_TNOH);
			 crcidx = m_ch[ch].itemNum* IDX_TNOH6_CYCLE + IDX_TNOH6_8n;
		    append_crc16(txAllBuff, crcidx);
		    m_Gcmd.txCnt = crcidx+LEN_CRC;
			HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
			printf("> END \r\n");
			TX_Memo(ID_TDUH_5);
		break;

		case STEP2:

			if(strtol_n(TDUH_5_2_Buff[TDUH_5_2_Cnt], &len, IDX_TOFH2_7, LEN_TOFH2_7_3, 1, 288, VIEW_ADD_1))return;

			memcpy(txAllBuff, TDUH_5_2_Buff[TDUH_5_2_Cnt], len);
			TxStr_Str_Input("cmd",			  IDX_COMM_1,	 LEN_COMM_1_4,		CMD_TOFH);
			 crcidx = len* IDX_TOFH2_CYCLE + IDX_TOFH2_8n;
		    append_crc16(txAllBuff, crcidx);
		    m_Gcmd.txCnt = crcidx+LEN_CRC;
			HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
			printf("> END \r\n");
			TX_Memo(ID_TDUH_5);
		break;

		case STEP3:
			 len = TOTAL_LEN_TDDH3(m_ch[ch].itemNum);
			memcpy(txAllBuff, TDUH_5_3_Buff[TDUH_5_3_Cnt], len);
			TxStr_Str_Input("cmd",			  IDX_COMM_1,	 LEN_COMM_1_4,		CMD_TDDH);
			 crcidx = len* IDX_TDDH3_CYCLE + IDX_TDDH3_11n;
		    append_crc16(txAllBuff, crcidx);
		    m_Gcmd.txCnt = crcidx+LEN_CRC;
			HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
			printf("> END \r\n");
			TX_Memo(ID_TDUH_5);
		break;
	}

}

// ========================================================================================
// [6] 5분자료 전송대상 정보 (TNOH) - 가변 구조
// ========================================================================================
void Tx_6_TNOH(uint8_t ch)
{

	if(m_ch[ch].transferMode != TXMODE_HAF_NUM) return;

	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TNOH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      ch);
	TxStr_TxMode_Input("transferMode", IDX_COMM_5,   LEN_COMM_5_3,      TXMODE_HAF_NUM);

	// 바디 (고정)
	TxStr_Int_Input("measureTime",   IDX_TNOH6_6,   LEN_TNOH6_6_10,    m_ch[ch].measureTime);
	TxStr_Int_Input("measureQty",    IDX_TNOH6_7,   LEN_TNOH6_7_2,     m_ch[ch].itemNum);

	// 바디 (가변)



	int commIdx;
	uint8_t tf_idx = m_ch[ch].itemMode;
	uint8_t cnt = 0;
	for(int i = 0; i < m_ch[ch].itemNum; i++)
	{
		if(m_ch[0].item[i].protectStatus[tf_idx] != NORMAL ||m_ch[ch].item[i].operStatus[tf_idx])
		{
			cnt++;
			commIdx = i * IDX_TNOH6_CYCLE;

			TxStr_Faci_Input("facCode",     commIdx + IDX_TNOH6_8n, LEN_TNOH6_8n_5,  m_ch[ch].item[i].facCode);
			TxStr_Item_Code_Input("itemCode", commIdx + IDX_TNOH6_9n, LEN_TNOH6_9n_1,  m_ch[ch].item[i].itemCode);
			TxStr_Int_Input("operStatus",   commIdx + IDX_TNOH6_10n, LEN_TNOH6_10n_1, m_ch[ch].item[i].operStatus[tf_idx]);
			TxStr_Int_Input("protectStatus",  commIdx + IDX_TNOH6_11n, LEN_TNOH6_11n_1, m_ch[ch].item[i].protectStatus[tf_idx]);
		}
	}
	TxStr_Int_Input("allLan",		  IDX_COMM_4,	 LEN_COMM_4_4,		TOTAL_LEN_TNOH6(cnt));

	// 테일러 (CRC)
	uint16_t crcidx = cnt * IDX_TNOH6_CYCLE + IDX_TNOH6_8n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TNOH_6);
	Save_30Day_6_TNOH();
}

// ========================================================================================
// [9] 서버시간 조회 요청 (TTIM) - 바디 없는 고정 길이 구조
// ========================================================================================
void Tx_9_TTIM(uint8_t ch)
{

	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TTIM);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      ch);
	TxStr_Int_Input("allLan",         IDX_COMM_4,    LEN_COMM_4_4,      TOTAL_LEN_TTIM9);

    append_crc16(txAllBuff, IDX_TTIM9_CRC);
    m_Gcmd.txCnt = IDX_TTIM9_CRC+LEN_CRC;
	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TTIM_9);
}

// ========================================================================================
// [10] 게이트웨이 업그레이드 결과 전송 (TUPG) - 고정 길이 구조
// ========================================================================================
void Tx_10_TUPG(uint8_t ch)
{
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,     LEN_COMM_1_4,     CMD_TUPG);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,     LEN_COMM_2_7,     workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,     LEN_COMM_3_3,     ch);
	TxStr_Int_Input("allLan",         IDX_COMM_4,     LEN_COMM_4_4,     TOTAL_LEN_TUPG10);

	// 바디
	TxStr_IP_Input("IP",           IDX_TUPG10_5,   LEN_TUPG10_5_16,  m_ch[ch].IP);
	TxStr_IP_Input("GWip",         IDX_TUPG10_6,   LEN_TUPG10_6_16,  m_ch[ch].GWip);
	TxStr_Int_Input("manuCode",    IDX_TUPG10_7,   LEN_TUPG10_7_2,   m_ch[ch].manuCode);
	TxStr_Str_Input("GWmodel",     IDX_TUPG10_8,   LEN_TUPG10_8_20,  m_ch[ch].GWmodel);
	TxStr_Str_Input("fwVer",       IDX_TUPG10_9,   LEN_TUPG10_9_20,  m_ch[ch].fwVer);
	TxStr_Str_Input("heshCode",    IDX_TUPG10_10,  LEN_TUPG10_10_32,  m_ch[ch].heshCode);
    append_crc16(txAllBuff, IDX_TUPG10_CRC);

    m_Gcmd.txCnt = IDX_TUPG10_CRC+LEN_CRC;
	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TUPG_10);
}

// ========================================================================================
// [11] 버전정보 응답 전송 (TVER) - 고정 길이 구조
// ========================================================================================
void Tx_11_TVER(uint8_t ch)
{
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,     LEN_COMM_1_4,     CMD_TVER);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,     LEN_COMM_2_7,     workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,     LEN_COMM_3_3,     ch);
	TxStr_Int_Input("allLan",         IDX_COMM_4,     LEN_COMM_4_4,     TOTAL_LEN_TVER11);

	// 바디
	TxStr_IP_Input("IP",           IDX_TVER11_5,   LEN_TVER11_5_16,  m_ch[ch].IP);
	TxStr_IP_Input("GWip",         IDX_TVER11_6,   LEN_TVER11_6_16,  m_ch[ch].GWip);
	TxStr_Int_Input("manuCode",    IDX_TVER11_7,   LEN_TVER11_7_2,   m_ch[ch].manuCode);
	TxStr_Str_Input("GWmodel",     IDX_TVER11_8,   LEN_TVER11_8_20,  m_ch[ch].GWmodel);
	TxStr_Str_Input("fwVer",       IDX_TVER11_9,   LEN_TVER11_9_20,  m_ch[ch].fwVer);
	TxStr_Str_Input("heshCode",    IDX_TVER11_10,  LEN_TVER11_10_32,  m_ch[ch].heshCode);
	append_crc16(txAllBuff, IDX_TVER11_CRC);
    m_Gcmd.txCnt = IDX_TVER11_CRC+LEN_CRC;
	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TVER_11);
}

// ========================================================================================
// [15] 방지시설 정상여부 관계정보 조회 응답 (TFCR) - 가변 구조
// ========================================================================================
void Tx_15_TFCR(uint8_t ch)
{
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,     LEN_COMM_1_4,     CMD_TFCR);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,     LEN_COMM_2_7,     workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,     LEN_COMM_3_3,     ch);
	TxStr_Int_Input("allLan",         IDX_COMM_4,     LEN_COMM_4_4,     TOTAL_LEN_TFCR15(m_ch[ch].protectRelyCnt));

	// 바디 (고정)
	TxStr_Int_Input("protectRelyCnt", IDX_TFCR15_5, LEN_TFCR15_5_2, m_ch[ch].protectRelyCnt);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < m_ch[ch].protectRelyCnt; i++)
	{
		commIdx = i * IDX_TFCR15_CYCLE;

		TxStr_Int_Input("disposBuff",   commIdx + IDX_TFCR15_6n, LEN_TFCR15_6n_5, m_ch[ch].disposBuff[i]);
		TxStr_Int_Input("protectBuff",  commIdx + IDX_TFCR15_7n, LEN_TFCR15_7n_5, m_ch[ch].protectBuff[i]);
	}

	// 테일러 (CRC)
	uint16_t crcidx = m_ch[ch].itemNum* IDX_TFCR15_CYCLE + IDX_TFCR15_6n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TFCR_15);
}

// ========================================================================================
// [21] 게이트웨이 설정정보 응답/전송 (TCN2) - 가변 구조
// ========================================================================================
void Tx_21_TCN2(uint8_t ch)
{
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,      LEN_COMM_1_4,     CMD_TCN2);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,      LEN_COMM_2_7,     workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,      LEN_COMM_3_3,     ch);
	TxStr_Int_Input("allLan",         IDX_COMM_4,      LEN_COMM_4_4,     TOTAL_LEN_TCN2_20(m_ch[ch].itemNum));


	// 바디 (고정)
	TxStr_IP_Input("IP",           IDX_TCN2_21_5,   LEN_TCN2_21_5_16,  m_ch[ch].IP);
	TxStr_IP_Input("GWip",         IDX_TCN2_21_6,   LEN_TCN2_21_6_16,  m_ch[ch].GWip);
	TxStr_Int_Input("manuCode",    IDX_TCN2_21_7,   LEN_TCN2_21_7_2,   m_ch[ch].manuCode);
	TxStr_Str_Input("GWmodel",     IDX_TCN2_21_8,   LEN_TCN2_21_8_20,  m_ch[ch].GWmodel);
	TxStr_Str_Input("fwVer",       IDX_TCN2_21_9,   LEN_TCN2_21_9_20,  m_ch[ch].fwVer);
	TxStr_Str_Input("heshCode",    IDX_TCN2_21_10,  LEN_TCN2_21_10_32, m_ch[ch].heshCode);
	TxStr_PW_Input("passWard",    IDX_TCN2_21_11,  LEN_TCN2_21_11_16_RAW, m_ch[ch].passWard);
	TxStr_Int_Input("noTxTime",    IDX_TCN2_21_12,  LEN_TCN2_21_12_4,  m_ch[ch].noTxTime);
	TxStr_Int_Input("transferMode",  IDX_TCN2_21_13,  LEN_TCN2_21_13_1,  m_ch[ch].transferMode);
	TxStr_Int_Input("disposDelTime",    IDX_TCN2_21_14,  LEN_TCN2_21_14_3,  m_ch[ch].disposDelTime);
	TxStr_Int_Input("protectDelTime",  IDX_TCN2_21_15,  LEN_TCN2_21_15_3,  m_ch[ch].protectDelTime);
	TxStr_Int_Input("measureQty",  IDX_TCN2_21_16,  LEN_TCN2_21_16_2,  m_ch[ch].itemNum);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < m_ch[ch].itemNum; i++)
	{
		commIdx = i * IDX_TCN2_21_CYCLE;

		TxStr_Faci_Input("facCode",   commIdx + IDX_TCN2_21_17n, LEN_TCN2_21_17_5n, m_ch[ch].item[i].facCode);
		TxStr_Item_Code_Input("itemCode", commIdx + IDX_TCN2_21_18n, LEN_TCN2_21_18_1n, m_ch[ch].item[i].itemCode);
		TxStr_float_Input("valueMin",   commIdx + IDX_TCN2_21_19n, LEN_TCN2_21_19_6n, m_ch[ch].item[i].rangeMin);
		TxStr_float_Input("valueMax",   commIdx + IDX_TCN2_21_20n, LEN_TCN2_21_20_6n, m_ch[ch].item[i].rangeMax);
		TxStr_float_Input("valueSdrd",  commIdx + IDX_TCN2_21_21n, LEN_TCN2_21_21_6n, m_ch[ch].item[i].rangeStandard);
	}
	// 테일러 (CRC)
	uint16_t crcidx = m_ch[ch].itemNum* IDX_TCN2_21_CYCLE + IDX_TCN2_21_17n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
	printf("> END \r\n");
	TX_Memo(ID_TCN2_20);
}


BYTE hash_result[SHA256_DIGEST_VALUELEN]; // 32바이트 버퍼
void test_sha256(void)
{
    const BYTE *test_data = (const BYTE *)"abc";
    UINT data_len = strlen((const char *)test_data);

    // KISA SHA-256 통합 함수 호출
    SHA256_Encrpyt(test_data, data_len, hash_result);

    // [검증] hash_result에 저장된 32바이트 값이 아래의 HEX 값과 정확히 일치해야 합니다.
    // BA 78 16 BF 8F 01 CF EA 41 41 40 DE 5D AE 22 23
    // B0 03 61 A3 96 17 7A 9C B4 10 FF 61 F2 00 15 AD
}

void Test_Config()
{
//    test_sha256();
}


uint8_t strtol_n(const uint8_t *str, uint32_t * data, uint16_t idx, int n,  uint32_t min, uint32_t max, uint8_t viewAdd)
{
    char buf[64] = {0,};

    char *endptr;
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;

    memcpy(buf, (const char*)(str+idx), n);
    buf[n] = '\0';

    uint32_t tempData = (uint32_t)strtoul(buf, &endptr, 10);

    uint8_t bool1 = (min <= tempData && tempData <= max);
    uint8_t bool2 = (endptr != buf);
    uint8_t bool3 = (*endptr == '\0' || *endptr == ' '); // 혹시 모르니깐 공백도

    if(bool1 && bool2 && bool3)
    {
       *data = tempData;
       printf("<%hhu>\r\n", viewAdd);//debuge
       return 0;
    }
    else
    {
        printf("error \r\n");//debuge
        TX_NAK();
        return 1;
    }

}

uint8_t strtof_n(const uint8_t *str, float* data, uint16_t idx, int n,  float min, float max, uint8_t viewAdd)
{
    char buf[64] ={0,};

    char *endptr;
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;

    memcpy(buf, (const char*)(str+idx), n);
    buf[n] = '\0';

    float tempData = strtof(buf, &endptr);

    uint8_t bool1 = (min <= tempData && tempData <= max);
    uint8_t bool2 = (endptr != buf);
    uint8_t bool3 = (*endptr == '\0' || *endptr == ' '); // 혹시 모르니깐 공백도

    if(bool1 && bool2 && bool3)
    {
       *data = tempData;
       printf("<%hhu>\r\n", viewAdd);//debuge
       return 0;
    }
    else
    {
        printf("error \r\n");//debuge
        TX_NAK();
        return 1;
    }

}

void strstr_n(const uint8_t *str, char *strDst ,uint16_t idx, int n)
{


    if (str == NULL || strDst == NULL)
    {
        printf("error \r\n");//debuge
        TX_NAK();
        return;
    }


    memcpy(strDst, (char*)str+idx, n);
    strDst[n] = '\0';

}
//


uint8_t Check_Faci_Code(const uint8_t *str,  uint32_t* data, uint16_t idx, uint8_t viewAdd)
{
    char strTemp[10] ={0,};

    if (str == NULL)
    {
        return 1;
    }

    memcpy(strTemp, (char*)str+idx, 5);
    strTemp[5] = '\0';

    char code = strTemp[0];
    uint32_t codeNum;
    switch (code)
    {
        case 'E':
            codeNum = 10000;
        break;

        case 'P':
            codeNum = 20000;
        break;

        case 'F':
            codeNum = 30000;
        break;

        default:
            printf("error \r\n");//debuge
            TX_NAK();
            return 1;
        break;
    }

    char *endptr;
    uint32_t tempData = (uint32_t)strtoul(strTemp+1, &endptr, 10);
    uint8_t bool1 = (tempData <= 9999);
    uint8_t bool2 = (endptr != strTemp+1);
    uint8_t bool3 = (*endptr == '\0' || *endptr == ' '); // 혹시 모르니깐 공백도

    if(bool1 && bool2 && bool3)
    {
       *data = tempData + codeNum;
       printf("<%hhu>\r\n", viewAdd);//debuge
       return 0;
    }
    else
    {
        printf("error \r\n");//debuge
        TX_NAK();
        return 1;
    }
}


uint8_t Check_Item_Code(const uint8_t *str, uint32_t* data, uint16_t idx, uint8_t viewAdd)
{
    if (str == NULL || data == NULL) return 1;  // NULL 체크 추가 권장

    char item = str[idx];
    uint32_t val;

    switch (item)
    {
        case 'A': val = 1; break;
        case 'D': val = 2; break;
        case 'T': val = 3; break;
        case 'H': val = 4; break;
        case 'a': val = 5; break;
        case 'b': val = 6; break;
        default:
            printf("error \r\n");
            TX_NAK();
            return 1;
    }

    *data = val;
    printf("<%hhu>\r\n", viewAdd);
    return 0;
}


uint8_t Check_Tx_Mode_Code(const char *str, uint32_t *data, uint16_t idx, uint8_t viewAdd)
{
    if (str == NULL || data == NULL)
    {
        printf("error\r\n");
        TX_NAK();
        return 1;
    }

    const char *p = str + idx;   // strncmp는 '\0' 만나면 멈춤 → memcpy 같은 무조건 3바이트 읽기 없음

    if (strncmp(p, TXMODE_HAF, 3) == 0)
        *data = 0;
    else if (strncmp(p, TXMODE_ALL, 3) == 0)
        *data = 1;
    else if (strncmp(p, TXMODE_FIV, 3) == 0)
        *data = 2;
    else
    {
        printf("error\r\n");
        TX_NAK();
        return 1;
    }

    printf("<%u>\r\n", (unsigned)viewAdd);
    return 0;
}

uint8_t Check_IP_Code(const uint8_t *str,  uint8_t* IPbuff, uint16_t idx, uint8_t viewAdd)
{
    char strTemp[20] = {0,};
    char *endptr;

    if (str == NULL)
    {
        printf("error\r\n");
        TX_NAK();
        return 1;
    }

    memcpy(strTemp, (char*)str+idx, LEN_PRSI17_5_15);
    strTemp[15] = '\0';

    uint8_t tempbuff[4] = {0,};
    uint16_t bigBuff[2] = {0,};

    tempbuff[0] = (uint8_t)strtoul(strTemp, &endptr, 10);
    tempbuff[1] = (uint8_t)strtoul(strTemp+4, &endptr, 10);

    bigBuff[0] = (uint16_t)strtoul(strTemp+8, &endptr, 10);
    tempbuff[2] = (uint8_t)bigBuff[0];
    bigBuff[1] = (uint16_t)strtoul(strTemp+12, &endptr, 10);
    tempbuff[3] = (uint8_t)bigBuff[1];


    uint8_t bool1 = (tempbuff[0] == 192);
    uint8_t bool2 = (tempbuff[1] == 168);
    uint8_t bool3 = (bigBuff[0] <= 255);
    uint8_t bool4 = (bigBuff[1] <= 255);
    uint8_t bool5 = (endptr != strTemp+12);
    uint8_t bool6 = (*endptr == '\0' || *endptr == ' '); // 혹시 모르니깐 공백도

    if(bool1 && bool2 && bool3 && bool4 && bool5 && bool6)
    {
       memcpy(IPbuff, tempbuff, 4);
       printf("<%hhu>\r\n", viewAdd);//debuge
       return 0;
    }
    else
    {
        printf("error \r\n");//debuge
        TX_NAK();
        return 1;
    }

}


uint32_t MM_End_MMDD(uint32_t YYMMDD)
{
    uint32_t YY = (YYMMDD/10000)*10000;
    uint32_t MMDD = YYMMDD/10000;
    switch (MMDD)
    {
        case 131: return (YY+201); break;
        case 228: return (YY+301); break;
        case 331: return (YY+401); break;
        case 430: return (YY+501); break;
        case 531: return (YY+601); break;
        case 630: return (YY+701); break;
        case 731: return (YY+801); break;
        case 831: return (YY+901); break;
        case 930: return (YY+1001); break;
        case 1031: return(YY+1101); break;
        case 1130: return(YY+1201); break;
        case 1231: return(YY+10000+101); break;
        default: return 0; break;
    }
}




uint32_t MM_End_Day(uint8_t YY, uint8_t MM)
{

    switch (MM)
    {
		case 1: return 31 ; break;
		case 2:
			if(YY%4==0)return 29;
			else return 28;
		break;
		case 3: return 31; break;
		case 4: return 30; break;
		case 5: return 31; break;
		case 6: return 30; break;
		case 7: return 31; break;
		case 8: return 31; break;
		case 9: return 30; break;
		case 10: return 31; break;
		case 11: return 30; break;
		case 12: return 31; break;

    }
    return 30;
}

void YYMMDDhhmm_Cal()
{
	static uint32_t timeStamp;

	static uint32_t msCnt;
	if(HAL_GetTick()-timeStamp >= 1000)
	{
		timeStamp = HAL_GetTick();
		m_time.sec++;
		if(m_time.sec==60)
		{
			m_time.sec = 0;
			m_time.min++;
			if(m_time.min==60)
			{
				m_time.minChange = 1;
				m_time.min = 0;
				m_time.hour++;
				if(m_time.hour==24)
				{
					m_time.hour = 0;
					m_time.DD++;
					if(m_time.DD>MM_End_Day(m_time.YY, m_time.MM))
					{
						m_time.DD = 1;
						m_time.MM++;
						if(m_time.MM > 12)
						{
							m_time.MM = 1;
							m_time.YY++;
						}
					}
				}
			}

		}
	}
}
void Day_Cal_TOFH_2(uint32_t startDay, uint32_t endDay)
{
    //startDay, endDay : YYMMDDmmhh
    uint16_t timeCnt = 0;
    uint32_t sYY = DAY_YY(startDay);//
    uint32_t eYY = DAY_YY(endDay);//
    uint32_t sMM = DAY_MM(startDay);//
    uint32_t eMM = DAY_MM(endDay);//
    uint32_t sDD = DAY_DD(startDay);//
    uint32_t eDD = DAY_DD(endDay);//
    uint32_t sDay = startDay/10000;
    uint32_t eDay = endDay/10000;
    uint32_t Daybuff[4];



}

void TDAH_1_Config(uint8_t ch)
{
	if(m_time.minChange &&(m_time.min%5 == 0))
	{

		switch (m_ch[ch].transferMode)
		{
			case TXMODE_HAF_NUM:
				if(m_time.min == 0 ||m_time.min == 30)
				{
					m_Gcmd.txCmd = ID_TDAH_1;
					m_time.minChange = 0;
					m_ch[ch].itemMode = HAF_IDX;
				}
			break;

			case TXMODE_FIV_NUM:
				if(m_time.min%5 == 0)
				{
					m_Gcmd.txCmd = ID_TDAH_1;
					m_time.minChange = 0;
					m_ch[ch].itemMode = FIV_IDX;
				}
			break;

			case TXMODE_ALL_NUM:
				if(m_time.min%5 == 0)
				{
					m_Gcmd.txCmd = ID_TDAH_1;
					m_time.minChange = 0;
					m_ch[ch].itemMode = HAF_IDX;
					if(m_time.min == 0 ||m_time.min == 30)
					{
						m_ch[ch].itemAllFlag = 1;
					}
				}

			break;
		}
	}

}
void TOFH_2_Config(uint8_t ch)
{
    //startDay, endDay : YYMMDDmmhh


    uint32_t startDay = 2606051100; //읽어오기
    uint32_t endDay = 2606071300; // 읽어오기

    uint32_t sDay = startDay/10000;
    uint32_t eDay = endDay/10000;
    uint32_t sTime = startDay%10000;
    uint32_t eTime = endDay%10000;
	uint32_t s5Time, e5Time;
	uint32_t s30Time, e30Time;

    uint16_t TimeSE[4][2]= {{0,2355},{0,2355},{0,2355},{0,2355}};

    uint32_t temp;
    uint32_t yymmdd = sDay;
	static uint32_t timeStamp;


	s5Time = sTime;
	e5Time = eTime;
	s30Time = sTime;
	e30Time = eTime;


	memcpy(m_ch[ch].tohfTimeSE, TimeSE, sizeof(TimeSE));

	if(s5Time%5 != 0)
	{
		if((s5Time%100)>55)
		{
			s5Time /= 100;
			s5Time *= 100;
			s5Time += 100;
			if(s5Time == 2400) s5Time = 0;
		}
		else
		{
			while(s5Time%5 != 0)
			{
				s5Time++;
			}
		}

	}
	if(e5Time%5 != 0)
	{
		while(e5Time%5 != 0)
		{
			if(e5Time)e5Time--;
		}
	}

	if((s30Time%100) != 0 && (s30Time%100) != 30)
	{
		if((s30Time%100)<30)
		{
			s30Time /= 100;
			s30Time *= 100;
			s30Time += 30;
		}
		else
		{
			s30Time /= 100;
			s30Time *= 100;
			s30Time += 100;
			if(s30Time == 2400) s30Time = 0;
		}
	}

	if((e30Time%100) != 0 && (e30Time%100) != 30)
	{
		if((e30Time%100)<30)
		{
			e30Time /= 100;
			e30Time *= 100;
		}
		else
		{
			e30Time /= 100;
			e30Time *= 100;
			e30Time += 30;
		}

	}



	m_ch[ch].tohfTimeSE[0][0] = sTime;
	for(int i =0 ;i < 4;i++)
	{
		m_ch[ch].tohfDayBuff[i] = yymmdd;
		if(yymmdd == eDay)
		{
			m_ch[ch].tohfTimeSE[i][1] = eTime;
			m_ch[ch].tohTotalDay = i;
			break;
		}
		temp = MM_End_MMDD(yymmdd);
		if (temp) yymmdd = temp;
		else yymmdd++;

	}

	m_Gcmd.txCmd = ID_TOFH_2;

	switch (m_ch[ch].transferMode)
	{
		case TXMODE_HAF_NUM:
			m_ch[ch].itemMode = HAF_IDX;
		break;

		case TXMODE_FIV_NUM:
			m_ch[ch].itemMode = FIV_IDX;
		break;

		case TXMODE_ALL_NUM:
			m_ch[ch].itemMode = HAF_IDX;
			m_ch[ch].itemAllFlag = 1;
		break;
	}


}

void TDDH_3_Config(uint8_t ch)
{
	if(m_time.hour == 0 && m_time.min == 0)
	{
		m_Gcmd.txCmd = ID_TDDH_3;
		m_time.minChange = 0;

		switch (m_ch[ch].transferMode)
		{
			case TXMODE_HAF_NUM:
				m_ch[ch].itemMode = HAF_IDX;
			break;

			case TXMODE_FIV_NUM:
				m_ch[ch].itemMode = FIV_IDX;
			break;

			case TXMODE_ALL_NUM:
				m_ch[ch].itemMode = HAF_IDX;
				m_ch[ch].itemAllFlag = 1;
			break;
		}

	}

}

void TFDH_4_Config(uint8_t ch)
{

}




void Rx_Passing_5_PDUH()
{

    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;
    printf("RX_5_PDUH \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PDUH5)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PDUH5, TOTAL_LEN_PDUH5, VIEW_ADD_3))return;

        if(Check_Tx_Mode_Code((char*)m_Gcmd.passingBuff, &tempData, IDX_COMM_5, VIEW_ADD_4))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PDUH5_6, LEN_PDUH5_6_10, MIN_5_PDUH_6, MAX_5_PDUH_6, VIEW_ADD_3))return;
        if(strtol_n(m_Gcmd.passingBuff, chkBuff+1, IDX_PDUH5_7, LEN_PDUH5_7_10, MIN_5_PDUH_7, MAX_5_PDUH_7, VIEW_ADD_3))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PDUH5_CRC))return;

        printf("> OK\r\n");

        m_ch[ch].startDay= chkBuff[0];
        printf("startDay %u \r\n",chkBuff[0]);
        m_ch[ch].endDay = chkBuff[1];
        printf("endDay %u \r\n",chkBuff[1]);

        TX_ACK(ID_PDUH_5);
    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }


}

// [7] PFST - 미전송자료 전송시간 변경 요청
void Rx_Passing_7_PFST()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[1] = {0,};
    uint32_t ch = 0;

    printf("RX_7_PFST \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PFST7)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PFST7, TOTAL_LEN_PFST7, VIEW_ADD_3))return;
        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PFST7_5, LEN_PFST7_5_4, MIN_7_PFST_5, MAX_7_PFST_5, VIEW_ADD_4))return; // 전송시간
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PFST7_CRC))return;
        printf("> OK\r\n");

        m_ch[ch].noTxTime = chkBuff[0];
        printf("noTxTime %u \r\n",chkBuff[0]);
        TX_ACK(ID_PFST_7);


    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}
// [8] PSEP - 비밀번호 변경 요청
void Rx_Passing_8_PSEP()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[1] = {0,};
    uint8_t tempBuff[16] = {0,};
    uint8_t passWard[10] = {0,};
    uint32_t ch = 0;

    printf("RX_8_PSEP \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PSEP8)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PSEP8, TOTAL_LEN_PSEP8, VIEW_ADD_3))return;

        memcpy(tempBuff, m_Gcmd.passingBuff+IDX_PSEP8_5, 16);
        Greenlink_Decrypt(tempBuff, passWard, LEN_PSEP8_5_16_RAW ,LEN_PSEP8_5_10);

        if(strtol_n(passWard, chkBuff, 0, LEN_PSEP8_5_10, MIN_8_PSEP_5, MAX_8_PSEP_5, VIEW_ADD_4))return; // 암호화 패스워드 //u32에 맞게


        if(Check_crc16(m_Gcmd.passingBuff, IDX_PSEP8_CRC))return;
        printf("> OK\r\n");

        m_ch[ch].passWard = chkBuff[0];
        printf("passWard %u \r\n",chkBuff[0]);
        TX_ACK(ID_PSEP_8);

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [9] PTIM - 서버시간 응답
void Rx_Passing_9_PTIM()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;

    printf("RX_9_PTIM \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PTIM9)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PTIM9, TOTAL_LEN_PTIM9, VIEW_ADD_3))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PTIM9_5_1, LEN_PTIM9_5_1_6, MIN_9_PTIM_5_1, MAX_9_PTIM_5_1, VIEW_ADD_4))return; // 서버YYMMDD (내가쪼갬)
        if(strtol_n(m_Gcmd.passingBuff, chkBuff+1, IDX_PTIM9_5_2, LEN_PTIM9_5_2_6, MIN_9_PTIM_5_2, MAX_9_PTIM_5_2, VIEW_ADD_5))return; // 서버hhmmss (내가쪼갬)
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PTIM9_CRC))return;
        printf("> OK\r\n");

        printf("sevrDay %u \r\n",chkBuff[0]);
        printf("sevrTime %u \r\n",chkBuff[1]);
        uint32_t gwDay = DAY_YYMMDD(m_time.YY,m_time.MM,m_time.DD);
        uint32_t gwTime =DAY_hhmmss(m_time.hour,m_time.min,m_time.sec);
		if(chkBuff[0] != gwDay)
		{
			m_time.YY = DAY_YY(chkBuff[0]);
			m_time.MM = DAY_MM(chkBuff[0]);
			m_time.DD = DAY_DD(chkBuff[0]);
		}
		if(abs(chkBuff[1] - gwTime) >= 5)
		{
			m_time.hour = DAY_hh(chkBuff[1]);
			m_time.min = DAY_mm(chkBuff[1]);
			m_time.sec = DAY_ss(chkBuff[1]);
		}
        TX_ACK(ID_PTIM_9);
    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [10] PUPG - 게이트웨이 업그레이드 요청
void Rx_Passing_10_PUPG()
{
    uint32_t tempData = 0;
    uint8_t tempIpBuff[4] = {0,};
    uint8_t inBuff[144] = {0,};
    uint8_t outBuff[131] = {0,};
    uint32_t chkBuff[5] = {0,};
    uint32_t ch = 0;
	char strData10R[10] = {0,};
	char strData40R[40] = {0,};
	char strData50R[50] = {0,};
	char strData10R_2[10] = {0,};

    printf("RX_10_PUPG \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PUPG10)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PUPG10, TOTAL_LEN_PUPG10, VIEW_ADD_3))return;

        memcpy(inBuff, m_Gcmd.passingBuff+IDX_PUPG10_5_RAW, LEN_PUPG10_5_144_RAW);
        Greenlink_Decrypt(inBuff, outBuff, LEN_PUPG10_5_144_RAW ,LEN_PUPG10_5_131);


        if(strtol_n(outBuff, chkBuff, IDX_PUPG10_5, LEN_PUPG10_5_1, MIN_10_PUPG_5, MAX_10_PUPG_5, VIEW_ADD_4))return;   // FTP 타입
        strstr_n(outBuff, strData40R, IDX_PUPG10_6, LEN_PUPG10_6_40);  // FTP IP/Domain
        if(strtol_n(outBuff, chkBuff+1, IDX_PUPG10_7, LEN_PUPG10_7_5, MIN_10_PUPG_7, MAX_10_PUPG_7, VIEW_ADD_6))return;   // FTP Port
        strstr_n(outBuff, strData50R, IDX_PUPG10_8, LEN_PUPG10_8_50);  // 파일 경로
        strstr_n(outBuff, strData10R, IDX_PUPG10_9, LEN_PUPG10_9_10);  // FTP ID
        strstr_n(outBuff, strData10R_2, IDX_PUPG10_10, LEN_PUPG10_10_10);// FTP PWD
        if(Check_IP_Code(outBuff, tempIpBuff, IDX_PUPG10_11, VIEW_ADD_10))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PUPG10_CRC))return;
        printf("> OK\r\n");

        m_ch[ch].FTPtype = chkBuff[0];
        printf("FTPtype %u \r\n",chkBuff[0]);
        m_ch[ch].FTPport = chkBuff[1];
        printf("FTPport %u \r\n",chkBuff[1]);

        memcpy(m_ch[ch].FTPipDomain, strData40R, LEN_PUPG10_6_40);
        printf("FTPipDomain %s \r\n",strData40R);
        memcpy(m_ch[ch].road, strData50R, LEN_PUPG10_8_50);
        printf("road %s \r\n",strData50R);
        memcpy(m_ch[ch].FTPid, strData10R, LEN_PUPG10_9_10);
        printf("FTPid %s \r\n",strData10R);
        memcpy(m_ch[ch].FTPpwd, strData10R_2, LEN_PUPG10_10_10);
        printf("FTPpwd %s \r\n",strData10R_2);
        memcpy(m_ch[ch].IP, tempIpBuff, 4);
        printf("%hhu.%hhu.%hhu.%hhu \r\n",tempIpBuff[0], tempIpBuff[1], tempIpBuff[2], tempIpBuff[3]);
        TX_ACK(ID_PUPG_10);

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [11] PVER - 버전정보 요청
void Rx_Passing_11_PVER()
{
    uint32_t tempData = 0;
    uint32_t ch = 0;

    printf("RX_11_PVER \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PVER11)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PVER11, TOTAL_LEN_PVER11, VIEW_ADD_3))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PVER11_CRC))return;

        printf("> OK\r\n");

        m_Gcmd.txCmd = ID_TVER_11;
    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [12] PSET - GW 시간 변경 요청
void Rx_Passing_12_PSET()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;

    printf("RX_12_PSET \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PSET12)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PSET12, TOTAL_LEN_PSET12, VIEW_ADD_3))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PSET12_5_1, LEN_PSET12_5_1_6, MIN_12_PSET_5_1, MAX_12_PSET_5_1, VIEW_ADD_4))return; // 서버시간 변경값 YYMMDD (내가쪼갬)
        if(strtol_n(m_Gcmd.passingBuff, chkBuff+1, IDX_PSET12_5_2, LEN_PSET12_5_2_6, MIN_12_PSET_5_2, MAX_12_PSET_5_2, VIEW_ADD_5))return; // 서버시간 변경값 hhmmss (내가쪼갬)
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PSET12_CRC))return;
        printf("> OK\r\n");

        printf("sevrDay %u \r\n",chkBuff[0]);
        printf("sevrTime %u \r\n",chkBuff[1]);


		m_time.YY = DAY_YY(chkBuff[0]);
		m_time.MM = DAY_MM(chkBuff[0]);
		m_time.DD = DAY_DD(chkBuff[0]);
		m_time.hour = DAY_hh(chkBuff[1]);
		m_time.min = DAY_mm(chkBuff[1]);
		m_time.sec = DAY_ss(chkBuff[1]);

        TX_ACK(ID_PSET_12);

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [13] PFCC - 시설코드 변경 요청
void Rx_Passing_13_PFCC()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;
    printf("RX_13_PFCC \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PFCC13)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PFCC13, TOTAL_LEN_PFCC13, VIEW_ADD_3))return;

        if(Check_Faci_Code(m_Gcmd.passingBuff, chkBuff, IDX_PFCC13_5, VIEW_ADD_4))return; // 이전 시설코드
        if(Check_Faci_Code(m_Gcmd.passingBuff, chkBuff+1, IDX_PFCC13_6, VIEW_ADD_5))return; // 변경 시설코드
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PFCC13_CRC))return;
        printf("> OK\r\n");

        for(int i =0 ;i < 10;i++)
        {
            if(m_ch[ch].disposBuff[i] == chkBuff[0])
            {
                 m_ch[ch].disposBuff[i] = chkBuff[1];
                 printf("disposBuff %u \r\n",chkBuff[1]);
                 break;
            }
            else if(m_ch[ch].protectBuff[i] == chkBuff[0])
            {
                 m_ch[ch].protectBuff[i] = chkBuff[1];
                 printf("protectBuff %u \r\n",chkBuff[1]);
                 break;
            }
        }
        TX_ACK(ID_PFCC_13);

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [14] PAST - 측정범위 변경 요청 (N개 가변항목 구조)
void Rx_Passing_14_PAST()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[20][2] = {0,};
    float chkBuff_F[20][3] = {0,};
    uint32_t ch = 0;

    printf("RX_14_PAST \r\n");
    // 가변 패킷이므로 최소 고정부 크기(20바이트) 이상 수신되었는지 1차 체크
    if(m_Gcmd.passingCnt >= IDX_PAST14_6n)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, IDX_PAST14_6n, 1000, VIEW_ADD_3))return;

        uint32_t itemCount = 0;
        if(strtol_n(m_Gcmd.passingBuff, &itemCount,  IDX_PAST14_5, LEN_PAST14_5_2, MIN_14_PAST_5, MAX_14_PAST_5, VIEW_ADD_4))return; // 변경 항목 수(N)

        // 가변 루프 전진용 인덱스 설정
        uint16_t variableIdx = IDX_PAST14_6n;

        for(int i = 0; i < itemCount; i++)
        {
            // 루프마다 각각 배열(i)이나 구조체 멤버에 순서대로 매칭하여 파싱

            if(Check_Faci_Code(m_Gcmd.passingBuff, &chkBuff[i][0], variableIdx, VIEW_ADD_5))return;// 시설코드
            if(Check_Item_Code(m_Gcmd.passingBuff, &chkBuff[i][1], variableIdx + 5, VIEW_ADD_6))return;

            // ★ 규격서 확인: 범위 한계 설정값(최소/최대/기준)이 실수 형태이면 strtof_n 사용!
            if(strtof_n(m_Gcmd.passingBuff, &chkBuff_F[i][0], variableIdx + 6, LEN_PAST14_8n_6, MIN_14_PAST_8, MAX_14_PAST_8, VIEW_ADD_7))return; // 최소값
            if(strtof_n(m_Gcmd.passingBuff, &chkBuff_F[i][1], variableIdx + 12, LEN_PAST14_9n_6, MIN_14_PAST_9, MAX_14_PAST_9, VIEW_ADD_8))return;// 최대값
            if(strtof_n(m_Gcmd.passingBuff, &chkBuff_F[i][2], variableIdx + 18, LEN_PAST14_10n_6, MIN_14_PAST_10, MAX_14_PAST_10, VIEW_ADD_9))return;// 기준값

            // 정의된 보폭 수치(24바이트)만큼 다음 항목으로 점프
            variableIdx += IDX_PAST14_CYCLE;
        }

        // 루프 탈출 후 최종 위치의 2바이트 강제 수신 테스트 매칭
        if(Check_crc16(m_Gcmd.passingBuff, variableIdx))return;
        printf("> OK\r\n");


        for(int i = 0; i < itemCount; i++)
        {
                m_ch[ch].item[i].facCode = chkBuff[i][0];
                printf("facCode %u \r\n",chkBuff[i][0]);
                m_ch[ch].item[i].itemCode = chkBuff[i][1];
                printf("itemCode %u \r\n",chkBuff[i][1]);
                m_ch[ch].item[i].rangeMin = chkBuff_F[i][0];
                printf("measureMin %f \r\n",chkBuff_F[i][0]);
                m_ch[ch].item[i].rangeMax = chkBuff_F[i][1];
                printf("measureMax %f \r\n",chkBuff_F[i][1]);
                m_ch[ch].item[i].rangeStandard = chkBuff_F[i][2];
                printf("measureStandard %f \r\n",chkBuff_F[i][2]);
        }
        TX_ACK(ID_PAST_14);
    }
}
// [15] PFCR - 관계정보 조회 요청
void Rx_Passing_15_PFCR()
{
    uint32_t tempData = 0;
    uint32_t ch = 0;

    printf("RX_15_PFCR \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PFCR15)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PFCR15, TOTAL_LEN_PFCR15, VIEW_ADD_3))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PFCR15_CRC))return;
        printf("> OK\r\n");

        m_Gcmd.txCmd = ID_TFCR_15;
    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

//[16] PFRS - 방지시설 정상여부 관계정보 변경 요청 (N개 가변관계 구조)
void Rx_Passing_16_PFRS()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[20][2] = {0,};
    uint32_t ch = 0;

    printf("RX_16_PFRS \r\n");
    // 최소 고정부 크기(20바이트) 이상 수신 체크
    if(m_Gcmd.passingCnt >= IDX_PFRS16_6n)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, IDX_PFRS16_6n, 1000, VIEW_ADD_3))return;

        uint32_t relationCount = 0;
        if(strtol_n(m_Gcmd.passingBuff, &relationCount, IDX_PFRS16_5, LEN_PFRS16_5_2, MIN_16_PFRS_5, MAX_16_PFRS_5, VIEW_ADD_4))return; // 관계 정보 수(N)

        uint16_t variableIdx = IDX_PFRS16_6n;

        for(int i = 0; i < relationCount; i++)
        {
            if(Check_Faci_Code(m_Gcmd.passingBuff, &chkBuff[i][0], variableIdx, VIEW_ADD_5))return; // 배출시설코드
            if(Check_Faci_Code(m_Gcmd.passingBuff, &chkBuff[i][1], variableIdx + 5, VIEW_ADD_6))return; // 방지시설코드
            // 정의된 보폭 수치(10바이트)만큼 다음 항목으로 점프
            variableIdx += IDX_PFRS16_CYCLE;
        }

        // 루프 탈출 후 최종 위치의 2바이트 강제 수신 테스트 매칭
        if(Check_crc16(m_Gcmd.passingBuff, variableIdx))return;
        printf("> OK\r\n");

        for(int i = 0; i < relationCount; i++)
        {
            m_ch[ch].disposBuff[i] = chkBuff[i][0];
            printf("disposBuff %u \r\n",chkBuff[i][0]);
            m_ch[ch].protectBuff[i] = chkBuff[i][1];
            printf("protectBuff %u \r\n",chkBuff[i][1]);
        }
        TX_ACK(ID_PFRS_16);
    }
}
// [17] PRSI - 통신서버IP 변경 요청
void Rx_Passing_17_PRSI()
{

    uint32_t tempData = 0;
    uint8_t tempIpBuff[4] = {0,};
    uint8_t tempBuff[16] = {0,};
    uint8_t rawIpBuff[15] = {0,};
    uint32_t ch = 0;

    printf("RX_17_PRSI \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PRSI17)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PRSI17, TOTAL_LEN_PRSI17, VIEW_ADD_3))return;

        memcpy(tempBuff, m_Gcmd.passingBuff+IDX_PRSI17_5, 16);
        Greenlink_Decrypt(tempBuff, rawIpBuff, LEN_PRSI17_5_16_RAW ,LEN_PRSI17_5_15);

        if(Check_IP_Code(rawIpBuff, tempIpBuff, 0, VIEW_ADD_10))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PRSI17_CRC))return;
         printf("> OK\r\n");


         memcpy(m_ch[ch].IP, tempIpBuff, 4);
         for(int i =0 ;i < 4;i++)
         {
            printf("%hhu.\r\n",tempIpBuff[i]);
         }
         TX_ACK(ID_PRSI_17);

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [18] PDAT - 자료전송모드 변경 요청
void Rx_Passing_18_PDAT()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[1] = {0,};
    uint32_t ch = 0;

    printf("RX_18_PDAT \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PDAT18)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PDAT18, TOTAL_LEN_PDAT18, VIEW_ADD_3))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PDAT18_5, LEN_PDAT18_5_1, MIN_18_PDAT_5, MAX_18_PDAT_5, VIEW_ADD_4))return; // 전송모드

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PDAT18_CRC))return;
        printf("> OK\r\n");

        m_ch[ch].transferMode = chkBuff[0];
        printf("transferMode %u \r\n",chkBuff[0]);
        TX_ACK(ID_PDAT_18);

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [19] PODT - 유예시간 설정 변경 요청
void Rx_Passing_19_PODT()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;

    printf("RX_19_PODT \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PODT19)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PODT19, TOTAL_LEN_PODT19, VIEW_ADD_3))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PODT19_5, LEN_PODT19_5_3, MIN_19_PODT_5, MAX_19_PODT_5, VIEW_ADD_4))return; // 배출 가동유예
        if(strtol_n(m_Gcmd.passingBuff, chkBuff+1, IDX_PODT19_6, LEN_PODT19_6_3, MIN_19_PODT_6, MAX_19_PODT_6, VIEW_ADD_5))return; // 방지 정지유예

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PODT19_CRC))return;
        printf("> OK\r\n");

        m_ch[ch].disposDelTime = chkBuff[0];
        printf("disposDelTime %u \r\n",chkBuff[0]);
        m_ch[ch].protectDelTime = chkBuff[1];
        printf("protectDelTime %u \r\n",chkBuff[1]);
        TX_ACK(ID_PODT_19);

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}
void Rx_Passing_20_PCN2()
{
    uint32_t tempData = 0;
    uint32_t ch = 0;

    printf("RX_20_PCN2 \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PCN2_20)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PCN2_20, TOTAL_LEN_PCN2_20, VIEW_ADD_3))return;
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PCN220_CRC))return;
        printf("> OK\r\n");
		m_Gcmd.txCmd = ID_TCN2_20;

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

// [22] PRBT - GW 재기동 요청
void Rx_Passing_22_PRBT()
{
    uint32_t tempData = 0;
    uint32_t ch = 0;

    printf("RX_22_PRBT \r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PRBT22)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PRBT22, TOTAL_LEN_PRBT22, VIEW_ADD_3))return;
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PRBT22_CRC))return;
        printf("> OK\r\n");
        TX_ACK(ID_PRBT_22);

    }
    else
    {
        printf("cntErr %hhu\r\n", m_Gcmd.passingCnt);
    }
}

void CallBack_TimeOut_Check()
{
    if(m_Gcmd.txMsgTimeStamp && HAL_GetTick() - m_Gcmd.txMsgTimeStamp > 30000)
    {
        printf("TimeOut \r\n");
        switch (m_Gcmd.ID)
        {
            case ID_TDAH_1 : printf("TDAH_1 ");	break;
            case ID_TOFH_2 : printf("TOFH_2 ");	break;
            case ID_TDDH_3 : printf("TDDH_3 ");	break;
            case ID_TFDH_4 : printf("TFDH_4 ");	break;
            case ID_TDUH_5 : printf("TDUH_5 ");	break;
            case ID_TNOH_6 : printf("TNOH_6 ");	break;
            case ID_TTIM_9 : printf("TTIM_9 ");	break;
            case ID_TUPG_10: printf("TUPG_10");	break;
            case ID_TVER_11: printf("TVER_11");	break;
            case ID_TFCR_15: printf("TFCR_15");	break;
            case ID_TFCR_16: printf("TFCR_16");	break;
            case ID_TCN2_20: printf("TCN2_20");	break;
            case ID_PDUH_5 : printf("PDUH_5 ");	break;
            case ID_PFST_7 : printf("PFST_7 ");	break;
            case ID_PSEP_8 : printf("PSEP_8 ");	break;
            case ID_PTIM_9 : printf("PTIM_9 ");	break;
            case ID_PUPG_10: printf("PUPG_10");	break;
            case ID_PVER_11: printf("PVER_11");	break;
            case ID_PSET_12: printf("PSET_12");	break;
            case ID_PFCC_13: printf("PFCC_13");	break;
            case ID_PAST_14: printf("PAST_14");	break;
            case ID_PFCR_15: printf("PFCR_15");	break;
            case ID_PFRS_16: printf("PFRS_16");	break;
            case ID_PRSI_17: printf("PRSI_17");	break;
            case ID_PDAT_18: printf("PDAT_18");	break;
            case ID_PODT_19: printf("PODT_19");	break;
            case ID_PCN2_20: printf("PCN2_20");	break;
            case ID_PRBT_22: printf("PRBT_22");	break;


        }
        m_Gcmd.txMsgTimeStamp = 0;
    }
}

void Rx_Get_Gateway(uint8_t rxData)
{
	m_Gcmd.rxBuff[m_Gcmd.rxCnt++] = rxData;
	m_Gcmd.rxCnt %= 50;
	m_Gcmd.rxTimeStamp = HAL_GetTick();
}

void Rx_Gateway_Config()//
{
	if(HAL_GetTick() - m_Gcmd.rxTimeStamp>30 && m_Gcmd.rxTimeStamp)
	{
		memcpy(m_Gcmd.passingBuff, m_Gcmd.rxBuff, m_Gcmd.rxCnt);
		memset(m_Gcmd.rxBuff, 0, sizeof(m_Gcmd.rxBuff));
		m_Gcmd.passingCnt = m_Gcmd.rxCnt;

            if(strncmp((char*)m_Gcmd.passingBuff, CMD_PDUH, 4) == 0){Rx_Passing_5_PDUH();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PFST, 4) == 0){Rx_Passing_7_PFST();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PSEP, 4) == 0){Rx_Passing_8_PSEP();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PTIM, 4) == 0){Rx_Passing_9_PTIM();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PUPG, 4) == 0){Rx_Passing_10_PUPG();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PVER, 4) == 0){Rx_Passing_11_PVER();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PSET, 4) == 0){Rx_Passing_12_PSET();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PFCC, 4) == 0){Rx_Passing_13_PFCC();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PAST, 4) == 0){Rx_Passing_14_PAST();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PFCR, 4) == 0){Rx_Passing_15_PFCR();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PFRS, 4) == 0){Rx_Passing_16_PFRS();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PRSI, 4) == 0){Rx_Passing_17_PRSI();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PDAT, 4) == 0){Rx_Passing_18_PDAT();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PODT, 4) == 0){Rx_Passing_19_PODT();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PCN2, 4) == 0){Rx_Passing_20_PCN2();}
        else if(strncmp((char*)m_Gcmd.passingBuff, CMD_PRBT, 4) == 0){Rx_Passing_22_PRBT();}
        else if(m_Gcmd.passingBuff[0] == MSG_ACK && m_Gcmd.passingCnt == 1)Rx_Passing_ACK();
        else if(m_Gcmd.passingBuff[0] == MSG_NAK && m_Gcmd.passingCnt == 1)Rx_Passing_NAK();
        else if(m_Gcmd.passingBuff[0] == MSG_EOT && m_Gcmd.passingCnt == 1)Rx_Passing_EOT();

        memset(m_Gcmd.passingBuff, 0, sizeof(m_Gcmd.passingBuff));
        m_Gcmd.passingCnt = 0;
		m_Gcmd.rxTimeStamp = 0;
		m_Gcmd.rxCnt = 0;
	}
}

void Tx_Gateway_Config(uint8_t ch)
{
	if(m_Gcmd.txCmd==0) return;

	switch (m_Gcmd.txCmd)
	{
		case ID_TDAH_1 :
			Tx_1_TDAH(ch);
		break;

		case ID_TOFH_2 :
			Tx_2_TOFH(ch);
		break;

		case ID_TDDH_3 :
			Tx_3_TDDH(ch);
		break;
		case ID_TFDH_4 :
			Tx_4_TFDH(ch);
		break;
		case ID_TDUH_5 :
			Tx_5_TDUH(ch);
		break;
		case ID_TNOH_6 :
			Tx_6_TNOH(ch);
		break;
		case ID_TTIM_9 :
			Tx_9_TTIM(ch);
		break;
		case ID_TUPG_10:
			Tx_10_TUPG(ch);
		break;
		case ID_TVER_11:
			Tx_11_TVER(ch);
		break;
		case ID_TFCR_15:
			Tx_15_TFCR(ch);
		break;

		case ID_TCN2_20:
			Tx_21_TCN2(ch);
		break;
	}



	m_Gcmd.txCmd = 0;
}

void Gateway_Init()
{
	m_time.lastTxTime = 2606071100;
	m_time.wakeUpTime = 2606091255;
	m_time.YY = 0;
	m_time.MM = 0;
	m_time.DD = 0;
	m_time.hour = 0;
	m_time.min = 0;
	m_time.sec = 0;

	workPlaceCode = 12345;
	TOFH_2_Config(0);

}
void Gateway_Config()
{

	YYMMDDhhmm_Cal();

	ACK_ReSend();
	TxMsg_ReSend();

	TDAH_1_Config(0);
	TDDH_3_Config(0);


	Rx_Gateway_Config();
	Tx_Gateway_Config(0);
}


void Uart_Simple_Rx_Passing(UART_T* uart, uint8_t rxData)
{
	switch (uart->rxStep)
	{
		case STEP0:
			if(rxData == '[')
			{
				uart->rxCmdAdd = 0;
				uart->rxCmdData  = 0;
				uart->rxStep = STEP1;
			}

		break;

		case STEP1:
			if('0' <= rxData && rxData <= '9')
			{
				rxData = rxData -'0';
				uart->rxCmdAdd *= 10;
				uart->rxCmdAdd += rxData;
			}
			else if(rxData == ',')
			{
				uart->rxStep = STEP2;
			}
			else
			{
				uart->rxStep = STEP0;
			}

		break;

		case STEP2:
			if('0' <= rxData && rxData <= '9')
			{
				rxData = rxData -'0';
				uart->rxCmdData *= 10;
				uart->rxCmdData += rxData;
			}
			else if(rxData == '-')//must start
			{
				uart->rxCmdData = -1;
			}
			else if(rxData == ']')
			{
				uart->rxCmdChk |= (1<< uart->rxRingCnt);
				uart->rxRingBuff[uart->rxRingCnt][IDX_RX_CMD] = uart->rxCmdAdd;
				uart->rxRingBuff[uart->rxRingCnt][IDX_RX_DATA] = uart->rxCmdData;
				uart->rxRingCnt++;
				uart->rxRingCnt %= 10;

				uart->rxStep = STEP0;
			}
			else
			{
				uart->rxStep = STEP0;
			}
		break;


	}
}
void Testfunction()
{

}


void TxTest()
{
	static uint32_t timeStamp;

	if(HAL_GetTick()-timeStamp >= 1000 )
	{
		timeStamp = HAL_GetTick();
		printf("%u \r\n",HAL_GetTick());
	}
}
void Uart_RxBuff_View(UART_T* uart, uint8_t data)
{
	uart->rxViewBuff[uart->rxViewCnt++] = data;
	uart->rxViewCnt %= RX_BUFF_SIZE;
}



static void UART1_RxRestart(void)
{
    // 1) 수신 중단/상태 초기화
    (void)HAL_UART_AbortReceive_IT(&huart1);

    // 2) 에러 플래그 정리 (ORE/FE/NE/PE 등)
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_FEFLAG(&huart1);
    __HAL_UART_CLEAR_NEFLAG(&huart1);
    __HAL_UART_CLEAR_PEFLAG(&huart1);

    // 3) IDLE 등 라인 상태도 정리(선택)
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);

    // 4) RX 인터럽트 재가동
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_ERR);

    // 5) 수신 재시작
    (void)HAL_UART_Receive_IT(&huart1, Rx_data1, 1);
}

static void UART2_RxRestart(void)
{
    // 1) 수신 중단/상태 초기화
    (void)HAL_UART_AbortReceive_IT(&huart2);

    // 2) 에러 플래그 정리 (ORE/FE/NE/PE 등)
    __HAL_UART_CLEAR_OREFLAG(&huart2);
    __HAL_UART_CLEAR_FEFLAG(&huart2);
    __HAL_UART_CLEAR_NEFLAG(&huart2);
    __HAL_UART_CLEAR_PEFLAG(&huart2);

    // 3) IDLE 등 라인 상태도 정리(선택)
    __HAL_UART_CLEAR_IDLEFLAG(&huart2);

    // 4) RX 인터럽트 재가동
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_ERR);

    // 5) 수신 재시작
    (void)HAL_UART_Receive_IT(&huart2, Rx_data2, 1);
}

static void UART3_RxRestart(void)
{
    // 1) 수신 중단/상태 초기화
    (void)HAL_UART_AbortReceive_IT(&huart3);

    // 2) 에러 플래그 정리 (ORE/FE/NE/PE 등)
    __HAL_UART_CLEAR_OREFLAG(&huart3);
    __HAL_UART_CLEAR_FEFLAG(&huart3);
    __HAL_UART_CLEAR_NEFLAG(&huart3);
    __HAL_UART_CLEAR_PEFLAG(&huart3);

    // 3) IDLE 등 라인 상태도 정리(선택)
    __HAL_UART_CLEAR_IDLEFLAG(&huart3);

    // 4) RX 인터럽트 재가동
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_ERR);

    // 5) 수신 재시작
    (void)HAL_UART_Receive_IT(&huart3, Rx_data3, 1);
}



void Debug_Print_Value(uint8_t idx, int value, int magin)
{
	static int minBuff[10] ={10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000};
	static int maxBuff[10]={-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000};
	static int chkCnt[10], chkFlag[10];

	static int minCntBuff[10], maxCntBuff[10], okCntBuff[10];
	char str[50] = {0,};
	int len;
	float okRatio;
	if(idx >= 10) return;

	if(!chkFlag[idx])
	{
		if(minBuff[idx]> value) minBuff[idx] = value;
		if(maxBuff[idx]<value) maxBuff[idx] = value;
		chkCnt[idx]++;
		if(chkCnt[idx] >=200)
		{
			chkFlag[idx] = 1;
		}
		len = snprintf(str,sizeof(str),"[make] min = %d max = %d value = %d \r\n",minBuff[idx],maxBuff[idx], value);
		if(len < 0) len = 0;
		if(len > sizeof(str)) len = sizeof(str);
		HAL_UART_Transmit(&huart2,(uint8_t*)str,len,100);
	}
	else
	{
		if(minBuff[idx] - magin > value)
		{
			minCntBuff[idx]++;
		}
		else if(maxBuff[idx] + magin < value)
		{
			maxCntBuff[idx]++;
		}
		else
		{
			okCntBuff[idx]++;
		}

		int total = okCntBuff[idx] + minCntBuff[idx] + maxCntBuff[idx];
		if(total > 0)
			okRatio = ((float)okCntBuff[idx] / (float)total) * 100.0f;
		else
			okRatio = 0.0f;

		len = snprintf(str, sizeof(str),"Lmt[%d %d] Cnt[%d %d] <%d> %.2f\r\n",minBuff[idx], maxBuff[idx], minCntBuff[idx], maxCntBuff[idx], value, okRatio);
		if(len < 0) len = 0;
		if(len > sizeof(str)) len = sizeof(str);

		HAL_UART_Transmit(&huart2,(uint8_t*)str,len,100);


	}

}



uint16_t uart1ErrCnt, uart2ErrCnt, uart3ErrCnt;
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
    	uart1ErrCnt++;
        UART1_RxRestart();
    }

    if(huart->Instance == USART2)
    {
    	uart2ErrCnt++;
    	UART2_RxRestart();
    }
    if(huart->Instance == USART3)
    {
    	uart3ErrCnt++;
    	UART3_RxRestart();
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	 if (huart == &huart2)
	 {
		HAL_UART_Receive_IT(&huart2, Rx_data2, 1);
		Uart_RxBuff_View(&m_uart2, Rx_data2[0]);
        Uart_Simple_Rx_Passing(&m_uart2, Rx_data2[0]);



	 }
	 if(huart == &huart1)
	 {
		HAL_UART_Receive_IT(&huart1, Rx_data1, 1);
		Uart_RxBuff_View(&m_uart1, Rx_data1[0]);
        Rx_Get_Gateway(Rx_data1[0]);

	 }
	 if(huart == &huart3)
	 {
		HAL_UART_Receive_IT(&huart3, Rx_data3, 1);
		Uart_RxBuff_View(&m_uart3, Rx_data3[0]);
		Uart_Simple_Rx_Passing(&m_uart3, Rx_data3[0]);

	 }
}



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)//485
{
	if(huart == &huart2)
	{
//		HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_RESET);//485
	}

}

void Uart_Gulobal()
{

}





