//cmd.c

#include "cmd.h"

UART_T m_uart1;
UART_T m_uart2;
UART_T m_uart3;


uint8_t Rx_data3[1];
uint8_t Rx_data2[1];
uint8_t Rx_data1[1];

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
	len = sprintf(str,"u1 re %d %d\r\n",cmd, data);
	HAL_UART_Transmit(&huart1,str,len,100);
}
void Uart2_Passing_Pop(int cmd, int data)
{
	uint8_t str[30] = {0,};
	uint8_t len;
	len = sprintf(str,"u2 re %d %d\r\n",cmd, data);
	HAL_UART_Transmit(&huart2,str,len,100);
}
void Uart3_Passing_Pop(int cmd, int data)
{
	uint8_t str[30] = {0,};
	uint8_t len;
	len = sprintf(str,"u3 re %d %d\r\n",cmd, data);
	HAL_UART_Transmit(&huart3,str,len,100);
}


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




uint8_t chimneyNem = 2;
CHIMNEY_T m_chimney[4];
CMD_T m_Gcmd;

uint16_t data1 = 1;
uint16_t data2 = 22;
uint16_t data3 = 333;
uint16_t data4 = 4444;
uint16_t data5 = 0;
uint16_t data6 = 0;
uint16_t data7 = 0;
uint32_t data8 = 88888888;
uint16_t data9 = 0;
uint32_t data10 = 1010101010;
uint16_t data11 = 0;
uint16_t data12 = 0;
uint16_t data13 = 0;
uint16_t data14 = 0;
uint16_t data15 = 0;
uint16_t data16 = 0;
uint16_t data20 = 0;
uint16_t data32 = 0;

char strData1[1]   = "A";
char strData2[2]   = "B2";
char strData3[3]   = "C3C";
char strData4[4]   = "D44D";
char strData5[5]   = "E555E";
char strData6[6]   = "F6666F";
char strData7[7]   = "G77777G";
char strData8[8]   = "H888888H";     // 8개
char strData9[9]   = "I9999999I";    // 9개
char strData10[10] = "aaaaaaaaaa";   // 10개
char strData11[11] = "bbbbbbbbbbb";  // 11개
char strData12[12] = "cccccccccccc"; // 12개
char strData13[13] = "ddddddddddddd"; // 13개
char strData14[14] = "eeeeeeeeeeeeee"; // 14개
char strData15[15] = "fffffffffffffff"; // 15개
char strData16[16] = "gggggggggggggggg"; // 16개
char strData20[20] = "hhhhhhhhhhhhhhhhhhhh"; // 20개
char strData32[32] = "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"; // 32개


float floatData6 = 12.34, floatMin6= 9.28, floatMax6= 119.74, floatBase6= 500.06;



char txAllBuff[200];


void TxStr_Int_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data )
{
	char str[16] = {0,};
	int len = 0, termLen = 0;

	len = snprintf(str, sizeof(str), "%d",data);
//	printf("%s : idx:%d, fixLen:%d, data:%d\r\n",debugStr, idx, fixLen, data);
	if(len > fixLen)
	{
		//error
		printf("error\r\n");
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

}
// float 데이터를 받아서 000.00 형식으로 txAllBuff에 우측 정렬(좌측 공백 채움) 삽입하는 함수
void TxStr_float_Input(char* debugStr, uint16_t idx, uint16_t fixLen, float data)
{
	char str[16] = {0,};
	int len = 0, termLen = 0;

	// 규격서 규칙: 소수점 3째자리에서 반올림하여 2자리로 고정 표현
	// 예: 25.716 -> 25.72 / 0.0 -> 0.00
	// %.2f 자체적으로 셋째 자리 반올림을 수행하지만, 미세한 부동소수점 오차 방지를 위해 roundf 후 출력합니다.
	float roundedData = roundf(data * 100.0f) / 100.0f;

	// 소수점 아래 2자리 고정 형식으로 문자열 변환
	len = snprintf(str, sizeof(str), "%.2f", roundedData);

	// 디버그 출력 (소수는 %f 또는 %.2f로 매칭)
//	printf("%s : idx:%d, fixLen:%d, data:%.2f\r\n", debugStr, idx, fixLen, roundedData);

	// 변환된 문자열이 지정된 길이(fixLen)를 초과한 경우 (에러 처리)
	if (len > fixLen)
	{
		printf("error: data too long for fixLen\r\n");
		return;
	}
	// 자릿수가 부족한 경우: 남는 자리는 좌측에 공백 배치하여 우측 정렬
	else if (len < fixLen)
	{
		termLen = fixLen - len;

		// 1. 부족한 자릿수만큼 공백(' ')을 먼저 채움
		for (int i = 0; i < termLen; i++)
		{
			txAllBuff[idx++] = ' ';
		}
		// 2. 공백 뒤에 이어서 변환된 소수 문자열을 복사 (우측 정렬 완성)
		memcpy(txAllBuff + idx, str, len);
	}
	// 자릿수가 딱 맞는 경우
	else if (len == fixLen)
	{
		memcpy(txAllBuff + idx, str, fixLen);
	}
}

void TxStr_Str_Input(char* debugStr, uint16_t idx, uint16_t fixLen, char* str )
{
    memcpy(txAllBuff+idx, str, fixLen);
}


// ========================================================================================
// [1] 측정자료 전송 (TDAH) - 가변 구조
// ========================================================================================
void Tx_TDAH_1()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,    LEN_COMM_1_4,      strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,    LEN_COMM_2_7,      strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,    LEN_COMM_3_3,      strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,    LEN_COMM_4_4,      data4);
	TxStr_Str_Input("dataType",     IDX_COMM_5,    LEN_COMM_5_3,      strData3);

	// 바디 (고정)
	TxStr_Int_Input("measureTime",  IDX_TDAH1_6,   LEN_TDAH1_6_10,    data10);
	TxStr_Int_Input("itemCount",    IDX_TDAH1_7,   LEN_TDAH1_7_2,     data2);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < chimneyNem; i++)
	{
		commIdx = i * IDX_TDAH1_CYCLE;

		TxStr_Str_Input("facilityCode", commIdx + IDX_TDAH1_8n,  LEN_TDAH1_8n_5,  strData5);
		TxStr_Str_Input("itemCode",     commIdx + IDX_TDAH1_9n,  LEN_TDAH1_9n_1,  strData1);
		TxStr_float_Input("measureValue", commIdx + IDX_TDAH1_10n, LEN_TDAH1_10n_6, floatData6);
		TxStr_Int_Input("dataStatus",   commIdx + IDX_TDAH1_11n, LEN_TDAH1_11n_1, data1);
		TxStr_Int_Input("opStatus",     commIdx + IDX_TDAH1_12n, LEN_TDAH1_12n_1, data1);
		TxStr_Int_Input("preventCheck", commIdx + IDX_TDAH1_13n, LEN_TDAH1_13n_1, data1);
	}

	// 테일러 (CRC)
	uint16_t crcidx = (chimneyNem - 1) * IDX_TDAH1_CYCLE + IDX_TDAH1_13n + LEN_TDAH1_13n_1;
	TxStr_Int_Input("crc16",        crcidx,        2,                 data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [2] 전원단절구간자료 전송 (TOFH) - 가변 구조
// ========================================================================================
void Tx_2_TOFH()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,    LEN_COMM_1_4,      strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,    LEN_COMM_2_7,      strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,    LEN_COMM_3_3,      strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,    LEN_COMM_4_4,      data4);
	TxStr_Str_Input("dataType",     IDX_COMM_5,    LEN_COMM_5_3,      strData3);

	// 바디 (고정)
	TxStr_Int_Input("baseDate",     IDX_TOFH2_6,   LEN_TOFH2_6_8,     data8);
	TxStr_Int_Input("powerOffCount",IDX_TOFH2_7,   LEN_TOFH2_7_3,     data3);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < chimneyNem; i++)
	{
		commIdx = i * IDX_TOFH2_CYCLE;

		TxStr_Int_Input("powerOffTime", commIdx + IDX_TOFH2_8n, LEN_TOFH2_8n_4, data4);
	}

	// 테일러 (CRC)
	uint16_t crcidx = (chimneyNem - 1) * IDX_TOFH2_CYCLE + IDX_TOFH2_8n + LEN_TOFH2_8n_4;
	TxStr_Int_Input("crc16",        crcidx,        2,                 data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [3] 일일 마감자료 전송 (TDDH) - 가변 구조
// ========================================================================================
void Tx_3_TDDH()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,    LEN_COMM_1_4,      strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,    LEN_COMM_2_7,      strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,    LEN_COMM_3_3,      strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,    LEN_COMM_4_4,      data4);
	TxStr_Str_Input("dataType",     IDX_COMM_5,    LEN_COMM_5_3,      strData3);

	// 바디 (고정)
	TxStr_Int_Input("closeDate",    IDX_TDDH3_6,   LEN_TDDH3_6_8,     data8);
	TxStr_Int_Input("dayDataCount", IDX_TDDH3_7,   LEN_TDDH3_7_3,     data3);
	TxStr_Int_Input("tdahCount",    IDX_TDDH3_8,   LEN_TDDH3_8_3,     data3);
	TxStr_Int_Input("tofhCount",    IDX_TDDH3_9,   LEN_TDDH3_9_3,     data3);
	TxStr_Int_Input("itemCount",    IDX_TDDH3_10,  LEN_TDDH3_10_2,    data2);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < chimneyNem; i++)
	{
		commIdx = i * IDX_TDDH3_CYCLE;

		TxStr_Str_Input("facilityCode", commIdx + IDX_TDDH3_11n, LEN_TDDH3_11n_5, strData5);
		TxStr_Str_Input("itemCode",     commIdx + IDX_TDDH3_12n, LEN_TDDH3_12n_1, strData1);
		TxStr_Int_Input("normalCount",  commIdx + IDX_TDDH3_13n, LEN_TDDH3_13n_3, data3);
		TxStr_Int_Input("abnormalCount",commIdx + IDX_TDDH3_14n, LEN_TDDH3_14n_3, data3);
		TxStr_Int_Input("commFailCount",commIdx + IDX_TDDH3_15n, LEN_TDDH3_15n_3, data3);
		TxStr_Int_Input("pwrOffCount",  commIdx + IDX_TDDH3_16n, LEN_TDDH3_16n_3, data3);
		TxStr_Int_Input("checkCount",   commIdx + IDX_TDDH3_17n, LEN_TDDH3_17n_3, data3);
	}

	// 테일러 (CRC)
	uint16_t crcidx = (chimneyNem - 1) * IDX_TDDH3_CYCLE + IDX_TDDH3_17n + LEN_TDDH3_17n_3;
	TxStr_Int_Input("crc16",        crcidx,        2,                 data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [4] 미전송자료 전송 (TFDH) - 가변 구조
// ========================================================================================
void Tx_4_TFDH()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,    LEN_COMM_1_4,      strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,    LEN_COMM_2_7,      strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,    LEN_COMM_3_3,      strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,    LEN_COMM_4_4,      data4);
	TxStr_Str_Input("dataType",     IDX_COMM_5,    LEN_COMM_5_3,      strData3);

	// 바디 (고정)
	TxStr_Int_Input("measureTime",  IDX_TFDH4_6,   LEN_TFDH4_6_10,    data10);
	TxStr_Int_Input("itemCount",    IDX_TFDH4_7,   LEN_TFDH4_7_2,     data2);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < chimneyNem; i++)
	{
		commIdx = i * IDX_TFDH4_CYCLE;

		TxStr_Str_Input("facilityCode", commIdx + IDX_TFDH4_8n,  LEN_TFDH4_8n_5,  strData5);
		TxStr_Str_Input("itemCode",     commIdx + IDX_TFDH4_9n,  LEN_TFDH4_9n_1,  strData1);
		TxStr_float_Input("measureValue", commIdx + IDX_TFDH4_10n, LEN_TFDH4_10n_6, floatData6);
		TxStr_Int_Input("dataStatus",   commIdx + IDX_TFDH4_11n, LEN_TFDH4_11n_1, data1);
		TxStr_Int_Input("opStatus",     commIdx + IDX_TFDH4_12n, LEN_TFDH4_12n_1, data1);
		TxStr_Int_Input("exhaustCheck", commIdx + IDX_TFDH4_13n, LEN_TFDH4_13n_1, data1);
	}

	// 테일러 (CRC)
	uint16_t crcidx = (chimneyNem - 1) * IDX_TFDH4_CYCLE + IDX_TFDH4_13n + LEN_TFDH4_13n_1;
	TxStr_Int_Input("crc16",        crcidx,        2,                 data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [5] 저장자료 응답 (TDUH) - 가변 구조
// ========================================================================================
void Tx_5_TDUH()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,    LEN_COMM_1_4,      strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,    LEN_COMM_2_7,      strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,    LEN_COMM_3_3,      strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,    LEN_COMM_4_4,      data4);
	TxStr_Str_Input("dataType",     IDX_COMM_5,    LEN_COMM_5_3,      strData3);

	// 바디 (고정)
	TxStr_Int_Input("measureTime",  IDX_TDUH5_6,   LEN_TDUH5_6_10,    data10);
	TxStr_Int_Input("itemCount",    IDX_TDUH5_7,   LEN_TDUH5_7_2,     data2);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < chimneyNem; i++)
	{
		commIdx = i * IDX_TDUH5_CYCLE;

		TxStr_Str_Input("facilityCode", commIdx + IDX_TDUH5_8n,  LEN_TDUH5_8n_5,  strData5);
		TxStr_Str_Input("itemCode",     commIdx + IDX_TDUH5_9n,  LEN_TDUH5_9n_1,  strData1);
		TxStr_float_Input("measureValue", commIdx + IDX_TDUH5_10n, LEN_TDUH5_10n_6, floatData6);
		TxStr_Int_Input("dataStatus",   commIdx + IDX_TDUH5_11n, LEN_TDUH5_11n_1, data1);
		TxStr_Int_Input("opStatus",     commIdx + IDX_TDUH5_12n, LEN_TDUH5_12n_1, data1);
		TxStr_Int_Input("exhaustCheck", commIdx + IDX_TDUH5_13n, LEN_TDUH5_13n_1, data1);
	}

	// 테일러 (CRC)
	uint16_t crcidx = (chimneyNem - 1) * IDX_TDUH5_CYCLE + IDX_TDUH5_13n + LEN_TDUH5_13n_1;
	TxStr_Int_Input("crc16",        crcidx,        2,                 data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [6] 5분자료 전송대상 정보 (TNOH) - 가변 구조
// ========================================================================================
void Tx_6_TNOH()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,    LEN_COMM_1_4,      strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,    LEN_COMM_2_7,      strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,    LEN_COMM_3_3,      strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,    LEN_COMM_4_4,      data4);
	TxStr_Str_Input("dataType",     IDX_COMM_5,    LEN_COMM_5_3,      strData3);

	// 바디 (고정)
	TxStr_Int_Input("dataTime",     IDX_TNOH6_6,   LEN_TNOH6_6_10,    data10);
	TxStr_Int_Input("itemCount",    IDX_TNOH6_7,   LEN_TNOH6_7_2,     data2);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < chimneyNem; i++)
	{
		commIdx = i * IDX_TNOH6_CYCLE;

		TxStr_Str_Input("facilityCode", commIdx + IDX_TNOH6_8n,  LEN_TNOH6_8n_5,  strData5);
		TxStr_Str_Input("itemCode",     commIdx + IDX_TNOH6_9n,  LEN_TNOH6_9n_1,  strData1);
		TxStr_Int_Input("opStatus",     commIdx + IDX_TNOH6_10n, LEN_TNOH6_10n_1, data1);
		TxStr_Int_Input("preventCheck", commIdx + IDX_TNOH6_11n, LEN_TNOH6_11n_1, data1);
	}

	// 테일러 (CRC)
	uint16_t crcidx = (chimneyNem - 1) * IDX_TNOH6_CYCLE + IDX_TNOH6_11n + LEN_TNOH6_11n_1;
	TxStr_Int_Input("crc16",        crcidx,        2,                 data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [9] 서버시간 조회 요청 (TTIM) - 바디 없는 고정 길이 구조
// ========================================================================================
void Tx_9_TTIM()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,    LEN_COMM_1_4,      strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,    LEN_COMM_2_7,      strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,    LEN_COMM_3_3,      strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,    LEN_COMM_4_4,      data4);

	TxStr_Int_Input("crc16",        IDX_TTIM9_CRC, 2,                 data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [10] 게이트웨이 업그레이드 결과 전송 (TUPG) - 고정 길이 구조
// ========================================================================================
void Tx_10_TUPG()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,     LEN_COMM_1_4,     strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,     LEN_COMM_2_7,     strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,     LEN_COMM_3_3,     strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,     LEN_COMM_4_4,     data4);

	// 바디
	TxStr_Str_Input("serverIp",     IDX_TUPG10_5,   LEN_TUPG10_5_16,  strData16);
	TxStr_Str_Input("gwIp",         IDX_TUPG10_6,   LEN_TUPG10_6_16,  strData16);
	TxStr_Str_Input("makerCode",    IDX_TUPG10_7,   LEN_TUPG10_7_2,   strData2);
	TxStr_Str_Input("gwModel",      IDX_TUPG10_8,   LEN_TUPG10_8_20,  strData20);
	TxStr_Str_Input("fwVersion",    IDX_TUPG10_9,   LEN_TUPG10_9_20,  strData20);
	TxStr_Str_Input("hashCode",     IDX_TUPG10_10,  LEN_TUPG10_10_32, strData32);
	TxStr_Int_Input("crc16",        IDX_TUPG10_CRC, 2,                data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [11] 버전정보 응답 전송 (TVER) - 고정 길이 구조
// ========================================================================================
void Tx_11_TVER()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,     LEN_COMM_1_4,     strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,     LEN_COMM_2_7,     strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,     LEN_COMM_3_3,     strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,     LEN_COMM_4_4,     data4);

	// 바디
	TxStr_Str_Input("serverIp",     IDX_TVER11_5,   LEN_TVER11_5_16,  strData16);
	TxStr_Str_Input("gwIp",         IDX_TVER11_6,   LEN_TVER11_6_16,  strData16);
	TxStr_Str_Input("makerCode",    IDX_TVER11_7,   LEN_TVER11_7_2,   strData2);
	TxStr_Str_Input("gwModel",      IDX_TVER11_8,   LEN_TVER11_8_20,  strData20);
	TxStr_Str_Input("fwVersion",    IDX_TVER11_9,   LEN_TVER11_9_20,  strData20);
	TxStr_Str_Input("hashCode",     IDX_TVER11_10,  LEN_TVER11_10_32, strData32);
	TxStr_Int_Input("crc16",        IDX_TVER11_CRC, 2,                data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [15] 방지시설 정상여부 관계정보 조회 응답 (TFCR) - 가변 구조
// ========================================================================================
void Tx_15_TFCR()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,     LEN_COMM_1_4,     strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,     LEN_COMM_2_7,     strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,     LEN_COMM_3_3,     strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,     LEN_COMM_4_4,     data4);

	// 바디 (고정)
	TxStr_Int_Input("relationCount",IDX_TFCR15_5,   LEN_TFCR15_5_2,   data2);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < chimneyNem; i++)
	{
		commIdx = i * IDX_TFCR15_CYCLE;

		TxStr_Str_Input("exhaustCode",  commIdx + IDX_TFCR15_6n, LEN_TFCR15_6n_5, strData5);
		TxStr_Str_Input("preventCode",  commIdx + IDX_TFCR15_7n, LEN_TFCR15_7n_5, strData5);
	}

	// 테일러 (CRC)
	uint16_t crcidx = (chimneyNem - 1) * IDX_TFCR15_CYCLE + IDX_TFCR15_7n + LEN_TFCR15_7n_5;
	TxStr_Int_Input("crc16",        crcidx,         2,                data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}

// ========================================================================================
// [21] 게이트웨이 설정정보 응답/전송 (TCN2) - 가변 구조
// ========================================================================================
void Tx_21_TCN2()
{
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,      LEN_COMM_1_4,     strData4);
	TxStr_Str_Input("factoryCode",  IDX_COMM_2,      LEN_COMM_2_7,     strData7);
	TxStr_Str_Input("chimneyCode",  IDX_COMM_3,      LEN_COMM_3_3,     strData3);
	TxStr_Int_Input("totalLen",     IDX_COMM_4,      LEN_COMM_4_4,     data4);


	// 바디 (고정)
	TxStr_Str_Input("serverIp",     IDX_TCN2_21_5,   LEN_TCN2_21_5_16,  strData16);
	TxStr_Str_Input("gwIp",         IDX_TCN2_21_6,   LEN_TCN2_21_6_16,  strData16);
	TxStr_Str_Input("makerCode",    IDX_TCN2_21_7,   LEN_TCN2_21_7_2,   strData2);
	TxStr_Str_Input("gwModel",      IDX_TCN2_21_8,   LEN_TCN2_21_8_20,  strData20);
	TxStr_Str_Input("fwVersion",    IDX_TCN2_21_9,   LEN_TCN2_21_9_20,  strData20);
	TxStr_Str_Input("hashCode",     IDX_TCN2_21_10,  LEN_TCN2_21_10_32, strData32);
	TxStr_Str_Input("password",     IDX_TCN2_21_11,  LEN_TCN2_21_11_16, strData16);
	TxStr_Int_Input("unsendTime",   IDX_TCN2_21_12,  LEN_TCN2_21_12_4,  data4);
	TxStr_Str_Input("transferMode", IDX_TCN2_21_13,  LEN_TCN2_21_13_1,  strData1);
	TxStr_Int_Input("exhaustDelay", IDX_TCN2_21_14,  LEN_TCN2_21_14_3,  data3);
	TxStr_Int_Input("preventDelay", IDX_TCN2_21_15,  LEN_TCN2_21_15_3,  data3);
	TxStr_Int_Input("itemCount",    IDX_TCN2_21_16,  LEN_TCN2_21_16_2,  data2);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < chimneyNem; i++)
	{
		commIdx = i * IDX_TCN2_21_CYCLE;

		TxStr_Str_Input("facilityCode", commIdx + IDX_TCN2_21_17n, LEN_TCN2_21_17_5n, strData5);
		TxStr_Str_Input("itemCode",     commIdx + IDX_TCN2_21_18n, LEN_TCN2_21_18_1n, strData1);
		TxStr_float_Input("rangeMin",   commIdx + IDX_TCN2_21_19n, LEN_TCN2_21_19_6n, floatMin6);
		TxStr_float_Input("rangeMax",   commIdx + IDX_TCN2_21_20n, LEN_TCN2_21_20_6n, floatMax6);
		TxStr_float_Input("rangeBase",  commIdx + IDX_TCN2_21_21n, LEN_TCN2_21_21_6n, floatBase6);
	}

	// 테일러 (CRC)
	uint16_t crcidx = (chimneyNem - 1) * IDX_TCN2_21_CYCLE + IDX_TCN2_21_21n + LEN_TCN2_21_21_6n;
	TxStr_Int_Input("crc16",        crcidx,          2,                 data2);

	HAL_UART_Transmit(&huart2, txAllBuff, strlen(txAllBuff), 100);
	printf("\r\n");
}


void Testfunction()
{
    static uint32_t timeStamp;

//    Tx_TVER_11();
    if(HAL_GetTick()-timeStamp >= 1000)
    {
        timeStamp = HAL_GetTick();
    }
}
void Tx_Config()
{
	TxStr_Int_Input("data1", 0, 3, data1 );
	TxStr_Int_Input("data2", 3, 3, data2 );
	TxStr_Int_Input("data3", 6, 3, data3 );
	TxStr_Int_Input("data4", 9, 3, data4 );

	HAL_UART_Transmit(&huart2,txAllBuff,strlen(txAllBuff),100);
	printf("\r\n");
}

void strtol_n(const char *str, long* data, uint16_t idx, int n)
{
    char buf[64];
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;

    strncpy(buf, str+idx, n);
    buf[n] = '\0';

    *data =  strtol(buf, NULL, 10);
}
void strtof_n(const char *str, float* data, uint16_t idx, int n)
{
    char buf[64];
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;

    strncpy(buf, str+idx, n);
    buf[n] = '\0';


    *data = strtof(buf, NULL);
}

void strstr_n(const char *str, char *strDst ,uint16_t idx, int n)
{

    if (str == NULL || strDst == NULL)
    {
        return;
    }


    strncpy(strDst, str+idx, n);
    strDst[n] = '\0';

}
//
void Passing_Rx_Gateway()//
{
	if(HAL_GetTick() - m_Gcmd.timeStamp>10 && m_Gcmd.timeStamp)
	{
		memcpy(m_Gcmd.passingBuff, m_Gcmd.rxBuff, m_Gcmd.rxCnt);
		memset(m_Gcmd.rxBuff, 0, sizeof(m_Gcmd.rxBuff));
		m_Gcmd.passingCnt = m_Gcmd.rxCnt;
		if(m_Gcmd.passingCnt == 12)
		{
//			strtol_n(m_Gcmd.passingBuff, 0, 4);
//			strtol_n(m_Gcmd.passingBuff, 4, 5);
//			strtol_n(m_Gcmd.passingBuff, 9, 2);
//			strtol_n(m_Gcmd.passingBuff, 11, 1);
		}


		m_Gcmd.timeStamp = 0;
		m_Gcmd.rxCnt = 0;
	}
}


void Rx_Passing_PVER_11()
{
    if(m_Gcmd.passingCnt == TOTAL_LEN_PVER11_REQ)
    {
 //       strtol_n(m_Gcmd.passingBuff, 0, 4);
 //       strtol_n(m_Gcmd.passingBuff, 4, 7);
  //      strtol_n(m_Gcmd.passingBuff, 11, 3);
  //      strtol_n(m_Gcmd.passingBuff, 14, 4);
  //      strtol_n(m_Gcmd.passingBuff, 18, 2);
    }

}

void Rx_Gateway_Config(uint8_t rxData)
{
	m_Gcmd.rxBuff[m_Gcmd.rxCnt++] = rxData;
	m_Gcmd.rxCnt %= 50;
	m_Gcmd.timeStamp = HAL_GetTick();
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


void TxTest()
{
	static uint32_t timeStamp;

	if(HAL_GetTick()-timeStamp >= 500 )
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
	uint8_t str[50] = {0,};
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
		HAL_UART_Transmit(&huart2,str,len,100);
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

		HAL_UART_Transmit(&huart2,str,len,100);


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
	 	Rx_Gateway_Config(Rx_data2[0]);



	 }
	 if(huart == &huart1)
	 {
		HAL_UART_Receive_IT(&huart1, Rx_data1, 1);
		Uart_RxBuff_View(&m_uart1, Rx_data1[0]);
	 	Uart_Simple_Rx_Passing(&m_uart1, Rx_data1[0]);

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





