//cmd.c

#include "cmd.h"
extern uint8_t readSDbuff[1200];

UART_T m_uart1;
UART_T m_uart2;
UART_T m_uart3;

uint8_t facIdxQ = 0;

uint8_t Rx_data3[1];
uint8_t Rx_data2[1];
uint8_t Rx_data1[1];
uint16_t uart1ErrCnt, uart2ErrCnt, uart3ErrCnt;

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



void UartRx2DataProcess()
{
	int cmd;
	int data;
	if(m_uart2.rxCmdChk == 0x0000)return;
	for(int i =0 ;i < 10;i++)
	{
		if(m_uart2.rxRingBuff[i][IDX_RX_CMD] !=0)
		{
			cmd = m_uart2.rxRingBuff[i][IDX_RX_CMD];
			data = m_uart2.rxRingBuff[i][IDX_RX_DATA];
			User_Setting_Passing_Pop(cmd, data);
			Uart_Clear_Rx(&m_uart2, i);
		}
	}
}

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
	 	Rx_Get_Gateway(Rx_data2[0]);//test



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




CHIMNEY_T m_ch;
CMD_T m_Gcmd;
TIME_T m_time;

uint8_t txAllBuff[1200];
uint16_t eep3Day5MinTable[288] =
{
    /* Day 1 : index 0 ~ 287 */
       0,    5,   10,   15,   20,   25,   30,   35,   40,   45,   50,   55,
     100,  105,  110,  115,  120,  125,  130,  135,  140,  145,  150,  155,
     200,  205,  210,  215,  220,  225,  230,  235,  240,  245,  250,  255,
     300,  305,  310,  315,  320,  325,  330,  335,  340,  345,  350,  355,
     400,  405,  410,  415,  420,  425,  430,  435,  440,  445,  450,  455,
     500,  505,  510,  515,  520,  525,  530,  535,  540,  545,  550,  555,
     600,  605,  610,  615,  620,  625,  630,  635,  640,  645,  650,  655,
     700,  705,  710,  715,  720,  725,  730,  735,  740,  745,  750,  755,
     800,  805,  810,  815,  820,  825,  830,  835,  840,  845,  850,  855,
     900,  905,  910,  915,  920,  925,  930,  935,  940,  945,  950,  955,
    1000, 1005, 1010, 1015, 1020, 1025, 1030, 1035, 1040, 1045, 1050, 1055,
    1100, 1105, 1110, 1115, 1120, 1125, 1130, 1135, 1140, 1145, 1150, 1155,
    1200, 1205, 1210, 1215, 1220, 1225, 1230, 1235, 1240, 1245, 1250, 1255,
    1300, 1305, 1310, 1315, 1320, 1325, 1330, 1335, 1340, 1345, 1350, 1355,
    1400, 1405, 1410, 1415, 1420, 1425, 1430, 1435, 1440, 1445, 1450, 1455,
    1500, 1505, 1510, 1515, 1520, 1525, 1530, 1535, 1540, 1545, 1550, 1555,
    1600, 1605, 1610, 1615, 1620, 1625, 1630, 1635, 1640, 1645, 1650, 1655,
    1700, 1705, 1710, 1715, 1720, 1725, 1730, 1735, 1740, 1745, 1750, 1755,
    1800, 1805, 1810, 1815, 1820, 1825, 1830, 1835, 1840, 1845, 1850, 1855,
    1900, 1905, 1910, 1915, 1920, 1925, 1930, 1935, 1940, 1945, 1950, 1955,
    2000, 2005, 2010, 2015, 2020, 2025, 2030, 2035, 2040, 2045, 2050, 2055,
    2100, 2105, 2110, 2115, 2120, 2125, 2130, 2135, 2140, 2145, 2150, 2155,
    2200, 2205, 2210, 2215, 2220, 2225, 2230, 2235, 2240, 2245, 2250, 2255,
    2300, 2305, 2310, 2315, 2320, 2325, 2330, 2335, 2340, 2345, 2350, 2355
};
uint16_t eep3Day30MinTable[48] =
{
    /* Day 1 : index 0 ~ 47 */
       0,   30,  100,  130,  200,  230,  300,  330,  400,  430,  500,  530,
     600,  630,  700,  730,  800,  830,  900,  930, 1000, 1030, 1100, 1130,
    1200, 1230, 1300, 1330, 1400, 1430, 1500, 1530, 1600, 1630, 1700, 1730,
    1800, 1830, 1900, 1930, 2000, 2030, 2100, 2130, 2200, 2230, 2300, 2330
};
uint32_t flashBuff[FLASH_SIZE_WORDS];
uint8_t flashSave = 0;



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

	m_time.YY = 26;
	m_time.MM = 8;
	m_time.DD = 15;

	m_time.hour = 13;
	m_time.min = 45;
	m_time.sec = 0;

}
void Led_Toggle()
{
	static uint32_t timeStamp;

	if(HAL_GetTick()-timeStamp >= 500 )
	{
		timeStamp = HAL_GetTick();
		HAL_GPIO_TogglePin(GPIOA, LED1_Pin);
		HAL_GPIO_TogglePin(GPIOA, LED2_Pin);
		HAL_GPIO_TogglePin(GPIOA, LED3_Pin);
	}
}

void Goto_TxCmd(uint8_t cmd)
{
	m_Gcmd.txCmd = cmd;
	m_Gcmd.txUse = 1;
	Debug_printf("Goto_TxCmd [%hhu]\r\n", cmd);
}

void Goto_TxCmd_Mode(uint8_t cmd, uint8_t mode)
{
	m_Gcmd.txCmd = cmd;
	m_Gcmd.txUse = 1;
	m_ch.itemMode = mode;
	Debug_printf("Goto_TxCmd_Mode [%hhu] [%hhu]\r\n", cmd, mode);
}




void Flash_Write_Word(uint16_t add, uint32_t data)
{
	if(add >=FLASH_IDX_MAX_OVER) return;
	flashBuff[add] = data;
	flashSave = 1;
}

void Flash_Write_All_Word()
{
	if(!flashSave)return;

	flashSave = 0;
    // 플래시 Unlock
    HAL_FLASH_Unlock();

    // Erase (필요시 1페이지, 또는 충분한 페이지 수 지정)
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
    EraseInitStruct.NbPages = 1; // 200바이트가 1페이지에 충분하다면
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

    // Write
    uint32_t address = FLASH_USER_START_ADDR;
    for (int i = 0; i < FLASH_SIZE_WORDS; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, flashBuff[i]);
        address += 4;
    }

    // 플래시 Lock
    HAL_FLASH_Lock();
}


void Flash_Read_All_Word()
{
    uint32_t flash_data[FLASH_SIZE_WORDS];

    uint32_t address = FLASH_USER_START_ADDR;
    for (int i = 0; i < FLASH_SIZE_WORDS; i++) {
        flash_data[i] = *(uint32_t*)address;
        address += 4;
    }

	memcpy(flashBuff, flash_data, sizeof(flashBuff));

}

void Flash_init()
{
#if 0
	facIdxQ = 0;
	for(int i =0 ;i < FLASH_SIZE_WORDS;i++)
	{
		flashBuff[i] = 0;
	}
	flashSave = 1;
	Flash_Write_All_Word();
	return;
#endif
	facIdxQ = 0;

	Flash_Read_All_Word();

	for(int i =0 ;i < m_ch.itemNum;i++)
	{
		m_ch.item[i].facCode = flashBuff[FLASH_GET_IDX_FAC(i)];
		m_ch.item[i].itemCode = flashBuff[FLASH_GET_IDX_ITEM(i)];
		m_ch.item[i].couple = flashBuff[FLASH_GET_IDX_COUPLE(i)];

		m_ch.item[i].rangeMin = (float)flashBuff[FLASH_GET_IDX_MIN(i)];
		m_ch.item[i].rangeMax = (float)flashBuff[FLASH_GET_IDX_MAX(i)];
		m_ch.item[i].rangeStandard = (float)flashBuff[FLASH_GET_IDX_STAND(i)];
	}
	m_ch.IP[0] = flashBuff[FLASH_IDX_IP_OLD_0];
	m_ch.IP[1] = flashBuff[FLASH_IDX_IP_OLD_1];
	m_ch.IP[2] = flashBuff[FLASH_IDX_IP_OLD_2];
	m_ch.IP[3] = flashBuff[FLASH_IDX_IP_OLD_3];


	m_ch.noTxTime = flashBuff[FLASH_IDX_NO_TXTIME];
	m_ch.passWard = flashBuff[FLASH_IDX_PASSWARD];
	m_ch.disposDelTime = flashBuff[FLASH_IDX_DISPOS_DELTIME];
	m_ch.protectDelTime = flashBuff[FLASH_IDX_PROTECT_DELTEIM];
	m_ch.transferMode = flashBuff[FLASH_IDX_TRANSFER_MODE];

	if(flashBuff[FLASH_IDX_REBOOT])
	{
		Flash_Write_Word(FLASH_IDX_REBOOT, 0);
		Goto_TxCmd(ID_TCN2_20);
	}
	if(flashBuff[FLASH_IDX_PW_OFF]) m_ch.cmd2pwOffFlag = 1;
	else m_ch.cmd2pwOffFlag = 0;



}

void Debug_printf(const char *format, ...)
{
    char buffer[128]; // 변환된 문자열을 담을 임시 버퍼 (크기는 시스템에 맞게 조절)
    va_list args;     // 가변 인자 목록을 저장할 변수

    // 1. 가변 인자 처리 시작 (format 이후의 인자들을 args에 묶음)
    va_start(args, format);

    // 2. 가변 인자들을 사용해 형식에 맞게 문자열을 만들어 buffer에 저장
    // vsnprintf는 버퍼 크기(sizeof)를 넘지 않게 막아주어 안전해!
    vsnprintf(buffer, sizeof(buffer), format, args);

    // 3. 가변 인자 처리 종료
    va_end(args);

    // 4. 완성된 텍스트(buffer)를 실제 UART로 전송 (이 부분을 DMA나 인터럽트 함수로 연결)
    // 예: HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer, strlen(buffer));
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
}

void Debug_Buff_Len(uint8_t* buff , uint8_t len)
{
	HAL_UART_Transmit(&huart2, buff, len, 100);
}
void TxAllBuff_Clear()
{
    memset(txAllBuff, 0, sizeof(txAllBuff));
    m_Gcmd.txCnt = 0;
}
void TxAll_CmdBuff_Clear()
{
	memset(m_Gcmd.txCmdBuff, 0, sizeof(m_Gcmd.txCmdBuff));
	m_Gcmd.txTotalCnt = 0;
}

void TxAllBuff_Set(uint8_t* msg, uint16_t len)
{
	memcpy(txAllBuff, msg, len);
    m_Gcmd.txCnt = len;
}
void TxAllBuff_ReSend()
{
	HAL_UART_Transmit(&huart1, txAllBuff, m_Gcmd.txCnt, 100);
}
void Tx_Cmd_Instruction(uint8_t* buff, uint16_t cnt)
{
	HAL_UART_Transmit(&huart1, (uint8_t*)buff, cnt, 100);
	Debug_printf("[Ras Inst] ");

	if(cnt>=25)cnt -= 2; //전문 crc 없애기 위해서 대략 25넘으면 전문이라고 생각
	HAL_UART_Transmit(&huart2, (uint8_t*)buff, cnt, 100);
	Debug_printf("\r\n");
}



void Tx_Head_DbugMsg(uint8_t txEn, uint8_t itemMode)
{

	if(txEn) Debug_printf(">>[TX] ");
	else Debug_printf(">>[SD] ");
	if(itemMode == FIV_IDX) Debug_printf("[FIV] ");
	else Debug_printf("[HAF] ");

}





void Debug_Ack_Eot()
{
	if(m_Gcmd.txMsgFlag)
	{
		if(HAL_GetTick() - m_Gcmd.txMsgTimeStamp>100)
		{
			Rx_Passing_ACK();
			Debug_printf("}\r\n");
			if(m_Gcmd.txCmdEnd)
			{
				m_Gcmd.txCmdEnd = 0;
				Debug_printf("}\r\n");
			}
		}
	}

	if(m_Gcmd.txAckFlag)
	{
		if(HAL_GetTick() - m_Gcmd.txAckTimeStamp>100 )
		{
			Rx_Passing_EOT();
			Debug_printf("}\r\n");
			if(m_Gcmd.txCmdEnd)
			{
				m_Gcmd.txCmdEnd = 0;
				Debug_printf("}\r\n");
			}
		}
	}
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
	return 0;

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
        Debug_printf("[ERR]CRC\r\n");
        return 1;
    }
}



#define timePoint
uint8_t Tx_DayTimeSave_org(uint32_t YYMMDD, uint16_t hhmm)
{
	if(m_Gcmd.txCmd == ID_TFDH_4)
	{
		m_ch.cmd4TxDay = YYMMDD;
		m_ch.cmd4TxTime = hhmm;
	}
	else if(m_Gcmd.txCmd == ID_TDUH_5)
	{
		m_ch.cmd5TxDay = YYMMDD;
		m_ch.cmd5TxTime = hhmm;

		if(YYMMDD > m_ch.cmd5endDay)
		{
			return SD_CMD5_OVER_NEXT;
		}
		else if(YYMMDD == m_ch.cmd5endDay)
		{
			if(hhmm > m_ch.cmd5endTime) return SD_CMD5_OVER_NEXT;
		}
	}
	return SD_OK;
}
uint8_t Tx_DayTimeSave(uint32_t YYMMDD, uint16_t hhmm)
{
	if(m_Gcmd.txCmd == ID_TFDH_4)
	{
		m_ch.cmd4TxDay = YYMMDD;
		m_ch.cmd4TxTime = hhmm;
	}
	else if(m_Gcmd.txCmd == ID_TDUH_5)
	{
		m_ch.cmd5TxDay = YYMMDD;
		m_ch.cmd5TxTime = hhmm;
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
	static uint32_t timeCnt;
	timeCnt++;

	if(timeCnt >= 100)
	{
		timeCnt = 0;
		m_time.sec++;
		if(m_time.sec%5==0)m_time.secChange1 = 1;
		if(m_time.sec==60)
		{
			m_time.sec = 0;
			m_time.min++;
			m_time.minChange4 = 1;
			if(m_time.min%5==0) m_time.minChange1 = 1;
			if(m_time.min==60)
			{
				m_time.min = 0;
				m_time.hour++;
				if(m_time.hour==24)
				{
					m_time.hour = 0;
					m_time.DD++;
					m_time.minChange3 = 1;
					m_time.minChange2 = 1;
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


uint32_t YYMMDD_Add(uint32_t YYMMDD)
{
	YYMMDD++;
	uint32_t YY = DAY_YY(YYMMDD);
	uint32_t MM = DAY_MM(YYMMDD);
	uint32_t DD = DAY_DD(YYMMDD);
	uint32_t yymmdd;
	if (DD > MM_End_Day(YY, MM))
	{
		DD = 1;
		MM++;
		if(MM>12)
		{
			MM = 1;
			YY++;
		}
		yymmdd = DAY_YYMMDD(YY, MM, DD);

	}
	else
	{
		yymmdd = YYMMDD;

	}
	return yymmdd;
}

uint32_t YYMMDD_Sub(uint32_t YYMMDD)
{
	YYMMDD--;
	uint32_t YY = DAY_YY(YYMMDD);
	uint32_t MM = DAY_MM(YYMMDD);
	uint32_t DD = DAY_DD(YYMMDD);
	uint32_t yymmdd;
	if (DD == 0)
	{
		if(MM==1)
		{
			YY--;
			MM = 12;
			DD = 31;
		}
		else
		{
			MM--;
			DD = MM_End_Day(YY, MM);
		}

		yymmdd = DAY_YYMMDD(YY, MM, DD);

	}
	else
	{
		yymmdd = YYMMDD;

	}
	return yymmdd;
}

uint32_t Get_YYMMDD()
{
	uint32_t yymmdd = DAY_YYMMDD(m_time.YY,m_time.MM,m_time.DD);
	return yymmdd;
}

uint16_t Get_hhmm()
{
	uint16_t hhmm = m_time.hour*100 + m_time.min;
	return hhmm;
}

uint32_t Get_hhmmss()
{
	uint32_t hhmmss = m_time.hour*10000 + m_time.min*100 +  m_time.sec;
	return hhmmss;
}


uint32_t Get_YYMMDDhhmm()
{
	uint32_t YYMMDDhhmm = Get_YYMMDD()*10000 + Get_hhmm();
	return YYMMDDhhmm;
}


uint32_t Get_Pre_YYMMDDhhmm()
{
    uint32_t YYMMDD = Get_YYMMDD();
    uint32_t hhmm   = Get_hhmm();
    const uint16_t *tbl = (m_ch.itemMode == HAF_IDX) ? eep3Day30MinTable : eep3Day5MinTable;
    uint16_t cnt = (m_ch.itemMode == HAF_IDX) ? DAY_1_30CNT : DAY_1_5CNT;

    for (int i = 0; i < cnt; i++)
    {
        if (tbl[i] == hhmm)
        {
            if (i == 0) { YYMMDD = YYMMDD_Sub(YYMMDD); hhmm = tbl[cnt-1]; }
            else        { hhmm = tbl[i-1]; }
            break;
        }
    }
    return YYMMDD * 10000 + hhmm;
}

uint32_t Get_Pre_YYYYMMDD()
{
    uint32_t YYMMDD = Get_YYMMDD();

    YYMMDD = YYMMDD_Sub(YYMMDD);

    return (YYMMDD+20000000);
}

uint32_t Get_Pre_YYMMDD()
{
    uint32_t YYMMDD = Get_YYMMDD();

    YYMMDD = YYMMDD_Sub(YYMMDD);

    return (YYMMDD);
}

uint8_t Chk_YYMMDDhhmm(uint32_t YYMMDDhhmm)
{
	uint32_t YYMMDD;
	uint32_t hhmm;
	uint32_t YY, MM, DD, hh, mm;
	uint8_t ok = 0;


	YYMMDD = YYMMDDhhmm/10000;
	hhmm = YYMMDDhhmm%10000;

	YY = DAY_YY(YYMMDD);
	MM = DAY_MM(YYMMDD);
	DD = DAY_DD(YYMMDD);
	hh = hhmm/100;
	mm = hhmm%100;

	if(26 <= YY && YY <= 45) ok += 1;
	if(1 <= MM && MM <= 12) ok += 1;
	if(1 <= DD && DD <= 31) ok += 1;
	if(hh <= 23) ok += 1;
	if(mm <= 59) ok += 1;
	if(ok==5)
	{
		return 1;
	}

	return 0;

}

uint8_t passPoint = 0;
void TxStr_Faci_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data)
{
    char str[16] = {0};
    if(passPoint)Debug_printf("%s ->",debugStr);

    if      (data / 10000 == FACI_CODE_E) str[0] = 'E';
    else if (data / 10000 == FACI_CODE_P) str[0] = 'P';
    else if (data / 10000 == FACI_CODE_F) str[0] = 'F';
    else return;

    // 나머지 4자리를 0패딩 (예: 14221 → "4221", 10005 → "0005")
    snprintf(str + 1, sizeof(str) - 1, "%04u", (unsigned)(data % 10000));

    memcpy(txAllBuff + idx, str, fixLen);  // +1: str[0] 접두 문자 포함
    if(passPoint)Debug_printf("%s\r\n",str);
}

void TxStr_chimCode_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data)
{
    char str[5] = {0};
    if(passPoint)Debug_printf("%s ->",debugStr);

    snprintf(str, sizeof(str), "%03u", data);

    memcpy(txAllBuff + idx, str, fixLen);
    if(passPoint)Debug_printf("%s\r\n",str);
}
void TxStr_Item_Code_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data)
{
    char str[3] = {0};
    if(passPoint)Debug_printf("%s ->",debugStr);

    switch (data)
    {
        case ITEM_CODE_A: str[0] = 'A'; break;
        case ITEM_CODE_D: str[0] = 'D'; break;
        case ITEM_CODE_T: str[0] = 'T'; break;
        case ITEM_CODE_H: str[0] = 'H'; break;
        case ITEM_CODE_a: str[0] = 'a'; break;
        case ITEM_CODE_d: str[0] = 'd'; break;
        case ITEM_CODE_t: str[0] = 't'; break;
        case ITEM_CODE_h: str[0] = 'h'; break;
        default:
            Debug_printf("[ERR]itemCode\r\n");
        break;
    }
    memcpy(txAllBuff + idx, str, 1);
    if(passPoint)Debug_printf("%s\r\n",str);
}

void TxStr_IP_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint8_t* buff)
{
    char str[16] = {0};
    uint8_t encryBuff[16] = {0,};
    if(passPoint)Debug_printf("%s ->",debugStr);
    snprintf(str, sizeof(str), "%03u.%03u.%03u.%03u",
             buff[0], buff[1], buff[2], buff[3]);

    Greenlink_Encrypt((uint8_t*)str, 15, encryBuff);

    memcpy(txAllBuff + idx, encryBuff, fixLen);  // 점 포함 15바이트 (널 제외)
    if(passPoint)Debug_printf("%s\r\n",str);
}
void TxStr_PW_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data)
{
    char str[16] = {0};
    uint8_t encryBuff[16] = {0,};

    if(passPoint)Debug_printf("%s ->",debugStr);

	snprintf(str, sizeof(str), "%d",data);


    Greenlink_Encrypt((uint8_t*)str, LEN_TCN2_21_11_10, encryBuff);

    memcpy(txAllBuff + idx, encryBuff, fixLen);  // 점 포함 15바이트 (널 제외)
    if(passPoint)Debug_printf("%s\r\n",str);
}

void TxStr_TxMode_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint8_t data)
{
    char str[5] ={0,};
    if(passPoint)Debug_printf("%s ->",debugStr);
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
    if(passPoint)Debug_printf("%s\r\n",str);
}

void TxStr_Int_Input(char* debugStr, uint16_t idx, uint16_t fixLen, uint32_t data )
{
	char str[16] = {0,};
	int len = 0, termLen = 0;

    if(passPoint)Debug_printf("%s ->",debugStr);
	len = snprintf(str, sizeof(str), "%u",data);
//	Debug_printf("%s : idx:%d, fixLen:%d, data:%d\r\n",debugStr, idx, fixLen, data);
	if(len > fixLen)
	{
		//error
		Debug_printf("[ERR]: data too long for fixLen: %u\r\n", data);
		Debug_printf("len : %d \r\n",len);
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
    if(passPoint)Debug_printf("%s\r\n",str);

}
void TxStr_float_Input(char* debugStr, uint16_t idx, uint16_t fixLen, float data)
{
	char str[24] = {0,};
	int len = 0, termLen = 0;
	float roundedData = roundf(data * 100.0f) / 100.0f;

    if(passPoint)Debug_printf("%s ->",debugStr);
	len = snprintf(str, sizeof(str), "%.2f", roundedData);


	if (len > fixLen)
	{
		Debug_printf("[ERR] data too long for fixLen\r\n");
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
	if(passPoint)Debug_printf("%s\r\n",str);
}

void TxStr_Str_Input(char* debugStr, uint16_t idx, uint16_t fixLen, char* str )
{
    if(passPoint)Debug_printf("%s ->",debugStr);
    memcpy(txAllBuff+idx, str, fixLen);
    if(passPoint)Debug_printf("%s\r\n",str);
}





uint8_t Fac_Code_Find(uint32_t facCode)
{
	uint8_t flashIdx;
	for(int i =0 ;i < m_ch.itemNum;i++)
	{
		if(facCode == m_ch.item[i].facCode)
		{
			return i;
		}
		if(m_ch.item[i].facCode == 0)
		{
			m_ch.item[i].facCode = facCode;

			flashIdx = FLASH_GET_IDX_FAC(i);
			Flash_Write_Word(flashIdx, facCode);
			if(passPoint)Debug_printf("facAddr %d \r\n",i);
			if(passPoint)Debug_printf("New facCode %u \r\n",facCode);
			return i;
		}
	}

	return 0xff;
}

uint8_t Fac_Code_Get_Frind(uint8_t coupleAdd, uint32_t* facCodeE, uint32_t* facCodePf)
{
	uint8_t coupleA, coupleNum;
	uint8_t facCodeC;
	for(int i =0 ;i < m_ch.itemNum;i++)
	{
		facCodeC = GET_FAC_C(m_ch.item[i].facCode);

		if(facCodeC == FACI_CODE_E)
		{
			coupleA = m_ch.item[i].couple/10;
			coupleNum = m_ch.item[i].couple%10;
			if(coupleA == coupleAdd)
			{
				*facCodeE = m_ch.item[i].facCode;
				*facCodePf = m_ch.item[coupleNum].facCode;
				return 1;
			}

		}
	}
	return 0;
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
       if(passPoint)Debug_printf("<%hhu>\r\n", viewAdd);//debuge
       return 0;
    }
    else
    {
        Debug_printf("[ERR]strtol_n<%hhu> \r\n", viewAdd);//debuge
        m_Gcmd.passingErr = 1;
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
       if(passPoint)Debug_printf("<%hhu>\r\n", viewAdd);//debuge
       return 0;
    }
    else
    {
        Debug_printf("[ERR]strtol_n<%hhu> \r\n", viewAdd);//debuge
        m_Gcmd.passingErr = 1;
        return 1;
    }

}

void strstr_n(const uint8_t *str, char *strDst ,uint16_t idx, int n, uint8_t viewAdd)
{


    if (str == NULL || strDst == NULL)
    {
        Debug_printf("[ERR]strstr_n<%hhu>\r\n", viewAdd);//debuge
        m_Gcmd.passingErr = 1;
        return;
    }


    memcpy(strDst, (char*)str+idx, n);
    strDst[n] = '\0';

}

uint8_t Check_Faci_Code(const uint8_t *str,  uint32_t* data, uint16_t idx, uint8_t viewAdd)
{
    char strTemp[10] ={0,};

    if (str == NULL)
    {
    	Debug_printf("[ERR]Faci_Code 0<%hhu>\r\n", viewAdd);//debuge
    	m_Gcmd.passingErr = 1;
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
            Debug_printf("[ERR]Faci_Code 1<%hhu>\r\n", viewAdd);//debuge
            m_Gcmd.passingErr = 1;
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
       if(passPoint)Debug_printf("<%hhu> %u\r\n", viewAdd, data);//debuge
       return 0;
    }
    else
    {
        Debug_printf("[ERR]Faci_Code 2<%hhu>\r\n", viewAdd);//debuge
        m_Gcmd.passingErr = 1;
        return 1;
    }
}


uint8_t Check_Item_Code(const uint8_t *str, uint32_t* data, uint16_t idx, uint8_t viewAdd)
{
    if (str == NULL || data == NULL)
	{
		Debug_printf("[ERR]Item_Code 0<%hhu>\r\n", viewAdd);
		m_Gcmd.passingErr = 1;
		return 1;  // NULL 체크 추가 권장
	}
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
            Debug_printf("[ERR]Item_Code 1<%hhu>\r\n", viewAdd);
            m_Gcmd.passingErr = 1;
            return 1;
    }

    *data = val;
    if(passPoint)Debug_printf("<%hhu>\r\n", viewAdd);
    return 0;
}


uint8_t Check_Tx_Mode_Code(const char *str, uint32_t *data, uint16_t idx, uint8_t viewAdd)
{
    if (str == NULL || data == NULL)
    {
        Debug_printf("[ERR]Tx_Mode_Code 1<%hhu>\r\n", viewAdd);
        m_Gcmd.passingErr = 1;
        return 1;
    }

    const char *p = str + idx;   // strncmp는 '\0' 만나면 멈춤 → memcpy 같은 무조건 3바이트 읽기 없음

    if (strncmp(p, TXMODE_HAF, 3) == 0)
        *data = TXMODE_HAF_NUM;
    else if (strncmp(p, TXMODE_ALL, 3) == 0)
        *data = TXMODE_ALL_NUM;
    else if (strncmp(p, TXMODE_FIV, 3) == 0)
        *data = TXMODE_FIV_NUM;
    else
    {
        Debug_printf("[ERR]Tx_Mode_Code 2<%hhu>\r\n", viewAdd);
        m_Gcmd.passingErr = 1;
        return 1;
    }

    if(passPoint)Debug_printf("<%u>\r\n", (unsigned)viewAdd);
    return 0;
}

uint8_t Check_IP_Code(const uint8_t *str,  uint8_t* IPbuff, uint16_t idx, uint8_t viewAdd)
{
    char strTemp[20] = {0,};
    char *endptr;

    if (str == NULL)
    {
        Debug_printf("[ERR] IP_Code 1<%hhu>\r\n", viewAdd);
        m_Gcmd.passingErr = 1;
        return 1;
    }

    memcpy(strTemp, (char*)str+idx, LEN_PRSI17_5_15);
    strTemp[15] = '\0';

    uint8_t tempbuff[4] = {0,};
    uint16_t bigBuff[4] = {0,};

    bigBuff[0] = (uint16_t)strtoul(strTemp, &endptr, 10);
    tempbuff[0] = (uint8_t)strtoul(strTemp, &endptr, 10);

    bigBuff[1] = (uint16_t)strtoul(strTemp+4, &endptr, 10);
	tempbuff[1] = (uint8_t)strtoul(strTemp+4, &endptr, 10);

    bigBuff[2] = (uint16_t)strtoul(strTemp+8, &endptr, 10);
    tempbuff[2] = (uint8_t)strtoul(strTemp+8, &endptr, 10);

    bigBuff[3] = (uint16_t)strtoul(strTemp+12, &endptr, 10);
    tempbuff[3] = (uint8_t)strtoul(strTemp+12, &endptr, 10);


    uint8_t bool1 = (bigBuff[0] <= 255);
    uint8_t bool2 = (bigBuff[1] <= 255);
    uint8_t bool3 = (bigBuff[2] <= 255);
    uint8_t bool4 = (bigBuff[3] <= 255);
    uint8_t bool5 = (endptr != strTemp+12);
    uint8_t bool6 = (*endptr == '\0' || *endptr == ' '); // 혹시 모르니깐 공백도

    if(bool1 && bool2 && bool3 && bool4 && bool5 && bool6)
    {
       memcpy(IPbuff, tempbuff, 4);
       if(passPoint)Debug_printf("<%hhu>\r\n", viewAdd);//debuge
       return 0;
    }
    else
    {
        Debug_printf("[ERR] IP_Code 2<%hhu>\r\n", viewAdd);
        m_Gcmd.passingErr = 1;
        return 1;
    }

}


uint8_t Check_PDUH_Time(uint32_t sDayTime, uint32_t eDayTime, uint8_t viewAdd)
{
	uint16_t sTime = sDayTime % 10000;
	uint16_t eTime = eDayTime % 10000;
	if(sDayTime > Get_YYMMDDhhmm())
	{
		Debug_printf("[ERR] DayTime 0<%hhu>\r\n", viewAdd);//debuge
		m_Gcmd.passingErr = 1;
		return 1;
	}

	if(sDayTime > eDayTime)
	{
		Debug_printf("[ERR] DayTime 1<%hhu>\r\n", viewAdd);//debuge
		m_Gcmd.passingErr = 1;
		return 1;
	}
	else if(sDayTime == eDayTime)
	{
		if(sTime > eTime)
		{
			Debug_printf("[ERR] DayTime 2<%hhu>\r\n", viewAdd);//debuge
			m_Gcmd.passingErr = 1;
			return 1;
		}
	}
	else
	{
		if(passPoint)Debug_printf(">>5_PDUH Pass\r\n");
		return 0;
	}

}






#define RsberyPoint
void Rsbery_Tx_IP(uint8_t* buff, uint16_t cnt)
{
	HAL_UART_Transmit(&huart1, (uint8_t*)buff, cnt, 100);

	Debug_printf("[Ras IP] ");
	HAL_UART_Transmit(&huart2, (uint8_t*)buff, cnt, 100);
	Debug_printf("\r\n");
}


void Rsbery_Tx_CMD(char*buff)
{
	int len = strlen(buff);

	HAL_UART_Transmit(&huart1, (uint8_t*)buff, len, 100);

	Debug_printf("[Ras Cmd] ");
	HAL_UART_Transmit(&huart2, (uint8_t*)buff, len, 100);
	Debug_printf("\r\n");
}

void Rsbery_Time_Passing(uint8_t* str)
{
	uint32_t YYMMDDhhmm = (uint32_t)strtoul((char*)(str+7), NULL, 10);
	uint32_t hhmm;
	Debug_printf("Rx : %s{",CMD_CTRL_TIME_RX);
	Debug_printf("> dayTime : %u \r\n", YYMMDDhhmm);

	if(Chk_YYMMDDhhmm(YYMMDDhhmm))
	{
		m_time.wakeUpDayTime = YYMMDDhhmm;
		m_time.YY = DAY_YY(YYMMDDhhmm);
		m_time.MM = DAY_MM(YYMMDDhhmm);
		m_time.DD = DAY_DD(YYMMDDhhmm);
		hhmm = YYMMDDhhmm%10000;
		m_time.hour = hhmm/100;
		m_time.min = hhmm%100;
		m_time.sec = 0;
		Debug_printf("Now DayTime : %u\r\n", YYMMDDhhmm);
		m_Gcmd.timeGet = 1;
	}
	else Debug_printf("[ERR] Now DayTime : %u\r\n", YYMMDDhhmm);
}


void Rsbery_GwIp_Passing(uint8_t* str)
{
	//Save gw ip
	//[<gwip,192.168.xxx.xxx>]
	int ip0 = 0, ip1 = 0,ip2 = 0, ip3 = 0;
	int parsed = sscanf((char*)str, "[<gwip,%3d.%3d.%3d.%3d>]", &ip0, &ip1, &ip2, &ip3);
	Debug_printf("Rx : %s{",CMD_CTRL_GW_IP_RX);

	if ((parsed == 4) &&
	(ip0 >= 0 && ip0 <= 255) &&
	(ip1 >= 0 && ip1 <= 255) &&
	(ip2 >= 0 && ip2 <= 255) &&
	(ip3 >= 0 && ip3 <= 255))
	{
		m_ch.GWip[0] = ip0;
		m_ch.GWip[1] = ip1;
		m_ch.GWip[2] = ip2;
		m_ch.GWip[3] = ip3;
		Debug_printf("Ok gwip -> %d.%d.%d.%d\r\n", ip0, ip1, ip2, ip3);
		m_Gcmd.gwIpGet = 1;
	}
	else Debug_printf("[ERR] ip1: %d ip2: %d ip3: %d ip4: %d\r\n", ip0, ip1, ip2, ip3);

}

void Rsbery_SurverIp_Tx()
{
	char buff[40] = {0,};
        int len;
	uint8_t ip0, ip1, ip2, ip3;
	Debug_printf("Rx : %s{",CMD_CTRL_SVR_IP);
	Debug_printf("Tx IP svrip\r\n");

	ip0 = flashBuff[FLASH_IDX_IP_OLD_0];
	ip1 = flashBuff[FLASH_IDX_IP_OLD_1];
	ip2 = flashBuff[FLASH_IDX_IP_OLD_2];
	ip3 = flashBuff[FLASH_IDX_IP_OLD_3];
	len = sprintf(buff,"[<svrip,%hhu.%hhu.%hhu.%hhu>]",ip0, ip1, ip2, ip3);
	Rsbery_Tx_IP((uint8_t*)buff, len);

	m_Gcmd.svrIpSet = 1;
}




void Rsbery_PUPG_Secsece()
{
	char buff[40] = {0,};
        int len;
    uint8_t bool1 = (flashBuff[FLASH_IDX_IP_NEW_0] == flashBuff[FLASH_IDX_IP_OLD_0]);//0: 192, 1:168, 2:XXX, 3:XXX
    uint8_t bool2 = (flashBuff[FLASH_IDX_IP_NEW_1] == flashBuff[FLASH_IDX_IP_OLD_1]);//0: 192, 1:168, 2:XXX, 3:XXX
	uint8_t bool3 = (flashBuff[FLASH_IDX_IP_NEW_2] == flashBuff[FLASH_IDX_IP_OLD_2]);//0: 192, 1:168, 2:XXX, 3:XXX
	uint8_t bool4 = (flashBuff[FLASH_IDX_IP_NEW_3] == flashBuff[FLASH_IDX_IP_OLD_3]);//0: 192, 1:168, 2:XXX, 3:XXX
	uint8_t ip0, ip1, ip2, ip3;
	Debug_printf("Rx : %s{",CMD_CTRL_TUPG);
	if(bool1 && bool2 && bool3 && bool4)
	{
		ip0 = flashBuff[FLASH_IDX_IP_OLD_0];
		ip1 = flashBuff[FLASH_IDX_IP_OLD_1];
		ip2 = flashBuff[FLASH_IDX_IP_OLD_2];
		ip3 = flashBuff[FLASH_IDX_IP_OLD_3];
		Debug_printf("old IP svrip %hhu.%hhu.%hhu.%hhu\r\n",ip0, ip1, ip2, ip3);
		m_ch.IP[0] = ip0;
		m_ch.IP[1] = ip1;
		m_ch.IP[2] = ip2;
		m_ch.IP[3] = ip3;
		Goto_TxCmd(ID_TUPG_10);
	}
	else
	{
		Debug_printf("new IP svrip\r\n");
		ip0 = flashBuff[FLASH_IDX_IP_NEW_0];
		ip1 = flashBuff[FLASH_IDX_IP_NEW_1];
		ip2 = flashBuff[FLASH_IDX_IP_NEW_2];
		ip3 = flashBuff[FLASH_IDX_IP_NEW_3];
		len = sprintf(buff,"[<svrip,%hhu.%hhu.%hhu.%hhu>]",ip0, ip1, ip2, ip3);
		Rsbery_Tx_IP((uint8_t*)buff, len);
	}




	Debug_printf("Succese PUPG \r\n");
	m_Gcmd.bootGet = 1;

}
void Rsbery_SurverIp_Err_Tx()//[<svrerr>]수신시
{
    char buff[40] = {0,};
	uint8_t ip0, ip1, ip2, ip3;
	int len;

    Debug_printf("Rx : [ERR]%s{", CMD_CTRL_SVR_ERR);

    m_Gcmd.svrErrCnt++;
    if(m_Gcmd.svrErrCnt > 3)
    {
        Debug_printf("[ERR]svrErr Over\r\n");
        m_Gcmd.svrErrCnt = 0;          // 다음 전환 시도를 위해 리셋

        m_Gcmd.ID    = 0;              // PRSI 대기 해제
        m_Gcmd.txCmd = 0;              // 세워둔 TCN2/TUPG 취소
        m_Gcmd.soketOpenSkip = 0;
        m_Gcmd.svrIpSet = 0;
        return;
    }

    // 구 IP로 복귀 시도
    ip0 = flashBuff[FLASH_IDX_IP_OLD_0];
	ip1 = flashBuff[FLASH_IDX_IP_OLD_1];
	ip2 = flashBuff[FLASH_IDX_IP_OLD_2];
	ip3 = flashBuff[FLASH_IDX_IP_OLD_3];
	Flash_Write_Word(FLASH_IDX_IP_NEW_0, ip0);
	Flash_Write_Word(FLASH_IDX_IP_NEW_1, ip1);
    Flash_Write_Word(FLASH_IDX_IP_NEW_2, ip2);   // NEW도 되돌림
    Flash_Write_Word(FLASH_IDX_IP_NEW_3, ip3);

    m_ch.IP[0] = ip0;
    m_ch.IP[1] = ip1;
    m_ch.IP[2] = ip2;
    m_ch.IP[3] = ip3;

    m_Gcmd.ID = 0;                     // 복귀는 PRSI 전환이 아니므로 해제

    Debug_printf("Back to Old IP svrip\r\n");
    len = sprintf(buff,"[<svrip,%hhu.%hhu.%hhu.%hhu>]",ip0, ip1, ip2, ip3);
    Rsbery_Tx_IP((uint8_t*)buff, len);

    m_Gcmd.svrIpSet = 1;
}


void Rsbery_SurverIp_Ok_Tx()//[<svrok>]수신시
{
	uint8_t bool1 = (flashBuff[FLASH_IDX_IP_NEW_0] != flashBuff[FLASH_IDX_IP_OLD_0]);//0: 192, 1:168, 2:XXX, 3:XXX
	uint8_t bool2 = (flashBuff[FLASH_IDX_IP_NEW_1] != flashBuff[FLASH_IDX_IP_OLD_1]);//0: 192, 1:168, 2:XXX, 3:XXX
	uint8_t bool3 = (flashBuff[FLASH_IDX_IP_NEW_2] != flashBuff[FLASH_IDX_IP_OLD_2]);//0: 192, 1:168, 2:XXX, 3:XXX
	uint8_t bool4 = (flashBuff[FLASH_IDX_IP_NEW_3] != flashBuff[FLASH_IDX_IP_OLD_3]);//0: 192, 1:168, 2:XXX, 3:XXX
	Debug_printf("Rx : %s{",CMD_CTRL_SVR_OK);
	m_Gcmd.svrErrCnt = 0;


	if(bool1 || bool2 || bool3 || bool4)
	{
		Flash_Write_Word(FLASH_IDX_IP_OLD_0, flashBuff[FLASH_IDX_IP_NEW_0]);
	    Flash_Write_Word(FLASH_IDX_IP_OLD_1, flashBuff[FLASH_IDX_IP_NEW_1]);
	    Flash_Write_Word(FLASH_IDX_IP_OLD_2, flashBuff[FLASH_IDX_IP_NEW_2]);
	    Flash_Write_Word(FLASH_IDX_IP_OLD_3, flashBuff[FLASH_IDX_IP_NEW_3]);
	    m_ch.IP[0] = flashBuff[FLASH_IDX_IP_NEW_0];
	    m_ch.IP[1] = flashBuff[FLASH_IDX_IP_NEW_1];
	    m_ch.IP[2] = flashBuff[FLASH_IDX_IP_NEW_2];
	    m_ch.IP[3] = flashBuff[FLASH_IDX_IP_NEW_3];
	}

	// 후속 명령은 분리해서
	if(m_Gcmd.ID == ID_PRSI_17)
	{
	    m_Gcmd.ID = 0;
	    Goto_TxCmd(ID_TCN2_20);
	    Debug_printf("[17] new Ip Server OK \r\n");
	}
	else if(bool1 || bool2 || bool3 || bool4)          // PUPG 경로
	{
	    Goto_TxCmd(ID_TUPG_10);
	    Debug_printf("[10] new Ip Server OK \r\n");
	}
	else                              // 기동 시 접속 확인
	{
	    Rsbery_Tx_CMD(CMD_CTRL_SOKET_T_CLOSE);
	    m_Gcmd.soketTStatus = SOKET_T_OPEN_CLR;
	    m_Gcmd.closeTime = HAL_GetTick();
	    Debug_printf("SurverIp ok\r\n");
	}

}
void Rsbery_PUPG_Fail(uint8_t status)
{
	//Old ip TUPG
	m_ch.IP[0] = flashBuff[FLASH_IDX_IP_OLD_0];
	m_ch.IP[1] = flashBuff[FLASH_IDX_IP_OLD_1];
	m_ch.IP[2] = flashBuff[FLASH_IDX_IP_OLD_2];
	m_ch.IP[3] = flashBuff[FLASH_IDX_IP_OLD_3];
	Goto_TxCmd(ID_TUPG_10);
	m_Gcmd.bootGet = 1;
	switch (status)
	{
		case PUPG_ABORT:
			Debug_printf("[ERR] PUPG ABORT{\r\n");
		break;

		case PUPG_DOWNFAIL:
			Debug_printf("[ERR] PUPG DOWNFAIL{\r\n");
		break;

		case PUPG_FLASHFALE:
			Debug_printf("[ERR] PUPG FLASHFALE{\r\n");
		break;
	}

}


void Rsbery_T_Open_Ok()//라즈로부터 수신시에 호출
{
	Debug_printf("Rx : %s{",CMD_CTRL_SOKET_T_OPEN_OK);
	m_Gcmd.soketTStatus = SOKET_T_OPEN_OK;
	Debug_printf("T Soket Open Ok\r\n");
}
void Rsbery_T_Open_Err()//라즈로부터 수신시에 호출
{
	Debug_printf("Rx : [ERR]%s{",CMD_CTRL_SOKET_T_OPEN_ERR);
	m_Gcmd.soketTStatus = SOKET_T_OPEN_ERR;
	Debug_printf("T Soket Open Err\r\n");
}
void Rsbery_T_Close_Ok()//라즈로부터 수신시에 호출
{
	Debug_printf("Rx : %s{",CMD_CTRL_SOKET_T_CLOSE_OK);
	m_Gcmd.soketTStatus = SOKET_T_CLOSE_OK;
	Debug_printf("T Soket Close Ok\r\n");
}

void Rsbery_P_Open()//라즈로부터 수신시에 호출
{
	Debug_printf("Rx : %s{",CMD_CTRL_SOKET_P_OPEN);
	m_Gcmd.soketPStatus = SOKET_P_OPEN;
	Debug_printf("P Soket Open \r\n");
}

void Rsbery_P_Close()//라즈로부터 수신시에 호출
{
	char buff[40] = {0,};
        int len;
	uint8_t ip0, ip1, ip2, ip3;

	Debug_printf("Rx : %s{",CMD_CTRL_SOKET_P_CLOSE);
	m_Gcmd.soketPStatus = SOKET_P_CLOSE;
	Debug_printf("P Soket Close\r\n");
	if(m_Gcmd.ID == ID_PRSI_17)
	{
		Debug_printf("[17]cmd IP svrip\r\n");
		ip0 = m_ch.IP[0];
		ip1 = m_ch.IP[1];
		ip2 = m_ch.IP[2];
		ip3 = m_ch.IP[3];
		len = sprintf(buff,"[<svrip,%hhu.%hhu.%hhu.%hhu>]",ip0, ip1, ip2, ip3);
		Rsbery_Tx_IP((uint8_t*)buff, len);
	}
}


void Rsbery_Tx_PUPG(uint8_t num, char*str, uint8_t strLen)
{
	char buff[60] = {0,};
	char strCpy[60] = {0,};

	 memcpy(strCpy, str, strLen);

	 snprintf(buff, sizeof(buff), "[<%u,%s>]", num, strCpy);
	 Rsbery_Tx_CMD(buff);

	 HAL_Delay(50);
}






uint8_t Rsbery_REQ_Config()
{
	static uint8_t step = STEP0;
	static uint32_t timeStamp,timeGap = 100;
	static uint8_t txCnt;
	uint32_t yymmdd;
	if(HAL_GetTick()-timeStamp >= timeGap)
	{
		timeStamp = HAL_GetTick();
		switch (step)
		{
			case STEP0:
				if(!m_Gcmd.timeGet)
				{
					txCnt++;
					if(txCnt >= 3){txCnt = 0; step = STEP1;}
					else Rsbery_Tx_CMD(CMD_CTRL_TIME_REQ);
				}
				else{txCnt = 0; step = STEP1; }
			break;

			case STEP1:
				if(!m_Gcmd.gwIpGet)
				{
					txCnt++;
					if(txCnt >= 3){txCnt = 0; step = STEP2;  timeGap = 700;}
					else Rsbery_Tx_CMD(CMD_CTRL_GW_IP_REQ);
				}
				else{txCnt = 0; step = STEP2; timeGap = 700;}
			break;

			case STEP2:
				if(!m_Gcmd.bootGet)
				{
					txCnt++;
					if(txCnt >= 3) {txCnt = 0; step = STEP3; timeGap = 100;}
					else Rsbery_Tx_CMD(CMD_CTRL_BOOT);
				}
				else
				{
					m_Gcmd.initRasComplete = 1;
					return 1;
				}
			break;

			case STEP3:
				if(!m_Gcmd.bootGet && m_Gcmd.timeGet)
				{
					Debug_printf(" ALL TOFH_2: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
					m_time.lastTxDayTime = SD_GetLast_1_TDAH();//YYMMDDhhmm
					yymmdd = m_time.lastTxDayTime/10000;
					Passing_Read_SD_TDDH3(yymmdd, FIV_IDX);
					Passing_Read_SD_TDDH3(yymmdd, HAF_IDX);
					if(m_time.lastTxDayTime && m_time.wakeUpDayTime)
					{
						if(m_time.wakeUpDayTime > m_time.lastTxDayTime+1)
						{
							Debug_printf(">TOFH_2 Ok sDay: %u > eDay: %u\r\n", m_time.lastTxDayTime, m_time.wakeUpDayTime);
							TOFH_2_Start(m_time.lastTxDayTime, m_time.wakeUpDayTime);
						}
						else Debug_printf(">TOFH_2 No  sDay: %u > eDay: %u}\r\n", m_time.lastTxDayTime, m_time.wakeUpDayTime);
					}
					m_Gcmd.initRasComplete = 1;
					return 1;
				}
			break;
		}
	}
	return 0;
}

#define fiv_haf_Point
uint8_t First_Finder(char* str)
{
	for(int i =0 ;i < strlen(str);i++)
	{
		if(str[i] !=' ')return i;
	}
	return 0;
}

uint8_t DisposProtec_Config(uint8_t num)
{
	for(int i =0 ;i < 60;i++)
	{
		if(m_ch.item[num].protect5sec[i] == OPER_START_GRACE) return OPER_START_GRACE;
	}

	for(int i =0 ;i < 60;i++)
	{
		if(m_ch.item[num].protect5sec[i] == PROTEC_STOP_GRACE) return PROTEC_STOP_GRACE;
	}

	for(int i =0 ;i < 60;i++)
	{
		if(m_ch.item[num].protect5sec[i] == ABNORMAL) return ABNORMAL;
	}

	return NORMAL;

}






void TDDH_3_Reset(uint8_t itemMode)
{
	for(int i =0 ;i <m_ch.itemNum ;i++)
	{
		 m_ch.item[i].nomalCnt[itemMode] = 0;
		 m_ch.item[i].abnomalCnt[itemMode] = 0;
		 m_ch.item[i].commuErrCnt[itemMode] = 0;
		 m_ch.item[i].powerOffCnt[itemMode] = 0;
		 m_ch.item[i].fixCnt[itemMode] = 0;
		 if(itemMode == FIV_IDX)
		 {
			for(int j =0 ;j < 6;j++)
			{
				m_ch.item[i].statusBuff[j] = 0;
			}
		 }
	}
	m_ch.TDAHcnt[itemMode] = 0;
	m_ch.TOFHcnt[itemMode] = 0;
	m_ch.cmd2pwOffFlag = 0;
	Flash_Write_Word(FLASH_IDX_PW_OFF, 0);

}
void Save_3_TDDH_when()
{
	uint32_t yyyymmdd = DAY_YYYYMMDD(Get_YYMMDD());
	if((m_time.min ==0) ||(m_time.min ==30))
	{
		SD_Clear_File(CMD_TDDH_HAF, Get_YYMMDD());
		Tx_3_TDDH(HAF_IDX, SD_MODE, yyyymmdd);
	}
	SD_Clear_File(CMD_TDDH_FIV, Get_YYMMDD());
	Tx_3_TDDH(FIV_IDX, SD_MODE, yyyymmdd);

}
uint8_t Five_Sec_operStatus(uint8_t num)
{
	uint8_t couple;
	uint8_t operStatus = N_A;
	if((GET_FAC_C(m_ch.item[num].facCode) == FACI_CODE_E)
	&&(m_ch.item[num].itemCode == ITEM_CODE_A))
	{
		couple = m_ch.item[num].couple%10;

		if((m_ch.item[num].operStatus5Sec == OPER_OK)
		&&(m_ch.item[couple].operStatus5Sec == OPER_NO))//비정상 1,8,9
		{
			if(m_ch.item[num].disposDel9_TimeCnt)
			{
				operStatus = OPER_START_GRACE;
				if((m_ch.item[num].operStatus5SecPre == OPER_OK)
				&&(m_ch.item[couple].operStatus5SecPre == OPER_OK))
				{
					if(!m_ch.item[num].protectDel8_TimeCnt)
						m_ch.item[num].protectDel8_TimeCnt = m_ch.protectDelTime;//만약 30분이면 m_ch.protectDelTime는 6이 대입됨
				}
			}
			else if(m_ch.item[num].protectDel8_TimeCnt)
			{
				if((m_ch.item[num].operStatus5SecPre == OPER_NO)
				&&(m_ch.item[couple].operStatus5SecPre == OPER_NO))
				{
					if(!m_ch.item[num].disposDel9_TimeCnt)
						m_ch.item[num].disposDel9_TimeCnt = m_ch.disposDelTime;//만약 30분이면 m_ch.disposDelTime 6이 대입됨
					operStatus = OPER_START_GRACE;
				}
				else operStatus = PROTEC_STOP_GRACE;
			}
			else
			{
				if((m_ch.item[num].operStatus5SecPre == OPER_NO)
				&&(m_ch.item[couple].operStatus5SecPre == OPER_NO))
				{
					m_ch.item[num].disposDel9_TimeCnt = m_ch.disposDelTime;
					operStatus = OPER_START_GRACE;
				}
				else if((m_ch.item[num].operStatus5SecPre == OPER_OK)
				&&(m_ch.item[couple].operStatus5SecPre == OPER_OK))
				{
					m_ch.item[num].protectDel8_TimeCnt = m_ch.protectDelTime;
					operStatus = PROTEC_STOP_GRACE;
				}
				else
				{
					operStatus = ABNORMAL;
				}

			}
		}
		else
		{
			if(m_ch.item[num].disposDel9_TimeCnt) operStatus = OPER_START_GRACE;
			else if(m_ch.item[num].protectDel8_TimeCnt) operStatus = PROTEC_STOP_GRACE;
			else operStatus = NORMAL;
		}
	}
	else operStatus = N_A;

	return operStatus;

}
void Five_Sec_GetData()
{
	uint8_t valueXXX[10] = {0,};// 원래 항상 저장되야함
	static uint8_t startFlag= 0;
	static uint32_t tdah1Cnt= 0;


	if (!m_time.secChange1) return;
	m_time.secChange1 = 0;

	uint8_t cnt = (m_time.min % 5) * 12 + m_time.sec / 5;
	if (startFlag == 0)
	{
		if (cnt != 0) return;
		startFlag = 1;
	}
	else if (cnt == 0)
	{
		Debug_printf(" ALL TDAH_1: %06u %06u Cnt : %04u{\r\n", Get_YYMMDD(),Get_hhmmss(), tdah1Cnt);
		Save_3_TDDH_when();
		TDAH_1_Start();
		tdah1Cnt++;

	}

    for(int i =0 ;i < m_ch.itemNum;i++)
    {
		if(m_ch.item[i].dataStatus== STATUS_UNDER_CHECK)
		{
			m_ch.item[i].status5sec[cnt] = STATUS_UNDER_CHECK;
			m_ch.item[i].value5sec[cnt] = valueXXX[i];// 측정값

		}
		else if(m_ch.item[i].dataStatus== STATUS_COMM_ERROR)
		{
			m_ch.item[i].status5sec[cnt] = STATUS_COMM_ERROR;
			m_ch.item[i].value5sec[cnt] = 0;
		}
		else
		{
			m_ch.item[i].value5sec[cnt] =valueXXX[i];// 측정값
			if(m_ch.item[i].value5sec[cnt] > 200) // 마이너스값만 아니면 되는데 아닐수가없음 ,최대값이상
				m_ch.item[i].status5sec[cnt] = STATUS_ABNORMAL;
			else
				m_ch.item[i].status5sec[cnt] = STATUS_NORMAL;
		}

    }

	for(int i =0 ;i < m_ch.itemNum;i++)
	{
		if(m_ch.item[i].status5sec[cnt] == STATUS_NORMAL)
		{
			if(m_ch.item[i].value5sec[cnt] >= m_ch.item[i].rangeStandard) m_ch.item[i].operStatus5Sec = OPER_OK;
			else m_ch.item[i].operStatus5Sec = OPER_NO;
		}
		else m_ch.item[i].operStatus5Sec = OPER_NO;
	}

	for(int i =0 ;i < m_ch.itemNum;i++)
	{
		m_ch.item[i].protect5sec[cnt] = Five_Sec_operStatus(i);
	}

	for(int i =0 ;i < m_ch.itemNum;i++)
	{
		m_ch.item[i].operStatus5SecPre = m_ch.item[i].operStatus5Sec;
	}



}
void Five_Min_GetData()
{
	uint8_t nomalCnt = 0, abnomalCnt = 0, commErrCnt = 0, underChkCnt = 0;
	float sum =0;
	uint8_t sumCnt = 0;
	uint8_t min5Cnt = m_ch.status5MinCnt;

	if(m_ch.status5MinCnt >= 6){m_ch.status5MinCnt = 0; return;}//혹시나해서

	for(int i =0 ;i < m_ch.itemNum;i++) //자료상태
	{
		nomalCnt = 0; abnomalCnt = 0; commErrCnt = 0; underChkCnt = 0;
		for(int j =0 ;j < 60; j++)
		{
			switch (m_ch.item[i].status5sec[j])
			{
				case STATUS_UNDER_CHECK:  underChkCnt++;	break;
				case STATUS_COMM_ERROR:	  commErrCnt++;		break;
				case STATUS_ABNORMAL:	  abnomalCnt++;		break;
				case STATUS_NORMAL:	 	  nomalCnt++;		break;
			}

		}
		if (nomalCnt >= 31)
		{
			m_ch.item[i].status[FIV_IDX] = STATUS_NORMAL;
			m_ch.item[i].nomalCnt[FIV_IDX]++;
		}
		else
		{
			if (underChkCnt)
			{
				m_ch.item[i].status[FIV_IDX] = STATUS_UNDER_CHECK;
				m_ch.item[i].fixCnt[FIV_IDX]++;
			}
			else if(commErrCnt)
			{
				m_ch.item[i].status[FIV_IDX] = STATUS_COMM_ERROR;
				m_ch.item[i].commuErrCnt[FIV_IDX]++;
			}
			else
			{
				m_ch.item[i].status[FIV_IDX] = STATUS_ABNORMAL;
				m_ch.item[i].abnomalCnt[FIV_IDX]++;
			}
		}

		m_ch.item[i].statusBuff[min5Cnt] = m_ch.item[i].status[FIV_IDX];
	}

	for(int i =0 ;i < m_ch.itemNum;i++)// 측정값
	{
		sum = 0;
		sumCnt = 0;
		for(int j =0 ;j < 60;j++)
		{
			if (m_ch.item[i].status[FIV_IDX] == STATUS_NORMAL)
			{
				if(m_ch.item[i].status5sec[j] == STATUS_NORMAL)
				{
					sum += m_ch.item[i].value5sec[j];
					sumCnt++;
				}
			}
			else
			{
				sum += m_ch.item[i].value5sec[j];
				sumCnt++;
			}
		}
		m_ch.item[i].value[FIV_IDX] = (sumCnt > 0) ? (sum / sumCnt) : 0;
		m_ch.item[i].valueBuff[min5Cnt] = (uint8_t)m_ch.item[i].value[FIV_IDX];
	}

	for(int i =0 ;i < m_ch.itemNum;i++) //가동상태
	{
		if(m_ch.item[i].status[FIV_IDX] == STATUS_NORMAL)
		{
			if(m_ch.item[i].value[FIV_IDX] >= m_ch.item[i].rangeStandard)
				m_ch.item[i].operStatus[FIV_IDX] = OPER_OK;
			else
				m_ch.item[i].operStatus[FIV_IDX] = OPER_NO;
		}
		else m_ch.item[i].operStatus[FIV_IDX] = OPER_NO;

		m_ch.item[i].operStatusBuff[min5Cnt] = m_ch.item[i].operStatus[FIV_IDX];
	}

	uint8_t couple;

	for(int i =0 ;i < m_ch.itemNum;i++)// 배출시설 정상여부
	{
		if(GET_FAC_C(m_ch.item[i].facCode) == FACI_CODE_E)
		{
			if (m_ch.item[i].itemCode == ITEM_CODE_A)
			{
				if (m_ch.item[i].status[FIV_IDX] == STATUS_NORMAL)
				{
					couple = m_ch.item[i].couple%10;
					if(m_ch.item[couple].status[FIV_IDX] == STATUS_NORMAL)
					{
						m_ch.item[i].protectStatus[FIV_IDX] = DisposProtec_Config(i);
					}
					else m_ch.item[i].protectStatus[FIV_IDX] = NORMAL;
				}
				else m_ch.item[i].protectStatus[FIV_IDX] = NORMAL;
			}
			else m_ch.item[i].protectStatus[FIV_IDX] = N_A;
		}
		else m_ch.item[i].protectStatus[FIV_IDX] = N_A;

		m_ch.item[i].protectStatusBuff[min5Cnt] = m_ch.item[i].protectStatus[FIV_IDX];
	}

	m_ch.status5MinCnt++;

	for(int i =0 ;i < m_ch.itemNum;i++)
	{
		if(m_ch.item[i].protectDel8_TimeCnt)
			m_ch.item[i].protectDel8_TimeCnt--;

		if(m_ch.item[i].disposDel9_TimeCnt)
			m_ch.item[i].disposDel9_TimeCnt--;
	}
}


void Thirty_Min_GetData()
{
	uint8_t nomalCnt = 0, nomalCnt2 = 0, abnomalCnt = 0, commErrCnt = 0, underChkCnt = 0, powerLossCnt = 0;
	uint8_t abnomal8Cnt = 0, abnomal9Cnt = 0;
	float sum =0;
	uint8_t sumCnt = 0;

	uint8_t validCnt = m_ch.status5MinCnt;   /* 이번 30분에 채워진 개수 */
	if (validCnt == 0){Debug_printf("Thirty_Min Out %hhu\r\n", validCnt); return;}            /* 아무것도 없으면 생성 안 함 */
	Debug_printf("Thirty_Min In %hhu \r\n", validCnt);
	for(int i =0 ;i < m_ch.itemNum;i++) //자료상태
	{
		nomalCnt = 0; abnomalCnt = 0; commErrCnt = 0; underChkCnt = 0, powerLossCnt = 0;
		for(int j =0 ;j < validCnt; j++)
		{
			switch (m_ch.item[i].statusBuff[j])
			{
				case STATUS_UNDER_CHECK:  underChkCnt++;	break;
				case STATUS_COMM_ERROR:	  commErrCnt++;		break;
				case STATUS_ABNORMAL:	  abnomalCnt++;		break;
				case STATUS_NORMAL:	 	  nomalCnt++;		break;
				case STATUS_POWER_LOSS:	  powerLossCnt++;	break;
			}
		}
		if (nomalCnt >= 3)
		{
			m_ch.item[i].status[HAF_IDX] = STATUS_NORMAL;
			m_ch.item[i].nomalCnt[HAF_IDX]++;
		}
		else
		{
			if (underChkCnt)
			{
				m_ch.item[i].status[HAF_IDX] = STATUS_UNDER_CHECK;
				m_ch.item[i].fixCnt[HAF_IDX]++;
			}
			else if(powerLossCnt)
			{
				m_ch.item[i].status[HAF_IDX] = STATUS_POWER_LOSS;
			}
			else if(commErrCnt)
			{
				m_ch.item[i].status[HAF_IDX] = STATUS_COMM_ERROR;
				m_ch.item[i].commuErrCnt[HAF_IDX]++;
			}
			else
			{
				m_ch.item[i].status[HAF_IDX] = STATUS_ABNORMAL;
				m_ch.item[i].abnomalCnt[HAF_IDX]++;
			}
		}
	}


	for(int i =0 ;i < m_ch.itemNum;i++)// 측정값
	{
		sum = 0;
		sumCnt = 0;
		for(int j =0 ;j < validCnt;j++)
		{
			if (m_ch.item[i].status[HAF_IDX] == STATUS_NORMAL)
			{
				if(m_ch.item[i].statusBuff[j] == STATUS_NORMAL)
				{
					sum += m_ch.item[i].valueBuff[j];
					sumCnt++;
				}
			}
			else
			{
				sum += m_ch.item[i].valueBuff[j];
				sumCnt++;
			}
		}
		m_ch.item[i].value[HAF_IDX] = (sumCnt > 0) ? (sum / sumCnt) : 0;
	}

	uint8_t operStatusCnt = 0;
	for(int i =0 ;i < m_ch.itemNum;i++) //가동상태
	{
		operStatusCnt = 0;
		for(int j =0 ;j < validCnt;j++)
		{
			if(m_ch.item[i].operStatusBuff[j] == OPER_OK)
			{
				operStatusCnt++;
			}
		}
		m_ch.item[i].operStatus[HAF_IDX] = operStatusCnt;
	}


	for(int i =0 ;i < m_ch.itemNum;i++) //자료상태
	{
		if (m_ch.item[i].protectStatusBuff[0] == N_A)
	    {
	        m_ch.item[i].protectStatus[HAF_IDX] = N_A;
	        continue;
	    }
		nomalCnt2 = 0; abnomalCnt = 0; abnomal8Cnt = 0; abnomal9Cnt = 0;
		for(int j =0 ;j < validCnt; j++)
		{
			switch (m_ch.item[i].protectStatusBuff[j])
			{
				case OPER_START_GRACE:  abnomal9Cnt++;	break;
				case PROTEC_STOP_GRACE:	abnomal8Cnt++;	break;
				case ABNORMAL:	  		abnomalCnt++;	break;
				case NORMAL:	 	    nomalCnt2++;		break;
			}
		}
		if (nomalCnt2 == validCnt)
		{
			m_ch.item[i].protectStatus[HAF_IDX] = NORMAL;
		}
		else
		{
			if (abnomal9Cnt) m_ch.item[i].protectStatus[HAF_IDX] = OPER_START_GRACE;
			else if(abnomal8Cnt) m_ch.item[i].protectStatus[HAF_IDX] = PROTEC_STOP_GRACE;
			else m_ch.item[i].protectStatus[HAF_IDX] = ABNORMAL;
		}
	}


	m_ch.status5MinCnt = 0;


}



uint8_t TOFH_2_Cal( uint8_t itemMode,  uint32_t startDayTime, uint32_t endDayTime)
{
    uint32_t sDay;
    uint32_t eDay = endDayTime/10000;
    uint32_t sTime;
    uint32_t eTime = endDayTime%10000;
	uint32_t yymmdd;
	uint32_t YYMMDDhhmm;
	uint32_t sTimeIdx=0, eTimeIdx=0;
    const uint16_t *tbl = (itemMode == HAF_IDX) ? eep3Day30MinTable : eep3Day5MinTable;
	uint16_t maxNum = (itemMode == HAF_IDX) ? DAY_1_30CNT : DAY_1_5CNT;
	uint16_t lastTime = (itemMode == HAF_IDX) ? 2330 : 2355;
	uint16_t TimeSE[4][2]= {{0,maxNum},{0,maxNum},{0,maxNum},{0,maxNum}};
	uint8_t lastEnd = 0;
	sDay = startDayTime/10000;
	sTime = startDayTime%10000;
	yymmdd = sDay;

	if (sTime >= lastTime)
	{
		Debug_printf("Last [FIV] YYMMDD %u\r\n",sDay);
		Tx_3_TDDH(itemMode, SD_MODE, DAY_YYYYMMDD(sDay));
		TDDH_3_Reset(itemMode);

		m_ch.cmd3DayOffBuff[itemMode][m_ch.cmd3Num[itemMode]++] = sDay;
		m_ch.cmd2PwOffStartDay[itemMode]= sDay;
		sDay  = YYMMDD_Add(sDay);
		sTime = 0;
		yymmdd = sDay;
		lastEnd = 1;
	}

	if(sTime == 0) sTimeIdx = 0;
	else
	{
		for(int i = 0; i < maxNum; i++)
		{
			 if(tbl[i] > sTime){sTimeIdx = i; break;}
		}
	}

	if(eTime >= lastTime) eTimeIdx = maxNum;
	else
	{
		for(int i = 0; i < maxNum; i++)
		{
			if(tbl[i] > eTime){eTimeIdx = i; break;}
		}
	}

	if((sDay == eDay)&&(sTimeIdx >= eTimeIdx))
	{
		Debug_printf("itemMode[%hhu] TOFH Out \r\n", itemMode);
		Debug_printf("Start %u %04u", sDay, tbl[sTimeIdx]);
		Debug_printf("End %u %04u", eDay, tbl[eTimeIdx]);
		return 0;
	}
	memcpy(m_ch.cmd2TimeSE[itemMode], TimeSE, sizeof(TimeSE));
	m_ch.cmd2TimeSE[itemMode][0][IDX_START] = sTimeIdx;

	Debug_printf("Start  %u %04u", yymmdd, tbl[sTimeIdx]);
	for(int i =0 ;i < 4;i++)
	{
		m_ch.cmd2DayBuff[itemMode][i] = yymmdd;
		if(yymmdd == eDay)
		{
			m_ch.cmd2TimeSE[itemMode][i][IDX_END] = eTimeIdx;
			m_ch.cmd2TotalDay[itemMode] = i+1;
			Debug_printf("End  %u %04u", yymmdd, tbl[eTimeIdx]);
			break;
		}
		else
		{
			yymmdd = YYMMDD_Add(yymmdd);
			if(yymmdd > eDay)
			{
				Debug_printf("overday %u\r\n", yymmdd);
				m_ch.cmd2TotalDay[itemMode] = 4;
			}
		}
	}

	yymmdd = sDay;
	for(int i =0 ;i < m_ch.cmd2TotalDay[itemMode]; i++)
	{
		uint16_t startTime = m_ch.cmd2TimeSE[itemMode][i][IDX_START];
		uint16_t endTime = m_ch.cmd2TimeSE[itemMode][i][IDX_END];
		for(int j =startTime ;j < endTime;j++)
		{
			YYMMDDhhmm = DAY_YYMMDDhhmm(yymmdd, tbl[j]);
			Save_1_TDAH_when(itemMode, YYMMDDhhmm);
		}
		m_ch.cmd2DayCnt[itemMode] = i;
		Tx_2_TOFH(itemMode, SD_MODE);
		if(endTime == maxNum)
		{
			m_ch.cmd2PwOffStartDay[itemMode] = yymmdd;
			Tx_TOFH2_Last(itemMode, SD_MODE);
			Tx_3_TDDH(itemMode, SD_MODE, DAY_YYYYMMDD(yymmdd  ));
			TDDH_3_Reset(itemMode);
			m_ch.cmd3DayOffBuff[itemMode][m_ch.cmd3Num[itemMode]++] = yymmdd;
			if(i==0 && !lastEnd)m_ch.cmd2PwOffStartDay[itemMode] = yymmdd;

		}

		yymmdd = YYMMDD_Add(yymmdd);
	}//2608150000
	m_ch.cmd2DayCnt[itemMode] = 0;

	return 1;

}

#define inputPoint

void TDAH_1_Buff_Input(uint8_t itemMode)
{
	char msg[2][4] = {"HAF","FIV"};
	Debug_printf("[IN] [%s] ID_TDAH_1 ", msg[itemMode]);
	m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = itemMode;
	m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TDAH_1;
	m_Gcmd.txTotalCnt++;
	Debug_printf("DayTime : %u\r\n", Get_YYMMDDhhmm());
}

void TOFH_2_Buff_Input(uint8_t itemMode)
{
	char msg[2][4] = {"HAF","FIV"};
	uint32_t yymmdd;
	uint16_t startTiem, endTime;


	Debug_printf("[IN] [%s] ID_TOFH_2\r\n", msg[itemMode]);
	for(int i =0 ;i < m_ch.cmd2TotalDay[itemMode];i++)
	{
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = itemMode;
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TOFH_2;
		m_Gcmd.txTotalCnt++;
		yymmdd = m_ch.cmd2DayBuff[itemMode][i];
		startTiem = m_ch.cmd2TimeSE[itemMode][i][IDX_START];
		endTime = m_ch.cmd2TimeSE[itemMode][i][IDX_END];
		Debug_printf("Day %u S: %u E: %u\r\n", yymmdd, startTiem, endTime);
	}
	Debug_printf("Cnt : %hhu \r\n", m_ch.cmd2TotalDay[itemMode]);


}


void TOFH_2_Last_Mass_Buff_Input(uint8_t itemMode)
{
	char msg[2][4] = {"HAF","FIV"};

	Debug_printf("[IN] [%s] ID_TOFH_2_LAST ", msg[itemMode]);
	if(m_ch.cmd2PwOffStartDay[itemMode])
	{
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = itemMode;
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TOFH_2_LAST;
		m_Gcmd.txTotalCnt++;

		Debug_printf("Day : %u ", m_ch.cmd2PwOffStartDay[itemMode]);
	}
	Debug_printf("Cnt : %hhu\r\n", (m_ch.cmd2PwOffStartDay[itemMode])? 1:0);

}


void TDDH_3_Last_Buff_Input(uint8_t itemMode)
{
	char msg[2][4] = {"HAF","FIV"};
	uint32_t yymmdd;

	Debug_printf("[IN] [%s] ID_TDDH_3_LAST\r\n", msg[itemMode]);
	for(int i =0 ;i < m_ch.cmd3Num[itemMode];i++)
	{
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = itemMode;
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TDDH_3_LAST;
		m_Gcmd.txTotalCnt++;

		yymmdd = m_ch.cmd3DayOffBuff[itemMode][i];
		Debug_printf("Day : %u\r\n", yymmdd);
	}
	Debug_printf("Cnt : %hhu \r\n", m_ch.cmd3Num[itemMode]);

}

void TOFH_2_Last_Buff_Input(uint8_t itemMode)
{
	char msg[2][4] = {"HAF","FIV"};
	Debug_printf("[IN] [%s] TOFH_2 Last ", msg[itemMode]);
	m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = itemMode;
	m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TOFH_2_LAST;
	m_Gcmd.txTotalCnt++;
	Debug_printf("Day : %u\r\n", Get_Pre_YYYYMMDD());
}
void TDDH_3_Buff_Input(uint8_t itemMode)
{
	char msg[2][4] = {"HAF","FIV"};
	Debug_printf("[IN] [%s] TDDH_3 ", msg[itemMode]);
	m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = itemMode;
	m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TDDH_3;
	m_Gcmd.txTotalCnt++;
	Debug_printf("Day : %u\r\n", Get_Pre_YYYYMMDD());
}

void TFDH_4_Buff_Input()
{

	if(m_ch.cmd4Active)
	{
		Debug_printf("[IN] TFDH_4\r\n");
		m_ch.cmd4Active = 0;
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = HAF_IDX;
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TFDH_4;
		m_Gcmd.txTotalCnt++;
	}
}

void TFDH_4_9999_Buff_Input()
{

	if(m_ch.noTxTime == 9999 && m_ch.cmd4Active)
	{
		Debug_printf("[IN] TFDH_4 9999\r\n");
		m_ch.cmd4Active = 0;
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = HAF_IDX;
		m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TFDH_4;
		m_Gcmd.txTotalCnt++;
	}
}

void TNOH_6_Buff_Input()
{
	for(int i =0 ;i < m_ch.itemNum; i++)
	{
		if(((m_ch.item[i].protectStatus[HAF_IDX] != NORMAL) && (m_ch.item[i].protectStatus[HAF_IDX] != N_A))
		||((1<=m_ch.item[i].operStatus[HAF_IDX]) && (m_ch.item[i].operStatus[HAF_IDX]<=5)))
		{
			Debug_printf("[IN] TNOH_6[%d]\r\n",i);
			m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_MODE] = HAF_IDX;
			m_Gcmd.txCmdBuff[m_Gcmd.txTotalCnt][IDX_CMD] = ID_TNOH_6;
			m_Gcmd.txTotalCnt++;
			break;
		}
	}

}
void TxCmd_Buff_Output()
{
	uint8_t itemMode;
	uint8_t cmd;
	m_Gcmd.txCmdCnt++;
	if(m_Gcmd.txCmdCnt >= m_Gcmd.txTotalCnt)
	{
		m_Gcmd.txCmdCnt = 0;
		TX_EOT();
		m_Gcmd.txCmdEnd = 1;
	}
	else
	{
		if(m_Gcmd.ID == ID_TDAH_1) TX_EOT();
		else if(m_Gcmd.txCmdBuff[m_Gcmd.txCmdCnt][IDX_CMD] != m_Gcmd.txCmdBuff[m_Gcmd.txCmdCnt-1][IDX_CMD]) TX_EOT();

		itemMode = m_Gcmd.txCmdBuff[m_Gcmd.txCmdCnt][IDX_MODE];
		cmd  = m_Gcmd.txCmdBuff[m_Gcmd.txCmdCnt][1];
		Goto_TxCmd_Mode(cmd, itemMode);
	}

}
#define startPoint

void TxCmd_Buff_TotalView()
{
	uint8_t itemMode;
	uint8_t cmd;
	Debug_printf("TotalCmd ");
	for(int i =0 ;i < m_Gcmd.txTotalCnt;i++)
	{
		itemMode = m_Gcmd.txCmdBuff[i][IDX_MODE];
		cmd  = m_Gcmd.txCmdBuff[i][1];
		Debug_printf("[%hhu] [%hhu] / ",itemMode, cmd );
	}
}
void TDAH_1_Start()
{

	Five_Min_GetData();
	if(m_time.min == 0 ||m_time.min == 30) Thirty_Min_GetData();

	if(m_Gcmd.txUse)return;
	if(m_Gcmd.soketPStatus == SOKET_P_OPEN) return;

	TxAll_CmdBuff_Clear();
	Debug_printf("@@TDAH_1_Start{\r\n");

	Debug_printf("transferMode %u\r\n", m_ch.transferMode);

	switch (m_ch.transferMode)
	{
		case TXMODE_HAF_NUM:
			Tx_1_TDAH(FIV_IDX, SD_MODE);
			if(m_time.min == 0 ||m_time.min == 30)
			{
				TDAH_1_Buff_Input(HAF_IDX);
				TNOH_6_Buff_Input();
				TFDH_4_9999_Buff_Input();
				Goto_TxCmd_Mode(ID_TDAH_1, HAF_IDX);
			}
		break;

		case TXMODE_FIV_NUM:
			Tx_1_TDAH(HAF_IDX, SD_MODE);
			TDAH_1_Buff_Input(FIV_IDX);
			TFDH_4_9999_Buff_Input();
			Goto_TxCmd_Mode(ID_TDAH_1, FIV_IDX);
		break;

		case TXMODE_ALL_NUM:
			TDAH_1_Buff_Input(FIV_IDX);
			if(m_time.min == 0 ||m_time.min == 30) TDAH_1_Buff_Input(HAF_IDX);
			TFDH_4_9999_Buff_Input();
			Goto_TxCmd_Mode(ID_TDAH_1, FIV_IDX);
		break;
	}
	TxCmd_Buff_TotalView();
	Debug_printf("@@End}\r\n");

}



void TOFH_2_Start(uint32_t startDayTime, uint32_t endDayTime)
{

	uint8_t flag5 = 0;
	uint8_t flag30 = 0;

    //startDay, endDay : YYMMDDmmhh
	if(m_Gcmd.txUse)return;
	if(m_Gcmd.soketPStatus == SOKET_P_OPEN) return;
	Debug_printf("@@TOFH_2_Start{\r\n");

	TxAll_CmdBuff_Clear();
	memset(m_ch.cmd2TimeSE, 0, sizeof(m_ch.cmd2TimeSE));
	m_ch.cmd2DayCnt[FIV_IDX] = 0;
	m_ch.cmd2DayCnt[HAF_IDX] = 0;
	m_ch.cmd2TotalDay[FIV_IDX] = 0;
	m_ch.cmd2TotalDay[HAF_IDX] = 0;
	m_ch.cmd2PwOffStartDay[FIV_IDX] = 0;
	m_ch.cmd2PwOffStartDay[HAF_IDX] = 0;

	flag5 = TOFH_2_Cal(FIV_IDX, startDayTime, endDayTime);
	if(flag5) return;
	flag30 = TOFH_2_Cal(HAF_IDX, startDayTime, endDayTime);

	if(!flag30)Debug_printf("[HAF]skip \r\n");
	Debug_printf("transferMode %u\r\n", m_ch.transferMode);

	m_ch.cmd2pwOffFlag = 1;
	Flash_Write_Word(FLASH_IDX_PW_OFF, 0);
	switch (m_ch.transferMode)
	{
		case TXMODE_HAF_NUM:
			if(flag30)
			{
				TOFH_2_Buff_Input(HAF_IDX);
				TOFH_2_Last_Mass_Buff_Input(HAF_IDX);
				TDDH_3_Last_Buff_Input(HAF_IDX);
				Goto_TxCmd_Mode(ID_TOFH_2, HAF_IDX);
			}
		break;

		case TXMODE_FIV_NUM:
			TOFH_2_Buff_Input(FIV_IDX);
			TOFH_2_Last_Mass_Buff_Input(FIV_IDX);
			TDDH_3_Last_Buff_Input(FIV_IDX);
			Goto_TxCmd_Mode(ID_TOFH_2, FIV_IDX);
		break;

		case TXMODE_ALL_NUM:
			TOFH_2_Buff_Input(FIV_IDX);
			if(flag30) TOFH_2_Buff_Input(HAF_IDX);

			TOFH_2_Last_Mass_Buff_Input(FIV_IDX);
			if(flag30) TOFH_2_Last_Mass_Buff_Input(HAF_IDX);

			TDDH_3_Last_Buff_Input(FIV_IDX);
			if(flag30) TDDH_3_Last_Buff_Input(HAF_IDX);

			Goto_TxCmd_Mode(ID_TOFH_2, FIV_IDX);
		break;
	}
	TxCmd_Buff_TotalView();
	Debug_printf("@@End}\r\n");
}

void TOFH_2_DayOff_Start()
{
	if(m_Gcmd.txUse)return;
	if(m_Gcmd.soketPStatus == SOKET_P_OPEN) return;

	if(m_time.minChange2 && m_ch.cmd2pwOffFlag)
	{
		m_time.minChange2 = 0;
		TxAll_CmdBuff_Clear();
		Debug_printf("@@ID_TOFH_2_OFF_Start{\r\n");
		Debug_printf("transferMode %u\r\n", m_ch.transferMode);

		switch (m_ch.transferMode)
		{
			case TXMODE_HAF_NUM:
				m_ch.cmd2PwOffStartDay[FIV_IDX] = Get_Pre_YYMMDD();
				m_ch.cmd2PwOffStartDay[HAF_IDX] = Get_Pre_YYMMDD();
				Tx_TOFH2_Last(FIV_IDX, SD_MODE);
				TOFH_2_Last_Buff_Input(HAF_IDX);
			break;

			case TXMODE_FIV_NUM:
				m_ch.cmd2PwOffStartDay[FIV_IDX] = Get_Pre_YYMMDD();
				m_ch.cmd2PwOffStartDay[HAF_IDX] = Get_Pre_YYMMDD();
				Tx_TOFH2_Last(HAF_IDX, SD_MODE);
				TOFH_2_Last_Buff_Input(FIV_IDX);
			break;

			case TXMODE_ALL_NUM:
				m_ch.cmd2PwOffStartDay[FIV_IDX] = Get_Pre_YYMMDD();
				m_ch.cmd2PwOffStartDay[HAF_IDX] = Get_Pre_YYMMDD();
				TOFH_2_Last_Buff_Input(FIV_IDX);
				TOFH_2_Last_Buff_Input(HAF_IDX);
			break;
		}



		Goto_TxCmd_Mode(ID_TOFH_2_LAST, FIV_IDX);
		TxCmd_Buff_TotalView();

		m_ch.cmd2OffAllFlag = 1;
		Debug_printf("@@End}\r\n");
	}
}


void TDDH_3_Start()
{
	if(m_Gcmd.txUse)return;
	if(m_Gcmd.soketPStatus == SOKET_P_OPEN) return;

	if(m_time.minChange3)
	{
		m_time.minChange3 = 0;
		Debug_printf(" ALL TDDH_3: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
		TxAll_CmdBuff_Clear();
		Debug_printf("@@TDDH_3_Start{\r\n");


		TDDH_3_Buff_Input(FIV_IDX);
		TDDH_3_Buff_Input(HAF_IDX);


		Goto_TxCmd_Mode(ID_TDDH_3, FIV_IDX);


		SD_Clear_File(CMD_TDDH_FIV, Get_Pre_YYMMDD());
		SD_Clear_File(CMD_TDDH_HAF, Get_Pre_YYMMDD());
		SD_CleanupAll();
		TxCmd_Buff_TotalView();
		Debug_printf("@@End}\r\n");

	}

}


void TFDH_4_Start()
{
	if(m_Gcmd.txUse)return;
	if(m_Gcmd.soketPStatus == SOKET_P_OPEN) return;
	if (m_ch.noTxTime != 9999 && m_ch.cmd4Active)
	{
		uint8_t txHour = m_ch.noTxTime/100;
		uint8_t txMin = m_ch.noTxTime%100;
		if(m_time.minChange4 && m_time.hour == txHour && m_time.min == txMin)
		{
			m_ch.cmd4Active = 0;
			Debug_printf(" ALL TFDH_4: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
			TxAll_CmdBuff_Clear();
			m_time.minChange4 = 0;
			Debug_printf("@@TFDH_4_Start{\r\n");
			TFDH_4_Buff_Input();
			Goto_TxCmd(ID_TFDH_4);
			uint32_t yymmdd = Get_YYMMDD();

			for(int i =0 ;i < 3;i++) yymmdd = YYMMDD_Sub(yymmdd);

			m_ch.cmd4day = yymmdd;
			SD_Set_Idx(0);
			Debug_printf("cmd4day Start %u\r\n", yymmdd);
			TxCmd_Buff_TotalView();
			Debug_printf("@@End}\r\n");
		}

	}

}
void TDUH_5_Start(uint8_t itemMode)
{
	uint8_t exist;
	uint8_t sameDay = 0;
	Debug_printf("@@TDUH_5_Start{\r\n");
	memset(m_ch.cmd5Buff, 0, sizeof(m_ch.cmd5Buff));
	memset(m_ch.cmd5TxBuff, 0, sizeof(m_ch.cmd5TxBuff));
	m_ch.cmd5idxMax = 0;
	m_ch.cmd5idx = 0;

	if(m_ch.cmd5startDay == m_ch.cmd5endDay) sameDay = 1;

	for(int i =IDX_CMD5_FIV ;i <= IDX_CMD3_HAF;i++)
	{
		if(itemMode == TXMODE_HAF_NUM)
		{
			if(i == IDX_CMD5_FIV) continue;
			if(i == IDX_CMD2_FIV) continue;
			if(i == IDX_CMD3_FIV) continue;

		}
		else if(itemMode == TXMODE_FIV_NUM)
		{
			if(i == IDX_CMD5_HAF) continue;
			if(i == IDX_CMD2_HAF) continue;
			if(i == IDX_CMD3_HAF) continue;
			if(i == IDX_CMD6) continue;
		}


		if(i == IDX_CMD3_FIV && sameDay)continue;
		if(i == IDX_CMD3_HAF && sameDay)continue;
		if(i == IDX_CMD2_FIV && sameDay)continue;
		if(i == IDX_CMD2_HAF && sameDay)continue;

		if(TDUH_5_Find(i))
		{
			m_ch.cmd5stepBuff[m_ch.cmd5idxMax] = i;
			m_ch.cmd5idxMax++;
		}
	}

	Goto_TxCmd(ID_TDUH_5);
	m_Gcmd.soketOpenSkip = 1;
	Debug_printf("@@End}\r\n");
}


uint8_t TDUH_5_Find(uint8_t idx)
{
	uint32_t day= m_ch.cmd5startDay;
	uint32_t time = m_ch.cmd5startTime;
	uint32_t eDay = m_ch.cmd5endDay;
	uint32_t eTime = m_ch.cmd5endTime;
	uint8_t status;
	uint8_t sFlag = 0;
	uint16_t getTime = 0;
	while(day<= eDay)
	{
		status = SD_Find_Time_Position(m_ch.cmd5Buff[idx], day, time, &getTime);
		switch (status)
		{
			case SD_OK:
				if(getTime && (day == eDay))//end보다 start가 클때
				{
					if(eTime<getTime)
					{
						return 0;
					}
				}
				m_ch.cmd5TxBuff[idx][IDX_START_DAY] = day;
				m_ch.cmd5TxBuff[idx][IDX_START_POS] = SD_Get_Idx();
				return 1;
			break;
			case SD_NO_EXIST_TIME:
			case SD_NO_EXIST_FILE:
				time = 0;
			break;
		}

		day = YYMMDD_Add(day);

	}

	return 0;
}

uint8_t TDUH_5_Pop(uint8_t idx)
{
	m_ch.cmd5day = m_ch.cmd5TxBuff[idx][IDX_START_DAY];
	uint32_t position = m_ch.cmd5TxBuff[idx][IDX_START_POS];
	SD_Set_Idx(position);

}

void Tx_Start_Config()
{
	TOFH_2_DayOff_Start();
	TDDH_3_Start();
	TFDH_4_Start();
}

#define txPoint
void TX_Memo(uint8_t cmd)
{
	m_Gcmd.ID = cmd;
	m_Gcmd.txMsgTimeStamp = HAL_GetTick();
	m_Gcmd.txMsgFlag = 1;
}




void TX_ACK(uint8_t cmd)
{
    uint8_t msg[1] = {MSG_ACK,};
    Tx_Cmd_Instruction(msg, 1);
    Debug_printf(">>ACK[%hhu]\r\n",cmd);

	m_Gcmd.ID = cmd;
    m_Gcmd.txAckTimeStamp = HAL_GetTick();
    m_Gcmd.txAckFlag = 1;
}

void TX_NAK()
{
    uint8_t msg[1] = {MSG_NAK,};
    Tx_Cmd_Instruction(msg, 1);
    Debug_printf(">>NAK\r\n");
}
void TX_EOT()
{
    uint8_t msg[1] = {MSG_EOT,};
    Tx_Cmd_Instruction(msg, 1);
    Rsbery_Tx_CMD(CMD_CTRL_SOKET_T_CLOSE);
    Debug_printf(">>EOT\r\n");
    m_Gcmd.txUse = 0;
	m_Gcmd.closeTime = HAL_GetTick();
}

void Save_1_TDAH_when(uint8_t itemMode, uint32_t YYMMDDhhmm)
{

	Tx_Head_DbugMsg(0, itemMode);
	Debug_printf(" Save_1_TDAH_when %u{\r\n", YYMMDDhhmm);
	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,      LEN_COMM_1_4,      CMD_TDUH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      0);
	TxStr_Int_Input("allLan",     IDX_COMM_4,        LEN_COMM_4_4,      TOTAL_LEN_TDAH1(m_ch.itemNum));

	TxStr_Str_Input("transferMode", IDX_COMM_5, LEN_COMM_5_3,
						(itemMode == HAF_IDX) ? TXMODE_HAF : TXMODE_FIV);

	// 바디 (고정)
	TxStr_Int_Input("measureTime",  IDX_TDAH1_6,   LEN_TDAH1_6_10,    YYMMDDhhmm);
	TxStr_Int_Input("measureQty",    IDX_TDAH1_7,   LEN_TDAH1_7_2,     m_ch.itemNum);
	m_ch.TOFHcnt[itemMode]++;

	uint32_t YYMMDD = YYMMDDhhmm/ 10000;
	uint16_t hhmm = YYMMDDhhmm%10000;
	uint16_t mm = hhmm/100;
	// 바디 (가변)
	int commIdx;
	uint8_t statusBuffCnt = (mm%30)/5;
	for(int i = 0; i < m_ch.itemNum; i++)
	{
		commIdx = i * IDX_TDAH1_CYCLE;

		TxStr_Faci_Input("facCode",     commIdx + IDX_TDAH1_8n,  LEN_TDAH1_8n_5,  m_ch.item[i].facCode);
		TxStr_Item_Code_Input("itemCode", commIdx + IDX_TDAH1_9n,  LEN_TDAH1_9n_1,  m_ch.item[i].itemCode);

		TxStr_float_Input("measureValue", commIdx + IDX_TDAH1_10n,  LEN_TDAH1_10n_6, VALUE_ZERO);
		TxStr_Int_Input("measureStatus",  commIdx + IDX_TDAH1_11n,  LEN_TDAH1_11n_1, STATUS_POWER_LOSS);

		TxStr_Int_Input("operStatus",     commIdx + IDX_TDAH1_12n,  LEN_TDAH1_12n_1, VALUE_ZERO);

		uint8_t ps;
		if ((GET_FAC_C(m_ch.item[i].facCode) == FACI_CODE_E) && (m_ch.item[i].itemCode == ITEM_CODE_A))
		    ps = NORMAL;      /* 배출 대표전류: 자료상태 비정상 → 정상(1) */
		else
		    ps = N_A;
		TxStr_Int_Input("protectStatus",    commIdx + IDX_TDAH1_13n,  LEN_TDAH1_13n_1, ps);
		m_ch.item[i].powerOffCnt[itemMode]++;

		if(itemMode == FIV_IDX)
		{
			m_ch.item[i].statusBuff[statusBuffCnt] = STATUS_POWER_LOSS;
		}
	}

	// 테일러 (CRC)
	uint16_t crcidx = m_ch.itemNum* IDX_TDAH1_CYCLE + IDX_TDAH1_8n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;




	switch (itemMode)
	{
		case FIV_IDX: SD_Write_Record(CMD_TDUH_FIV,YYMMDD, hhmm, txAllBuff, m_Gcmd.txCnt); break;
		case HAF_IDX: SD_Write_Record(CMD_TDUH_HAF, YYMMDD, hhmm, txAllBuff, m_Gcmd.txCnt); break;
	}
	TxAllBuff_Clear();
	Debug_printf("}");

}


// ========================================================================================
// [1] 측정자료 전송 (TDAH) - 가변 구조
// ========================================================================================
void Tx_1_TDAH(uint8_t itemMode, uint8_t txEn)
{
	Tx_Head_DbugMsg(txEn, itemMode);
	Debug_printf(" 1_TDAH %06u %06u{\r\n", Get_YYMMDD(), Get_hhmmss());


	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",          IDX_COMM_1,      LEN_COMM_1_4,      CMD_TDAH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      0);
	TxStr_Int_Input("allLan",     IDX_COMM_4,        LEN_COMM_4_4,      TOTAL_LEN_TDAH1(m_ch.itemNum));
	TxStr_Str_Input("dataType", IDX_COMM_5, LEN_COMM_5_3,
					(itemMode == HAF_IDX) ? TXMODE_HAF : TXMODE_FIV);

	// 바디 (고정)
	TxStr_Int_Input("measureTime",  IDX_TDAH1_6,   LEN_TDAH1_6_10,    Get_Pre_YYMMDDhhmm());
	TxStr_Int_Input("measureQty",    IDX_TDAH1_7,   LEN_TDAH1_7_2,     m_ch.itemNum);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < m_ch.itemNum; i++)
	{
		commIdx = i * IDX_TDAH1_CYCLE;

		TxStr_Faci_Input("facCode",     commIdx + IDX_TDAH1_8n,  LEN_TDAH1_8n_5,  m_ch.item[i].facCode);
		TxStr_Item_Code_Input("itemCode", commIdx + IDX_TDAH1_9n,  LEN_TDAH1_9n_1,  m_ch.item[i].itemCode);

		TxStr_float_Input("measureValue", commIdx + IDX_TDAH1_10n,  LEN_TDAH1_10n_6, m_ch.item[i].value[itemMode]);
		TxStr_Int_Input("measureStatus",  commIdx + IDX_TDAH1_11n,  LEN_TDAH1_11n_1, m_ch.item[i].status[itemMode]);

		TxStr_Int_Input("operStatus",     commIdx + IDX_TDAH1_12n,  LEN_TDAH1_12n_1, m_ch.item[i].operStatus[itemMode]);
		TxStr_Int_Input("protectStatus",    commIdx + IDX_TDAH1_13n,  LEN_TDAH1_13n_1, m_ch.item[i].protectStatus[itemMode]);
	}


	// 테일러 (CRC)
	uint16_t crcidx = m_ch.itemNum* IDX_TDAH1_CYCLE + IDX_TDAH1_8n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	if(txEn)
	{
		Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
		TX_Memo(ID_TDAH_1);
	}

	switch (itemMode)
	{
		case FIV_IDX: SD_Write_Record(CMD_TDUH_FIV, Get_YYMMDD(), Get_hhmm(), txAllBuff, m_Gcmd.txCnt); break;
		case HAF_IDX: SD_Write_Record(CMD_TDUH_HAF, Get_YYMMDD(), Get_hhmm(), txAllBuff, m_Gcmd.txCnt); break;
	}
	m_ch.TDAHcnt[itemMode]++;
	Debug_printf("}\r\n");

}

// ========================================================================================
// [2] 전원단절구간자료 전송 (TOFH) - 가변 구조
// ========================================================================================
void Tx_2_TOFH(uint8_t itemMode, uint8_t txEn)
{
	uint8_t dayCnt = m_ch.cmd2DayCnt[itemMode];
	uint32_t YYMMDD = m_ch.cmd2DayBuff[itemMode][dayCnt];
	uint16_t startTime = m_ch.cmd2TimeSE[itemMode][dayCnt][IDX_START];
	uint16_t endTime = m_ch.cmd2TimeSE[itemMode][dayCnt][IDX_END];
	uint16_t hhmm = 0;
	int commIdx = 0;
	uint16_t idxCnt = 0;

	Tx_Head_DbugMsg(txEn, itemMode);
	Debug_printf(" 2_TOFH %u{\r\n",YYMMDD);

	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TOFH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      0);
	TxStr_Str_Input("dataType", IDX_COMM_5, LEN_COMM_5_3,
					(itemMode == HAF_IDX) ? TXMODE_HAF : TXMODE_FIV);

	// 바디 (고정)
	TxStr_Int_Input("powerOffDay",  IDX_TOFH2_6,   LEN_TOFH2_6_8,  DAY_YYYYMMDD(YYMMDD  ));
	// 바디 (가변)



	for(int i = startTime; i < endTime; i++)
	{
		if(itemMode == FIV_IDX) hhmm = eep3Day5MinTable[i];
		else if(itemMode == HAF_IDX) hhmm = eep3Day30MinTable[i];

		commIdx = idxCnt * IDX_TOFH2_CYCLE;
		TxStr_Int_Input("powerOffTime", commIdx + IDX_TOFH2_8n, LEN_TOFH2_8n_4, hhmm);
		idxCnt++;
	}
	TxStr_Int_Input("powerOffCnt",  IDX_TOFH2_7,   LEN_TOFH2_7_3,     idxCnt);
	TxStr_Int_Input("allLan",		  IDX_COMM_4,	 LEN_COMM_4_4,		TOTAL_LEN_TOFH2(idxCnt));


	// 테일러 (CRC)
	uint16_t crcidx = idxCnt* IDX_TOFH2_CYCLE + IDX_TOFH2_8n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;

	if(txEn)
	{
		Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
		TX_Memo(ID_TOFH_2);
	}

	uint8_t sd_len = idxCnt*4;

	switch (itemMode)
	{
		case FIV_IDX:
			if(!txEn)SD_Write_Record(CMD_TOFH_FIV, YYMMDD, Get_hhmm(), (txAllBuff +IDX_TOFH2_8n), sd_len);
		break;

		case HAF_IDX:
			if(!txEn)SD_Write_Record(CMD_TOFH_HAF, YYMMDD, Get_hhmm(), (txAllBuff +IDX_TOFH2_8n), sd_len);
		break;
	}
	m_ch.cmd2DayCnt[itemMode]++;
	Debug_printf("}\r\n");
}
void Tx_TOFH2_Last(uint8_t itemMode, uint8_t txEn)
{
	uint16_t len = 0;
	uint8_t outPut = 0;
	uint32_t YYMMDD = m_ch.cmd2PwOffStartDay[itemMode];
	Tx_Head_DbugMsg(txEn, itemMode);
	Debug_printf("  TOFH2_Last %u {\r\n",YYMMDD);


	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TOFH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      0);
	TxStr_Str_Input("dataType", IDX_COMM_5, LEN_COMM_5_3,
					(itemMode == HAF_IDX) ? TXMODE_HAF : TXMODE_FIV);
	TxStr_Int_Input("powerOffDay",  IDX_TOFH2_6,   LEN_TOFH2_6_8,  DAY_YYYYMMDD(YYMMDD  ));

	uint16_t totalIdx= IDX_TOFH2_8n;
	uint16_t totalLen = 0;
	uint16_t tofhCnt = 0;
	SD_Set_Idx(0);
	for(int i =0 ;i < 10;i++)//하루에 최대 정전횟수 임시 10
	{
		if (itemMode == FIV_IDX) outPut = SD_Only_Read(CMD_TOFH_FIV, YYMMDD, &len);
		else outPut = SD_Only_Read(CMD_TOFH_HAF, YYMMDD, &len);

		memcpy(txAllBuff+totalIdx, readSDbuff, len);
		totalIdx += len;
		totalLen += len;

		if(outPut == SD_END_NEXT_FILE) break;
	}
	tofhCnt = totalLen/4;

	TxStr_Int_Input("allLan",		  IDX_COMM_4,	 LEN_COMM_4_4,		TOTAL_LEN_TOFH2(tofhCnt));
	TxStr_Int_Input("powerOffCnt",	IDX_TOFH2_7,   LEN_TOFH2_7_3,	  tofhCnt);

	uint16_t crcidx = tofhCnt* IDX_TOFH2_CYCLE + IDX_TOFH2_8n;
	append_crc16(txAllBuff, crcidx);
	m_Gcmd.txCnt = crcidx+LEN_CRC;

	if(txEn)
	{
		Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
		TX_Memo(ID_TOFH_2_LAST);
	}

	switch (itemMode)
	{
		case FIV_IDX:
			SD_Clear_File(CMD_TOFH_FIV, YYMMDD);
			SD_Write_Record(CMD_TOFH_FIV, YYMMDD, 2359, txAllBuff, m_Gcmd.txCnt);//SD_Find_Time_Position에 걸리기 위해서 어차피 하나밖에 없고
		break;
		case HAF_IDX:
			SD_Clear_File(CMD_TOFH_HAF, YYMMDD);
			SD_Write_Record(CMD_TOFH_HAF, YYMMDD, 2359, txAllBuff, m_Gcmd.txCnt);//SD_Find_Time_Position에 걸리기 위해서 어차피 하나밖에 없고
		break;
	}
	Debug_printf("}\r\n");
}

// ========================================================================================
// [3] 일일 마감자료 전송 (TDDH) - 가변 구조
// ========================================================================================
void Tx_3_TDDH(uint8_t itemMode, uint8_t txEn, uint32_t yyyymmdd)
{
	Tx_Head_DbugMsg(txEn, itemMode);
	Debug_printf(" 3_TDDH %u{\r\n",yyyymmdd);

	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TDDH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      0);
	TxStr_Int_Input("allLan",         IDX_COMM_4,    LEN_COMM_4_4,      TOTAL_LEN_TDDH3(m_ch.itemNum));
	TxStr_Str_Input("dataType", IDX_COMM_5, LEN_COMM_5_3,
					(itemMode == HAF_IDX) ? TXMODE_HAF : TXMODE_FIV);

//============================

//============================

	// 바디 (고정)

	m_ch.dayCnt[itemMode] = m_ch.TDAHcnt[itemMode] + m_ch.TOFHcnt[itemMode];
	TxStr_Int_Input("closeDate",    IDX_TDDH3_6,   LEN_TDDH3_6_8,     yyyymmdd);
	TxStr_Int_Input("dayCnt",       IDX_TDDH3_7,   LEN_TDDH3_7_3,     m_ch.dayCnt[itemMode]);
	TxStr_Int_Input("TDAHcnt",      IDX_TDDH3_8,   LEN_TDDH3_8_3,     m_ch.TDAHcnt[itemMode]);
	TxStr_Int_Input("TOFHcnt",      IDX_TDDH3_9,   LEN_TDDH3_9_3,     m_ch.TOFHcnt[itemMode]);
	TxStr_Int_Input("measureQty",   IDX_TDDH3_10,  LEN_TDDH3_10_2,    m_ch.itemNum );

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < m_ch.itemNum; i++)
	{
		commIdx = i * IDX_TDDH3_CYCLE;
		TxStr_Faci_Input("facCode",     commIdx + IDX_TDDH3_11n,  LEN_TDDH3_11n_5, m_ch.item[i].facCode);
		TxStr_Item_Code_Input("itemCode", commIdx + IDX_TDDH3_12n,  LEN_TDDH3_12n_1, m_ch.item[i].itemCode);
		TxStr_Int_Input("nomalCnt",     commIdx + IDX_TDDH3_13n,  LEN_TDDH3_13n_3, m_ch.item[i].nomalCnt[itemMode] );
		TxStr_Int_Input("FultCnt",      commIdx + IDX_TDDH3_14n,  LEN_TDDH3_14n_3, m_ch.item[i].abnomalCnt[itemMode]);
		TxStr_Int_Input("commuErrCnt",  commIdx + IDX_TDDH3_15n,  LEN_TDDH3_15n_3, m_ch.item[i].commuErrCnt[itemMode]);
		TxStr_Int_Input("powerOffCnt",  commIdx + IDX_TDDH3_16n,  LEN_TDDH3_16n_3, m_ch.item[i].powerOffCnt[itemMode]);
		TxStr_Int_Input("fixCnt",       commIdx + IDX_TDDH3_17n,  LEN_TDDH3_17n_3, m_ch.item[i].fixCnt[itemMode]);
	}
	// 테일러 (CRC)
	uint16_t crcidx = m_ch.itemNum * IDX_TDDH3_CYCLE + IDX_TDDH3_11n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	uint32_t yymmdd = yyyymmdd - 20000000;
	if(txEn)
	{
		Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
		TX_Memo(ID_TDDH_3);
		TDDH_3_Reset(itemMode);
	}

	switch (itemMode)
	{
		case FIV_IDX: SD_Write_Record(CMD_TDDH_FIV, yymmdd, 2359, txAllBuff, m_Gcmd.txCnt); break;//SD_Find_Time_Position에 걸리기 위해서 어차피 하나밖에 없고
		case HAF_IDX: SD_Write_Record(CMD_TDDH_HAF, yymmdd, 2359, txAllBuff, m_Gcmd.txCnt); break;//SD_Find_Time_Position에 걸리기 위해서 어차피 하나밖에 없고
	}
	Debug_printf("}\r\n");
}
void Tx_3_TDDH_Last(uint8_t itemMode)
{
	Tx_Head_DbugMsg(1, itemMode);
	Debug_printf(" 3_TDDH_PwOff{\r\n");

	uint8_t exit = 0;
	uint8_t cnt;
	uint32_t yymmdd;

	for(int i =0 ;i < 5;i++)
	{
		TxAllBuff_Clear();
		cnt = m_ch.cmd3DayOffCnt[itemMode];
		yymmdd = m_ch.cmd3DayOffBuff[itemMode][cnt];

		if (itemMode == FIV_IDX) m_ch.cmd3next = SD_Read_Day_TxMsg(CMD_TDDH_FIV, yymmdd);
		else 				  m_ch.cmd3next = SD_Read_Day_TxMsg(CMD_TDDH_HAF, yymmdd);


		switch (m_ch.cmd3next)
		{
			case SD_OK:
			case SD_END_NEXT_FILE:
				TX_Memo(ID_TDDH_3_LAST);
				m_ch.cmd3DayOffCnt[itemMode]++;
				Debug_printf(">> 3_TDDH_PwOff End(SD Ok) \r\n");
				exit = 1;
			break;

			case SD_ERR_NEXT_FILE:
				m_ch.cmd3DayOffCnt[itemMode]++;
				HAL_Delay(100);
				Debug_printf("[3][ERR] NextFile %u\r\n",m_ch.cmd3DayOffBuff[itemMode][m_ch.cmd3DayOffCnt[itemMode]]);
			break;

			case SD_ERR_RETRY:
				HAL_Delay(100);
				Debug_printf("[3][ERR] Retry \r\n");
			break;
		}
		if(exit)break;
	}
	if(!exit) Debug_printf("[3][ERR] SdCard  5cnt Over \r\n");
	Debug_printf("}\r\n");

}
// ========================================================================================
// [4] 미전송자료 전송 (TFDH) - 가변 구조
// ========================================================================================

void Tx_4_TFDH()
{
	Debug_printf(">>[Tx] 4_TFDH{\r\n");
	uint8_t exit = 0;
	for(int i =0 ;i < 5;i++)
	{
		TxAllBuff_Clear();
		m_ch.cmd4next = SD_Read_Day_TxMsg(CMD_TFDH, m_ch.cmd4day);
		Debug_printf("TxDay : %u  TxTime : %u\r\n",m_ch.cmd4TxDay,m_ch.cmd4TxTime);

		switch (m_ch.cmd4next)
		{
			case SD_OK:
			case SD_END_NEXT_FILE:
				TX_Memo(ID_TFDH_4);
				Debug_printf(">> 4_TFDH End(SD Ok) \r\n");
				exit = 1;
			break;

			case SD_ERR_NEXT_FILE:
				m_ch.cmd4day = YYMMDD_Add(m_ch.cmd4day);
				HAL_Delay(100);
				Debug_printf("[4][ERR] NextFile %u\r\n",m_ch.cmd4day);
			break;

			case SD_ERR_RETRY:
				HAL_Delay(100);
				Debug_printf("[4][ERR] Retry \r\n");
			break;
		}
		if(exit)break;
	}
	if(!exit) Debug_printf("[4][ERR] SdCard 5cnt Over \r\n");
	Debug_printf("}\r\n");

}

// ========================================================================================
// [5] 저장자료 응답 (TDUH) - 가변 구조
// ========================================================================================



void Tx_5_TDUH()
{
	uint8_t exit = 0;
	uint8_t idx;
	Debug_printf(">>[Tx] 5_TDUH{\r\n");
	for(int i =0 ;i < 5;i++)
	{
		TxAllBuff_Clear();
		m_ch.cmd5SdStatus = SD_Read_Day_TxMsg(m_ch.cmd5Buff[m_ch.cmd5idx], m_ch.cmd5day);
		Debug_printf("TxDay : %u  TxTime : %u\r\n",m_ch.cmd5TxDay,m_ch.cmd5TxTime);

		switch (m_ch.cmd5SdStatus)
		{
			case SD_OK:
				TX_Memo(ID_TDUH_5);
				exit = 1;
			break;
			case SD_ERR_NEXT_FILE:
			case SD_NO_EXIST_FILE:
				if(m_ch.cmd5endDay > m_ch.cmd5day)
				{
					m_ch.cmd5day = YYMMDD_Add(m_ch.cmd5day);
					Debug_printf("Next Ok %u\r\n",m_ch.cmd5day);
					SD_Set_Idx(0);
				}
				else
				{
					m_ch.cmd5idx++;
					if(m_ch.cmd5idx < m_ch.cmd5idxMax)
					{
						m_ch.cmd5Eot = 1;
						idx = m_ch.cmd5stepBuff[m_ch.cmd5idx];
						TDUH_5_Pop(idx);
					}
					else TX_EOT();

				}
				HAL_Delay(100);
			break;

			case SD_END_NEXT_FILE:
				TX_Memo(ID_TDUH_5);
				exit = 1;

				if(m_ch.cmd5endDay > m_ch.cmd5day)
				{
					m_ch.cmd5day = YYMMDD_Add(m_ch.cmd5day);
					Debug_printf("Next Ok %u\r\n",m_ch.cmd5day);
					SD_Set_Idx(0);
				}
				else
				{
					m_ch.cmd5idx++;
					if(m_ch.cmd5idx < m_ch.cmd5idxMax)
					{
						m_ch.cmd5Eot = 1;
						idx = m_ch.cmd5stepBuff[m_ch.cmd5idx];
						TDUH_5_Pop(idx);
					}
					else m_ch.cmd5Finsh = 1;
				}
			break;

			case SD_CMD5_OVER_NEXT:
				HAL_Delay(100);
				TX_EOT();
				m_ch.cmd5idx++;
				if(m_ch.cmd5idx < m_ch.cmd5idxMax)
				{
					m_ch.cmd5ReTry = 1;
					idx = m_ch.cmd5stepBuff[m_ch.cmd5idx];
					TDUH_5_Pop(idx);
				}

			break;

			case SD_ERR_RETRY:
				HAL_Delay(100);
				Debug_printf("[5][ERR] Retry \r\n");
			break;
		}
		if(exit)break;
	}
	if(!exit) Debug_printf("[5][ERR] SdCard  5cnt Over \r\n");
	Debug_printf("}\r\n");
}


// ========================================================================================
// [6] 5분자료 전송대상 정보 (TNOH) - 가변 구조
// ========================================================================================
void Tx_6_TNOH()
{

	if(m_ch.transferMode != TXMODE_HAF_NUM) return;
	Debug_printf(">>[Tx] 6_TNOH{\r\n");
	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TNOH);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      0);
	TxStr_TxMode_Input("transferMode", IDX_COMM_5,   LEN_COMM_5_3,      TXMODE_HAF_NUM);

	// 바디 (고정)
	TxStr_Int_Input("measureTime",   IDX_TNOH6_6,   LEN_TNOH6_6_10,    Get_Pre_YYMMDDhhmm());
	TxStr_Int_Input("measureQty",    IDX_TNOH6_7,   LEN_TNOH6_7_2,     m_ch.itemNum);

	// 바디 (가변)



	int commIdx;
	uint8_t cnt = 0;
	for(int i = 0; i < m_ch.itemNum; i++)
	{
		if(((m_ch.item[i].protectStatus[HAF_IDX] != NORMAL) && (m_ch.item[i].protectStatus[HAF_IDX] != N_A))
		||((1<=m_ch.item[i].operStatus[HAF_IDX]) && (m_ch.item[i].operStatus[HAF_IDX]<=5)))
		{
			cnt++;
			commIdx = i * IDX_TNOH6_CYCLE;

			TxStr_Faci_Input("facCode",     commIdx + IDX_TNOH6_8n, LEN_TNOH6_8n_5,  m_ch.item[i].facCode);
			TxStr_Item_Code_Input("itemCode", commIdx + IDX_TNOH6_9n, LEN_TNOH6_9n_1,  m_ch.item[i].itemCode);
			TxStr_Int_Input("operStatus",   commIdx + IDX_TNOH6_10n, LEN_TNOH6_10n_1, m_ch.item[i].operStatus[HAF_IDX]);
			TxStr_Int_Input("protectStatus",  commIdx + IDX_TNOH6_11n, LEN_TNOH6_11n_1, m_ch.item[i].protectStatus[HAF_IDX]);
		}
	}
	TxStr_Int_Input("allLan",		  IDX_COMM_4,	 LEN_COMM_4_4,		TOTAL_LEN_TNOH6(cnt));

	// 테일러 (CRC)
	uint16_t crcidx = cnt * IDX_TNOH6_CYCLE + IDX_TNOH6_8n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
	TX_Memo(ID_TNOH_6);
	SD_Write_Record(CMD_TNOH, Get_YYMMDD(), Get_hhmm(), txAllBuff, m_Gcmd.txCnt);
	Debug_printf("}\r\n");
}

// ========================================================================================
// [9] 서버시간 조회 요청 (TTIM) - 바디 없는 고정 길이 구조
// ========================================================================================
void Tx_9_TTIM()
{
	Debug_printf(">>[Tx] 9_TTIM{\r\n");
	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,    LEN_COMM_1_4,      CMD_TTIM);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,    LEN_COMM_2_7,      m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,    LEN_COMM_3_3,      0);
	TxStr_Int_Input("allLan",         IDX_COMM_4,    LEN_COMM_4_4,      TOTAL_LEN_TTIM9);

    append_crc16(txAllBuff, IDX_TTIM9_CRC);
    m_Gcmd.txCnt = IDX_TTIM9_CRC+LEN_CRC;
	Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
	TX_Memo(ID_TTIM_9);
	Debug_printf("}\r\n");
}

// ========================================================================================
// [10] 게이트웨이 업그레이드 결과 전송 (TUPG) - 고정 길이 구조
// ========================================================================================
void Tx_10_TUPG()
{
	Debug_printf(">>[Tx] 10_TUPG{\r\n");
	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,     LEN_COMM_1_4,     CMD_TUPG);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,     LEN_COMM_2_7,     m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,     LEN_COMM_3_3,     0);
	TxStr_Int_Input("allLan",         IDX_COMM_4,     LEN_COMM_4_4,     TOTAL_LEN_TUPG10);

	// 바디
	TxStr_IP_Input("IP",           IDX_TUPG10_5,   LEN_TUPG10_5_16,  m_ch.IP);
	TxStr_IP_Input("GWip",         IDX_TUPG10_6,   LEN_TUPG10_6_16,  m_ch.GWip);
	TxStr_Int_Input("manuCode",    IDX_TUPG10_7,   LEN_TUPG10_7_2,   m_ch.manuCode);
	TxStr_Str_Input("GWmodel",     IDX_TUPG10_8,   LEN_TUPG10_8_20,  m_ch.GWmodel);
	TxStr_Str_Input("fwVer",       IDX_TUPG10_9,   LEN_TUPG10_9_20,  m_ch.fwVer);
	TxStr_Str_Input("heshCode",    IDX_TUPG10_10,  LEN_TUPG10_10_32,  m_ch.heshCode);
    append_crc16(txAllBuff, IDX_TUPG10_CRC);

    m_Gcmd.txCnt = IDX_TUPG10_CRC+LEN_CRC;
	Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
	TX_Memo(ID_TUPG_10);
	Debug_printf("}\r\n");
}

// ========================================================================================
// [11] 버전정보 응답 전송 (TVER) - 고정 길이 구조
// ========================================================================================
void Tx_11_TVER()
{
	Debug_printf(">>[Tx] 11_TVER{\r\n");
	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,     LEN_COMM_1_4,     CMD_TVER);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,     LEN_COMM_2_7,     m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,     LEN_COMM_3_3,     0);
	TxStr_Int_Input("allLan",         IDX_COMM_4,     LEN_COMM_4_4,     TOTAL_LEN_TVER11);

	// 바디
	TxStr_IP_Input("IP",           IDX_TVER11_5,   LEN_TVER11_5_16,  m_ch.IP);
	TxStr_IP_Input("GWip",         IDX_TVER11_6,   LEN_TVER11_6_16,  m_ch.GWip);
	TxStr_Int_Input("manuCode",    IDX_TVER11_7,   LEN_TVER11_7_2,   m_ch.manuCode);
	TxStr_Str_Input("GWmodel",     IDX_TVER11_8,   LEN_TVER11_8_20,  m_ch.GWmodel);
	TxStr_Str_Input("fwVer",       IDX_TVER11_9,   LEN_TVER11_9_20,  m_ch.fwVer);
	TxStr_Str_Input("heshCode",    IDX_TVER11_10,  LEN_TVER11_10_32,  m_ch.heshCode);
	append_crc16(txAllBuff, IDX_TVER11_CRC);
    m_Gcmd.txCnt = IDX_TVER11_CRC+LEN_CRC;
	Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
	TX_Memo(ID_TVER_11);
	Debug_printf("}\r\n");
}

// ========================================================================================
// [15] 방지시설 정상여부 관계정보 조회 응답 (TFCR) - 가변 구조
// ========================================================================================
void Tx_15_TFCR()
{
	Debug_printf(">>[Tx] 15_TFCR{\r\n");
	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,     LEN_COMM_1_4,     CMD_TFCR);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,     LEN_COMM_2_7,     m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,     LEN_COMM_3_3,     0);
	TxStr_Int_Input("allLan",         IDX_COMM_4,     LEN_COMM_4_4,     TOTAL_LEN_TFCR15(m_ch.protectRelyCnt));

	// 바디 (고정)
	TxStr_Int_Input("protectRelyCnt", IDX_TFCR15_5, LEN_TFCR15_5_2, m_ch.protectRelyCnt);

	// 바디 (가변)
	int commIdx;
	uint32_t facCodeE = 0, facCodePf = 0;
	for(int i = 0; i < m_ch.protectRelyCnt; i++)
	{
		commIdx = i * IDX_TFCR15_CYCLE;
		if(Fac_Code_Get_Frind(i,&facCodeE, &facCodePf))
		{
			TxStr_Int_Input("disposBuff",   commIdx + IDX_TFCR15_6n, LEN_TFCR15_6n_5, facCodeE);
			TxStr_Int_Input("protectBuff",  commIdx + IDX_TFCR15_7n, LEN_TFCR15_7n_5, facCodePf);
		}
	}

	// 테일러 (CRC)
	uint16_t crcidx = m_ch.itemNum* IDX_TFCR15_CYCLE + IDX_TFCR15_6n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
	TX_Memo(ID_TFCR_15);
	Debug_printf("}\r\n");
}

// ========================================================================================
// [21] 게이트웨이 설정정보 응답/전송 (TCN2) - 가변 구조
// ========================================================================================
void Tx_21_TCN2()
{
	Debug_printf(">>[Tx] 21_TCN2{\r\n");
	TxAllBuff_Clear();
	// 공통 헤더
	TxStr_Str_Input("cmd",            IDX_COMM_1,      LEN_COMM_1_4,     CMD_TCN2);
	TxStr_Int_Input("workPlaceCode",  IDX_COMM_2,      LEN_COMM_2_7,     m_ch.workPlaceCode);
	TxStr_chimCode_Input("chimCode",  IDX_COMM_3,      LEN_COMM_3_3,     0);
	TxStr_Int_Input("allLan",         IDX_COMM_4,      LEN_COMM_4_4,     TOTAL_LEN_TCN2_21(m_ch.itemNum));


	// 바디 (고정)
	TxStr_IP_Input("IP",           IDX_TCN2_21_5,   LEN_TCN2_21_5_16,  m_ch.IP);
	TxStr_IP_Input("GWip",         IDX_TCN2_21_6,   LEN_TCN2_21_6_16,  m_ch.GWip);
	TxStr_Int_Input("manuCode",    IDX_TCN2_21_7,   LEN_TCN2_21_7_2,   m_ch.manuCode);
	TxStr_Str_Input("GWmodel",     IDX_TCN2_21_8,   LEN_TCN2_21_8_20,  m_ch.GWmodel);
	TxStr_Str_Input("fwVer",       IDX_TCN2_21_9,   LEN_TCN2_21_9_20,  m_ch.fwVer);
	TxStr_Str_Input("heshCode",    IDX_TCN2_21_10,  LEN_TCN2_21_10_32, m_ch.heshCode);
	TxStr_PW_Input("passWard",    IDX_TCN2_21_11,  LEN_TCN2_21_11_16_RAW, m_ch.passWard);
	TxStr_Int_Input("noTxTime",    IDX_TCN2_21_12,  LEN_TCN2_21_12_4,  m_ch.noTxTime);
	TxStr_Int_Input("transferMode",  IDX_TCN2_21_13,  LEN_TCN2_21_13_1,  m_ch.transferMode);
	TxStr_Int_Input("disposDelTime",    IDX_TCN2_21_14,  LEN_TCN2_21_14_3,  m_ch.disposDelTime);
	TxStr_Int_Input("protectDelTime",  IDX_TCN2_21_15,  LEN_TCN2_21_15_3,  m_ch.protectDelTime);
	TxStr_Int_Input("measureQty",  IDX_TCN2_21_16,  LEN_TCN2_21_16_2,  m_ch.itemNum);

	// 바디 (가변)
	int commIdx;
	for(int i = 0; i < m_ch.itemNum; i++)
	{
		commIdx = i * IDX_TCN2_21_CYCLE;

		TxStr_Faci_Input("facCode",   commIdx + IDX_TCN2_21_17n, LEN_TCN2_21_17_5n, m_ch.item[i].facCode);
		TxStr_Item_Code_Input("itemCode", commIdx + IDX_TCN2_21_18n, LEN_TCN2_21_18_1n, m_ch.item[i].itemCode);
		TxStr_float_Input("valueMin",   commIdx + IDX_TCN2_21_19n, LEN_TCN2_21_19_6n, m_ch.item[i].rangeMin);
		TxStr_float_Input("valueMax",   commIdx + IDX_TCN2_21_20n, LEN_TCN2_21_20_6n, m_ch.item[i].rangeMax);
		TxStr_float_Input("valueSdrd",  commIdx + IDX_TCN2_21_21n, LEN_TCN2_21_21_6n, m_ch.item[i].rangeStandard);
	}
	// 테일러 (CRC)
	uint16_t crcidx = m_ch.itemNum* IDX_TCN2_21_CYCLE + IDX_TCN2_21_17n;
    append_crc16(txAllBuff, crcidx);
    m_Gcmd.txCnt = crcidx+LEN_CRC;
	Tx_Cmd_Instruction(txAllBuff, m_Gcmd.txCnt);
	TX_Memo(ID_TCN2_20);
	Debug_printf("}\r\n");
}
















#define rxPoint

void TFDH_4_ACK()
{
	SD_Delete_Record(CMD_TFDH, m_ch.cmd4TxDay, m_ch.cmd4TxTime);
	if(m_ch.cmd4next == SD_END_NEXT_FILE)
	{
		m_ch.cmd4next = 0;
		m_ch.cmd4day = YYMMDD_Add(m_ch.cmd4day);
		if(m_ch.cmd4day > Get_YYMMDD())
		{
			m_ch.cmd4day = 0;
			TX_EOT();
			m_Gcmd.txCmdEnd = 1;
		}
		else
		{
			Goto_TxCmd(ID_TFDH_4);
		}

	}
	else
	{
		Goto_TxCmd(ID_TFDH_4);
	}

}


void TDUH_5_ACK()
{
	if(m_ch.cmd5Eot)
	{
		TX_EOT();
		Goto_TxCmd(ID_TDUH_5);
	}
	else if(m_ch.cmd5Finsh)
	{
		TX_EOT();
	}
	else
	{
		Goto_TxCmd(ID_TDUH_5);
	}

}



void Rx_Passing_ACK()
{
	uint8_t cmdId = m_Gcmd.ID;
	Debug_printf(">>RX ACK [%hhu]{\r\n",cmdId);

	switch (m_Gcmd.ID)
	{
		case ID_TDAH_1:
		case ID_TOFH_2:
		case ID_TOFH_2_LAST:
		case ID_TDDH_3_LAST:
		case ID_TDDH_3:
		case ID_TNOH_6:
			TxCmd_Buff_Output();
		break;

		case ID_TFDH_4:
			TFDH_4_ACK();
		break;

		case ID_TDUH_5:
			TDUH_5_ACK();
		break;


		case ID_TUPG_10:
		case ID_TVER_11:
		case ID_TFCR_15:
		case ID_TFCR_16:
		case ID_TCN2_20:
			TX_EOT();
			m_Gcmd.txCmdEnd = 1;
		break;
	}
	m_Gcmd.ID = 0;
	m_Gcmd.txMsgFlag = 0;


	m_Gcmd.stepNoneAck = STEP0;
	m_Gcmd.stepNoneMsg = STEP0;

}
void RX_Fail_NextStep()
{
	switch (m_Gcmd.ID)
	{
		case ID_TDAH_1:
			SD_Write_Record(CMD_TFDH, Get_YYMMDD(), Get_hhmm(), txAllBuff, m_Gcmd.txCnt);
			m_ch.cmd4Active = 1;
			TxCmd_Buff_Output();
		break;

		case ID_TOFH_2:
		case ID_TOFH_2_LAST:
		case ID_TDDH_3_LAST:
		case ID_TDDH_3:
			TxCmd_Buff_Output();
		break;

		case ID_TFDH_4:
			TFDH_4_ACK();
		break;

		case ID_TDUH_5:
			TDUH_5_ACK();
		break;

		case ID_TNOH_6:
			SD_Write_Record(CMD_TFDH, Get_YYMMDD(), Get_hhmm(), txAllBuff, m_Gcmd.txCnt);
			TxCmd_Buff_Output();
		break;

		case ID_TUPG_10:
		case ID_TVER_11:
		case ID_TFCR_15:
		case ID_TFCR_16:
		case ID_TCN2_20:
			TX_EOT();
		break;
	}
}

void Rx_Passing_NAK()
{
	if(m_Gcmd.txMsgFlag)
	{
		switch (m_Gcmd.stepNoneMsg)
		{
			case STEP0:
				TxAllBuff_ReSend();
				Debug_printf("[ERR]NAK ReSend[%hhu]{\r\n",m_Gcmd.ID);
				m_Gcmd.stepNoneMsg = STEP1;
			break;

			case STEP1://미송신
				Debug_printf("[ERR] NextStep [%hhu]\r\n",m_Gcmd.ID);
				RX_Fail_NextStep();
				m_Gcmd.ID = 0;
				m_Gcmd.txMsgFlag = 0;
				m_Gcmd.stepNoneMsg = STEP0;
			break;
		}
	}

}

void Rx_Passing_EOT()// 내가 서버로부터 바디가 있는 데이터를 받았을때만 해당
{
	Debug_printf(">>RX EOT [%hhu]{\r\n",m_Gcmd.ID);
	m_Gcmd.txAckFlag = 0;

	switch (m_Gcmd.ID)
	{
	    case ID_PFST_7 ://TCN2
	    case ID_PSEP_8 ://TCN2
	    case ID_PFCC_13://TCN2
	    case ID_PAST_14://TCN2
	    case ID_PDAT_18://TCN2
	    case ID_PODT_19://TCN2
			Goto_TxCmd(ID_TCN2_20);
			m_Gcmd.ID = 0;
	    break;

	    case ID_PTIM_9 ://END
	    	m_Gcmd.ID = 0;
	    	m_Gcmd.txCmdEnd = 1;
	    break;

	    case ID_PSET_12://END
			m_Gcmd.ID = 0;
			if(m_time.psetOldTime+1<Get_YYMMDDhhmm()) //잠든시간 14분, 깬시간 16분 : 15분 누락됨
			{
				Debug_printf(" ALL TOFH_2: %06u %06u\r\n", Get_YYMMDD(),Get_hhmmss());
				TOFH_2_Start(m_time.psetOldTime,Get_YYMMDDhhmm());
			}
			else m_Gcmd.txCmdEnd = 1;
		break;
	    case ID_PFRS_16://TFCR
			Goto_TxCmd(ID_TFCR_16);
			m_Gcmd.ID = 0;
		break;
		case ID_PRBT_22:
			Debug_printf(">>PRBT_22 PwOff Now!{\r\n");
			NVIC_SystemReset();
		break;
	}
}


void Passing_Read_SD_TDDH3(uint32_t YYMMDD,uint8_t itemMode)
{
	uint16_t len = 0;
	uint32_t tempData = 0;
    uint32_t ch = 0;
    uint32_t itemModeQ = 0;
	uint32_t itemBuff[10][7] = {0,};
	uint32_t totalBuff[5] = {0,};
	uint8_t outPut = 0;

	int commIdx;
	Debug_printf(">>SD Read 3_TDDH{\r\n");
	SD_Set_Idx(0);
	if (itemMode == FIV_IDX) outPut = SD_Only_Read(CMD_TDDH_FIV, YYMMDD, &len);
	else outPut = SD_Only_Read(CMD_TDDH_HAF, YYMMDD, &len);

	if((TOTAL_LEN_TDDH3(m_ch.itemNum) == len) && (outPut == SD_END_NEXT_FILE))
	{
	    if(strtol_n(readSDbuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
	    if(strtol_n(readSDbuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
	    if(strtol_n(readSDbuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, len, len, VIEW_ADD_3))return;
	    if(Check_Tx_Mode_Code((char*)readSDbuff, &itemModeQ, IDX_COMM_5, VIEW_ADD_4))return;

	    if(strtol_n(readSDbuff, &totalBuff[0], IDX_TDDH3_6, LEN_TDDH3_6_8, 0, 0xffffffff, VIEW_ADD_5))return;
	    if(strtol_n(readSDbuff, &totalBuff[1], IDX_TDDH3_7, LEN_TDDH3_7_3, 0, 999, VIEW_ADD_6))return;
	    if(strtol_n(readSDbuff, &totalBuff[2], IDX_TDDH3_8, LEN_TDDH3_8_3, 0, 999, VIEW_ADD_7))return;
	    if(strtol_n(readSDbuff, &totalBuff[3], IDX_TDDH3_9, LEN_TDDH3_9_3, 0, 999, VIEW_ADD_8))return;
	    if(strtol_n(readSDbuff, &totalBuff[4], IDX_TDDH3_10, LEN_TDDH3_10_2, 0, 99, VIEW_ADD_9))return;

		for(int i = 0; i < m_ch.itemNum; i++)
		{
			commIdx = i * IDX_TDDH3_CYCLE;
			if(Check_Faci_Code(readSDbuff, &itemBuff[i][0], commIdx + IDX_TDDH3_11n, VIEW_ADD_5))return;// 시설코드
            if(Check_Item_Code(readSDbuff, &itemBuff[i][1], commIdx + IDX_TDDH3_12n, VIEW_ADD_6))return;
            if(strtol_n(readSDbuff, &itemBuff[i][2], commIdx + IDX_TDDH3_13n, LEN_TDDH3_13n_3, 0, 999, VIEW_ADD_7))return;
            if(strtol_n(readSDbuff, &itemBuff[i][3], commIdx + IDX_TDDH3_14n, LEN_TDDH3_14n_3, 0, 999, VIEW_ADD_8))return;
            if(strtol_n(readSDbuff, &itemBuff[i][4], commIdx + IDX_TDDH3_15n, LEN_TDDH3_15n_3, 0, 999, VIEW_ADD_9))return;
            if(strtol_n(readSDbuff, &itemBuff[i][5], commIdx + IDX_TDDH3_16n, LEN_TDDH3_16n_3, 0, 999, VIEW_ADD_10))return;
            if(strtol_n(readSDbuff, &itemBuff[i][6], commIdx + IDX_TDDH3_17n, LEN_TDDH3_17n_3, 0, 999, VIEW_ADD_11))return;
		}
//		YYMMDD = totalBuff[0];
		m_ch.dayCnt[itemMode] = totalBuff[1];
		m_ch.TDAHcnt[itemMode] = totalBuff[2];
		m_ch.TOFHcnt[itemMode] = totalBuff[3];
//		m_ch.itemNum = totalBuff[4];
		for(int i = 0; i < m_ch.itemNum; i++)
		{
			m_ch.item[i].facCode = itemBuff[i][0];
			m_ch.item[i].itemCode = itemBuff[i][1];
			m_ch.item[i].nomalCnt[itemMode] = itemBuff[i][2];
			m_ch.item[i].abnomalCnt[itemMode] = itemBuff[i][3];
			m_ch.item[i].commuErrCnt[itemMode] = itemBuff[i][4];
			m_ch.item[i].powerOffCnt[itemMode] = itemBuff[i][5];
			m_ch.item[i].fixCnt[itemMode] = itemBuff[i][6];
		}
	}
	memset(readSDbuff, 0, sizeof(readSDbuff));
	Debug_printf("}\r\n");
}

void Rx_Passing_5_PDUH()
{

    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;
    uint32_t itemMode = 0;

	Debug_printf(" ALL 5_PDUH: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 5_PDUH Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PDUH5)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PDUH5, TOTAL_LEN_PDUH5, VIEW_ADD_3))return;

        if(Check_Tx_Mode_Code((char*)m_Gcmd.passingBuff, &itemMode, IDX_COMM_5, VIEW_ADD_4))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PDUH5_6, LEN_PDUH5_6_10, MIN_5_PDUH_6, MAX_5_PDUH_6, VIEW_ADD_5))return;
        if(strtol_n(m_Gcmd.passingBuff, chkBuff+1, IDX_PDUH5_7, LEN_PDUH5_7_10, MIN_5_PDUH_7, MAX_5_PDUH_7, VIEW_ADD_6))return;
		if(Check_PDUH_Time(chkBuff[0], chkBuff[1], VIEW_ADD_7))return;
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PDUH5_CRC))return;

        Debug_printf("[R] OK\r\n");

        m_ch.cmd5startDay= chkBuff[0]/10000;
        m_ch.cmd5startTime= chkBuff[0]%10000;
        Debug_printf("[R]start Day %u Time %u \r\n",m_ch.cmd5startDay, m_ch.cmd5startTime);

		if(chkBuff[1] > Get_YYMMDDhhmm()) chkBuff[1] = Get_YYMMDDhhmm();
        m_ch.cmd5endDay= chkBuff[1]/10000;
        m_ch.cmd5endTime = chkBuff[1]%10000;
        Debug_printf("[R]end Day %u Time %u \r\n",m_ch.cmd5endDay, m_ch.cmd5endTime);

		m_ch.cmd5day = m_ch.cmd5startDay;


		TDUH_5_Start(itemMode);
		Debug_printf(">>5_PDUH End\r\n");

    }
    else m_Gcmd.passingErr = 1;



}

// [7] PFST - 미전송자료 전송시간 변경 요청
void Rx_Passing_7_PFST()//
{
    uint32_t tempData = 0;
    uint32_t chkBuff[1] = {0,};
    uint32_t ch = 0;
	Debug_printf(" ALL 7_PFST: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 7_PFST Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PFST7)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PFST7, TOTAL_LEN_PFST7, VIEW_ADD_3))return;
        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PFST7_5, LEN_PFST7_5_4, MIN_7_PFST_5, MAX_7_PFST_5, VIEW_ADD_4))return; // 전송시간
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PFST7_CRC))return;
        Debug_printf("[R] OK\r\n");

        m_ch.noTxTime = chkBuff[0];
        Flash_Write_Word(FLASH_IDX_NO_TXTIME, m_ch.noTxTime);
        Debug_printf("[R]noTxTime %u \r\n",chkBuff[0]);
        TX_ACK(ID_PFST_7);
		Debug_printf(">>7_PFST End\r\n");

    }
    else m_Gcmd.passingErr = 1;
}
// [8] PSEP - 비밀번호 변경 요청
void Rx_Passing_8_PSEP()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[1] = {0,};
    uint8_t tempBuff[16] = {0,};
    uint8_t passWard[10] = {0,};
    uint32_t ch = 0;
	Debug_printf(" ALL 8_PSEP: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 8_PSEP Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PSEP8)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PSEP8, TOTAL_LEN_PSEP8, VIEW_ADD_3))return;

        memcpy(tempBuff, m_Gcmd.passingBuff+IDX_PSEP8_5, 16);
        Greenlink_Decrypt(tempBuff, passWard, LEN_PSEP8_5_16_RAW ,LEN_PSEP8_5_10);

        if(strtol_n(passWard, chkBuff, 0, LEN_PSEP8_5_10, MIN_8_PSEP_5, MAX_8_PSEP_5, VIEW_ADD_4))return; // 암호화 패스워드 //u32에 맞게


        if(Check_crc16(m_Gcmd.passingBuff, IDX_PSEP8_CRC))return;
        Debug_printf("[R] OK\r\n");

        m_ch.passWard = chkBuff[0];
        Flash_Write_Word(FLASH_IDX_PASSWARD, m_ch.passWard);
        Debug_printf("[R]passWard %u \r\n",chkBuff[0]);
        TX_ACK(ID_PSEP_8);
		Debug_printf(">>8_PSEP End\r\n");

    }
    else m_Gcmd.passingErr = 1;
}

// [9] PTIM - 서버시간 응답
void Rx_Passing_9_PTIM()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;

    Debug_printf(">>Rx : 9_PTIM Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PTIM9)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PTIM9, TOTAL_LEN_PTIM9, VIEW_ADD_3))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PTIM9_5_1, LEN_PTIM9_5_1_6, MIN_9_PTIM_5_1, MAX_9_PTIM_5_1, VIEW_ADD_4))return; // 서버YYMMDD (내가쪼갬)
        if(strtol_n(m_Gcmd.passingBuff, chkBuff+1, IDX_PTIM9_5_2, LEN_PTIM9_5_2_6, MIN_9_PTIM_5_2, MAX_9_PTIM_5_2, VIEW_ADD_5))return; // 서버hhmmss (내가쪼갬)
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PTIM9_CRC))return;
        Debug_printf("[R] OK\r\n");

        Debug_printf("[R]sevrDay %u \r\n",chkBuff[0]);
        Debug_printf("[R]sevrTime %u \r\n",chkBuff[1]);
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
		/////////오직 TTIM을 위해서/////////////
		m_Gcmd.ID = 0;
		m_Gcmd.txMsgFlag = 0;
		m_Gcmd.stepNoneAck = STEP0;
		m_Gcmd.stepNoneMsg = STEP0;
		//////////////////////
        TX_ACK(ID_PTIM_9);
        Debug_printf(">>9_PTIM End\r\n");
    }
    else m_Gcmd.passingErr = 1;
}

// [10] PUPG - 게이트웨이 업그레이드 요청
void Rx_Passing_10_PUPG()
{
    uint32_t tempData = 0;
    uint8_t tempIpBuff[4] = {0,};
    uint8_t inBuff[144] = {0,};
    uint8_t outBuff[131] = {0,};
    uint32_t ch = 0;
    char strData1[1] = {0,};
    char strData5R[5] = {0,};
	char strData10R[10] = {0,};
	char strData40R[40] = {0,};
	char strData50R[50] = {0,};
	char strData10R_2[10] = {0,};
	Debug_printf(" ALL 10_PUPG: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 10_PUPG Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PUPG10)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PUPG10, TOTAL_LEN_PUPG10, VIEW_ADD_3))return;

        memcpy(inBuff, m_Gcmd.passingBuff+IDX_PUPG10_5_RAW, LEN_PUPG10_5_144_RAW);
        Greenlink_Decrypt(inBuff, outBuff, LEN_PUPG10_5_144_RAW ,LEN_PUPG10_5_131);


        strstr_n(outBuff, strData1, IDX_PUPG10_5, LEN_PUPG10_5_1, VIEW_ADD_4);   // FTP 타입
        strstr_n(outBuff, strData40R, IDX_PUPG10_6, LEN_PUPG10_6_40, VIEW_ADD_4);  // FTP IP/Domain
        strstr_n(outBuff, strData5R, IDX_PUPG10_7, LEN_PUPG10_7_5, VIEW_ADD_4);  // FTP Port
        strstr_n(outBuff, strData50R, IDX_PUPG10_8, LEN_PUPG10_8_50, VIEW_ADD_4);  // 파일 경로
        strstr_n(outBuff, strData10R, IDX_PUPG10_9, LEN_PUPG10_9_10, VIEW_ADD_4);  // FTP ID
        strstr_n(outBuff, strData10R_2, IDX_PUPG10_10, LEN_PUPG10_10_10, VIEW_ADD_4);// FTP PWD
        if(Check_IP_Code(outBuff, tempIpBuff, IDX_PUPG10_11, VIEW_ADD_10))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PUPG10_CRC))return;
        Debug_printf("[R] OK\r\n");

        m_ch.FTPtype = strData1[0];
        uint8_t idx;

		idx = First_Finder(strData40R);
        memcpy(m_ch.FTPipDomain, strData40R + idx , LEN_PUPG10_6_40-idx);

		idx = First_Finder(strData5R);
        memcpy(m_ch.FTPport, strData5R+idx, LEN_PUPG10_7_5-idx);

		idx = First_Finder(strData50R);
        memcpy(m_ch.road, strData50R+idx, LEN_PUPG10_8_50-idx);

		idx = First_Finder(strData10R);
        memcpy(m_ch.FTPid, strData10R+idx, LEN_PUPG10_9_10-idx);

		idx = First_Finder(strData10R_2);
        memcpy(m_ch.FTPpwd, strData10R_2+idx, LEN_PUPG10_10_10-idx);

        memcpy(m_ch.IP, tempIpBuff, 4);

		Flash_Write_Word(FLASH_IDX_IP_NEW_0, m_ch.IP[0]);
		Flash_Write_Word(FLASH_IDX_IP_NEW_1, m_ch.IP[1]);
		Flash_Write_Word(FLASH_IDX_IP_NEW_2, m_ch.IP[2]);
		Flash_Write_Word(FLASH_IDX_IP_NEW_3, m_ch.IP[3]);
		Flash_Write_All_Word();

        Debug_printf("[R]FTPtype %c \r\n",strData1[0]);
        Debug_printf("[R]FTPipDomain %s \r\n",strData40R);
        Debug_printf("[R]FTPport %s \r\n",strData5R);
        Debug_printf("[R]road %s \r\n",strData50R);
        Debug_printf("[R]FTPid %s \r\n",strData10R);
        Debug_printf("[R]FTPpwd %s \r\n",strData10R_2);
        Debug_printf("[R] %hhu.%hhu.%hhu.%hhu \r\n",tempIpBuff[0], tempIpBuff[1], tempIpBuff[2], tempIpBuff[3]);
        TX_ACK(ID_PUPG_10);

		Rsbery_Tx_PUPG(PUPG_F_HOST, m_ch.FTPipDomain, strlen(m_ch.FTPipDomain));
		Rsbery_Tx_PUPG(PUPG_F_PORT, m_ch.FTPport, strlen(m_ch.FTPport));
		Rsbery_Tx_PUPG(PUPG_F_PATH, m_ch.road, strlen(m_ch.road));
		Rsbery_Tx_PUPG(PUPG_F_USER, m_ch.FTPid, strlen(m_ch.FTPid));
		Rsbery_Tx_PUPG(PUPG_F_PWD, m_ch.FTPpwd, strlen(m_ch.FTPpwd));
		Rsbery_Tx_PUPG(PUPG_F_FTP_TYPE, &m_ch.FTPtype, 1);
		char ipStr[20] ={0,};
		sprintf(ipStr,"%hhu.%hhu.%hhu.%hhu",m_ch.IP[0], m_ch.IP[1], m_ch.IP[2], m_ch.IP[3]);
		Rsbery_Tx_PUPG(PUPG_F_NEW_IP,ipStr, 15);
		Rsbery_Tx_PUPG(PUPG_F_PRE_TUPG, "tupg all", 9);
		Rsbery_Tx_PUPG(PUPG_F_START, "start", 5);
		Debug_printf(">>10_PUPG End\r\n");
    }
    else m_Gcmd.passingErr = 1;
}

// [11] PVER - 버전정보 요청
void Rx_Passing_11_PVER()
{
    uint32_t tempData = 0;
    uint32_t ch = 0;
	Debug_printf(" ALL 11_PVER: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 11_PVER Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PVER11)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PVER11, TOTAL_LEN_PVER11, VIEW_ADD_3))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PVER11_CRC))return;

        Debug_printf("[R] OK\r\n");
		Debug_printf(">>11_PVER End\r\n");
        Goto_TxCmd(ID_TVER_11);
        m_Gcmd.soketOpenSkip = 1;
    }
    else m_Gcmd.passingErr = 1;
}

// [12] PSET - GW 시간 변경 요청
void Rx_Passing_12_PSET()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;
	Debug_printf(" ALL 12_PSET: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 12_PSET Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PSET12)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PSET12, TOTAL_LEN_PSET12, VIEW_ADD_3))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PSET12_5_1, LEN_PSET12_5_1_6, MIN_12_PSET_5_1, MAX_12_PSET_5_1, VIEW_ADD_4))return; // 서버시간 변경값 YYMMDD (내가쪼갬)
        if(strtol_n(m_Gcmd.passingBuff, chkBuff+1, IDX_PSET12_5_2, LEN_PSET12_5_2_6, MIN_12_PSET_5_2, MAX_12_PSET_5_2, VIEW_ADD_5))return; // 서버시간 변경값 hhmmss (내가쪼갬)
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PSET12_CRC))return;
        Debug_printf("[R] OK\r\n");

        Debug_printf("[N]sevrDay %u \r\n", Get_YYMMDD());
        Debug_printf("[N]sevrTime %06u \r\n", Get_hhmmss());

        Debug_printf("[R]sevrDay %u \r\n",chkBuff[0]);
        Debug_printf("[R]sevrTime %u \r\n",chkBuff[1]);

		m_time.psetOldTime = Get_YYMMDDhhmm();
		m_time.YY = DAY_YY(chkBuff[0]);
		m_time.MM = DAY_MM(chkBuff[0]);
		m_time.DD = DAY_DD(chkBuff[0]);
		m_time.hour = DAY_hh(chkBuff[1]);
		m_time.min = DAY_mm(chkBuff[1]);
		m_time.sec = DAY_ss(chkBuff[1]);

        TX_ACK(ID_PSET_12);
		Debug_printf(">>12_PSET End\r\n");
    }
    else m_Gcmd.passingErr = 1;
}

// [13] PFCC - 시설코드 변경 요청
void Rx_Passing_13_PFCC()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;
    Debug_printf(" ALL 13_PFCC: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>13_PFCC Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PFCC13)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PFCC13, TOTAL_LEN_PFCC13, VIEW_ADD_3))return;

        if(Check_Faci_Code(m_Gcmd.passingBuff, chkBuff, IDX_PFCC13_5, VIEW_ADD_4))return; // 이전 시설코드
        if(Check_Faci_Code(m_Gcmd.passingBuff, chkBuff+1, IDX_PFCC13_6, VIEW_ADD_5))return; // 변경 시설코드
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PFCC13_CRC))return;
        Debug_printf("[R] OK\r\n");

        uint8_t facIdx, flashIdx;
		facIdx = Fac_Code_Find(chkBuff[0]);
		if(facIdx != 0xff)
		{
			m_ch.item[facIdx].facCode = chkBuff[1];
			Debug_printf("[R]old code %u \r\n",chkBuff[0]);
			Debug_printf("[R]new code %u \r\n",chkBuff[1]);
			flashIdx = FLASH_GET_IDX_FAC(facIdx);
			Flash_Write_Word(flashIdx, chkBuff[1]);
		}

        TX_ACK(ID_PFCC_13);
        Debug_printf(">>13_PFCC End\r\n");

    }
    else m_Gcmd.passingErr = 1;
}

// [14] PAST - 측정범위 변경 요청 (N개 가변항목 구조)
void Rx_Passing_14_PAST()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[20][2] = {0,};
    float chkBuff_F[20][3] = {0,};
    uint32_t ch = 0;
	Debug_printf(" ALL 14_PAST: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf("Rx : 14_PAST Start{\r\n");
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
        Debug_printf("[R] OK\r\n");


        for(int i = 0; i < itemCount; i++)
        {

        		uint8_t facIdx, flashIdx;
				facIdx = Fac_Code_Find(chkBuff[i][0]);
				if(facIdx != 0xff)
				{
					Debug_printf("[R]facAddr %u \r\n",facIdx);
					Debug_printf("[R]facCode %u \r\n",chkBuff[i][0]);

//					m_ch.item[facIdx].facCode = chkBuff[i][0];
//					flashIdx = FLASH_GET_IDX_FAC(facIdx);
//					Flash_Write_Word(flashIdx, chkBuff[i][0]);
//					Debug_printf("[R]facCode %u \r\n",chkBuff[i][0]);

					m_ch.item[facIdx].itemCode = chkBuff[i][1];
					flashIdx = FLASH_GET_IDX_ITEM(facIdx);
					Flash_Write_Word(flashIdx, chkBuff[i][1]);
					Debug_printf("[R]itemCode %u \r\n",chkBuff[i][1]);

					m_ch.item[facIdx].rangeMin = chkBuff_F[i][0];
					flashIdx = FLASH_GET_IDX_MIN(facIdx);
					Flash_Write_Word(flashIdx, (uint32_t)chkBuff_F[i][0]);
					Debug_printf("[R]measureMin %f \r\n",chkBuff_F[i][0]);

					m_ch.item[facIdx].rangeMax = chkBuff_F[i][1];
					flashIdx = FLASH_GET_IDX_MAX(facIdx);
					Flash_Write_Word(flashIdx, (uint32_t)chkBuff_F[i][1]);
					Debug_printf("[R]measureMax %f \r\n",chkBuff_F[i][1]);

					m_ch.item[facIdx].rangeStandard = chkBuff_F[i][2];
					flashIdx = FLASH_GET_IDX_STAND(facIdx);
					Flash_Write_Word(flashIdx, (uint32_t)chkBuff_F[i][2]);
					Debug_printf("[R]measureStandard %f \r\n",chkBuff_F[i][2]);

				}

        }
        TX_ACK(ID_PAST_14);
        Debug_printf(">>14_PAST End\r\n");
    }
    else m_Gcmd.passingErr = 1;
}
// [15] PFCR - 관계정보 조회 요청
void Rx_Passing_15_PFCR()
{
    uint32_t tempData = 0;
    uint32_t ch = 0;
	Debug_printf(" ALL 15_PFCR: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 15_PFCR Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PFCR15)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PFCR15, TOTAL_LEN_PFCR15, VIEW_ADD_3))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PFCR15_CRC))return;
        Debug_printf("[R] OK\r\n");

        Goto_TxCmd(ID_TFCR_15);
        m_Gcmd.soketOpenSkip = 1;
		Debug_printf(">>15_PFCR End\r\n");
    }
    else m_Gcmd.passingErr = 1;
}

//[16] PFRS - 방지시설 정상여부 관계정보 변경 요청 (N개 가변관계 구조)
void Rx_Passing_16_PFRS()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[20][2] = {0,};
    uint32_t ch = 0;
	Debug_printf(" ALL 16_PFRS: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 16_PFRS Start{\r\n");
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
        Debug_printf("[R] OK\r\n");

		uint8_t facC, facIdx, flashIdx, facIdxCp;

		m_ch.protectRelyCnt = relationCount;
		Debug_printf("[R]protectRelyCnt %u \r\n",relationCount);
        for(int i = 0; i < relationCount; i++)
        {
			facC = GET_FAC_C(chkBuff[i][0]);
			if(facC == FACI_CODE_E)
			{
				facIdx = Fac_Code_Find(chkBuff[i][0]);
				facIdxCp = Fac_Code_Find(chkBuff[i][1]);
				if((facIdx != 0xff) && (facIdxCp != 0xff))
				{
					m_ch.item[facIdx].couple = facIdxCp+ (i*10); //i는 관계넘버

					flashIdx = FLASH_GET_IDX_COUPLE(facIdx);
					Flash_Write_Word(flashIdx, m_ch.item[facIdx].couple);

					Debug_printf("[R]dispos %hhu \r\n",facIdx);
					Debug_printf("[R]protect %hhu \r\n",facIdxCp);

				}
				else Debug_printf("[R]No find E or P,F \r\n");
			}
			else Debug_printf("[R]dispos Not E\r\n");

        }
        TX_ACK(ID_PFRS_16);
        Debug_printf(">>16_PFRS End\r\n");
    }
    else m_Gcmd.passingErr = 1;
}
// [17] PRSI - 통신서버IP 변경 요청
void Rx_Passing_17_PRSI()
{

    uint32_t tempData = 0;
    uint8_t tempIpBuff[4] = {0,};
    uint8_t tempBuff[16] = {0,};
    uint8_t rawIpBuff[15] = {0,};
    uint32_t ch = 0;
	Debug_printf(" ALL 17_PRSI: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 17_PRSI Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PRSI17)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PRSI17, TOTAL_LEN_PRSI17, VIEW_ADD_3))return;

        memcpy(tempBuff, m_Gcmd.passingBuff+IDX_PRSI17_5, 16);
        Greenlink_Decrypt(tempBuff, rawIpBuff, LEN_PRSI17_5_16_RAW ,LEN_PRSI17_5_15);

        if(Check_IP_Code(rawIpBuff, tempIpBuff, 0, VIEW_ADD_10))return;

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PRSI17_CRC))return;
         Debug_printf("[R] OK\r\n");


         memcpy(m_ch.IP, tempIpBuff, 4);
         Flash_Write_Word(FLASH_IDX_IP_NEW_0, m_ch.IP[0]);
		 Flash_Write_Word(FLASH_IDX_IP_NEW_1, m_ch.IP[1]);
         Flash_Write_Word(FLASH_IDX_IP_NEW_2, m_ch.IP[2]);
		 Flash_Write_Word(FLASH_IDX_IP_NEW_3, m_ch.IP[3]);
         for(int i =0 ;i < 4;i++)
         {
            Debug_printf("%hhu.\r\n",tempIpBuff[i]);
         }
         TX_ACK(ID_PRSI_17);
		Debug_printf(">>17_PRSI End\r\n");

    }
    else m_Gcmd.passingErr = 1;
}

// [18] PDAT - 자료전송모드 변경 요청
void Rx_Passing_18_PDAT()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[1] = {0,};
    uint32_t ch = 0;
	Debug_printf(" ALL 18_PDAT: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 18_PDAT Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PDAT18)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PDAT18, TOTAL_LEN_PDAT18, VIEW_ADD_3))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PDAT18_5, LEN_PDAT18_5_1, MIN_18_PDAT_5, MAX_18_PDAT_5, VIEW_ADD_4))return; // 전송모드

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PDAT18_CRC))return;
        Debug_printf("[R] OK\r\n");

        m_ch.transferMode = chkBuff[0];
        Flash_Write_Word(FLASH_IDX_TRANSFER_MODE, m_ch.transferMode);
        Debug_printf("[R]transferMode %u \r\n",chkBuff[0]);
        TX_ACK(ID_PDAT_18);
		Debug_printf(">>18_PDAT End\r\n");

    }
    else m_Gcmd.passingErr = 1;

}

// [19] PODT - 유예시간 설정 변경 요청
void Rx_Passing_19_PODT()
{
    uint32_t tempData = 0;
    uint32_t chkBuff[2] = {0,};
    uint32_t ch = 0;
	Debug_printf(" ALL 19_PODT: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 19_PODT Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PODT19)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PODT19, TOTAL_LEN_PODT19, VIEW_ADD_3))return;

        if(strtol_n(m_Gcmd.passingBuff, chkBuff, IDX_PODT19_5, LEN_PODT19_5_3, MIN_19_PODT_5, MAX_19_PODT_5, VIEW_ADD_4))return; // 배출 가동유예
        if(strtol_n(m_Gcmd.passingBuff, chkBuff+1, IDX_PODT19_6, LEN_PODT19_6_3, MIN_19_PODT_6, MAX_19_PODT_6, VIEW_ADD_5))return; // 방지 정지유예

        if(Check_crc16(m_Gcmd.passingBuff, IDX_PODT19_CRC))return;
        Debug_printf("[R] OK\r\n");

        m_ch.disposDelTime = chkBuff[0]/5;
        Debug_printf("[R]disposDelTime %u \r\n",chkBuff[0]);
        m_ch.protectDelTime = chkBuff[1]/5;
        Debug_printf("[R]protectDelTime %u \r\n",chkBuff[1]);
		Flash_Write_Word(FLASH_IDX_DISPOS_DELTIME, m_ch.disposDelTime);
		Flash_Write_Word(FLASH_IDX_PROTECT_DELTEIM, m_ch.protectDelTime);

        TX_ACK(ID_PODT_19);
		Debug_printf(">>19_PODT End\r\n");

    }
    else m_Gcmd.passingErr = 1;
}
void Rx_Passing_20_PCN2()
{
    uint32_t tempData = 0;
    uint32_t ch = 0;
	Debug_printf(" ALL 20_PCN2: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 20_PCN2 Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PCN2_20)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PCN2_20, TOTAL_LEN_PCN2_20, VIEW_ADD_3))return;
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PCN220_CRC))return;
        Debug_printf("> OK\r\n");
		Goto_TxCmd(ID_TCN2_20);
		m_Gcmd.soketOpenSkip = 1;
		Debug_printf(">>20_PCN2 End\r\n");
    }
    else m_Gcmd.passingErr = 1;
}

// [22] PRBT - GW 재기동 요청
void Rx_Passing_22_PRBT()
{
    uint32_t tempData = 0;
    uint32_t ch = 0;
	Debug_printf(" ALL 22_PRBT: %06u %06u{\r\n", Get_YYMMDD(),Get_hhmmss());
    Debug_printf(">>Rx : 22_PRBT Start{\r\n");
    if(m_Gcmd.passingCnt == TOTAL_LEN_PRBT22)
    {
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_2, LEN_COMM_2_7, MIN_COMM_2, MAX_COMM_2, VIEW_ADD_1))return;
        if(strtol_n(m_Gcmd.passingBuff, &ch, IDX_COMM_3, LEN_COMM_3_3, MIN_COMM_3, MAX_COMM_3, VIEW_ADD_2))return;
        if(strtol_n(m_Gcmd.passingBuff, &tempData, IDX_COMM_4, LEN_COMM_4_4, TOTAL_LEN_PRBT22, TOTAL_LEN_PRBT22, VIEW_ADD_3))return;
        if(Check_crc16(m_Gcmd.passingBuff, IDX_PRBT22_CRC))return;
        Debug_printf("> OK\r\n");
        TX_ACK(ID_PRBT_22);

        Flash_Write_Word(FLASH_IDX_REBOOT, 1);
        Debug_printf(">>22_PRBT End\r\n");
    }
    else m_Gcmd.passingErr = 1;
}


void Test_Tx_To_RasPi()
{
//	if(dq)
	if(HAL_GPIO_ReadPin(BUTTON_1_GPIO_Port, BUTTON_1_Pin)==0)
	{
		HAL_Delay(100);
	    Rsbery_Tx_PUPG(PUPG_F_FTP_TYPE ,"1" ,1);  /* FTP 타입 (숫자만)   1 = SFTP              */
	    Rsbery_Tx_PUPG(PUPG_F_HOST     ,"192.168.219.113" ,15);  /* SFTP 서버 주소      예: 192.168.219.113   */
	    Rsbery_Tx_PUPG(PUPG_F_PORT     ,"22", 2);  /* SFTP 포트 (숫자만)  예: 22                */
	    Rsbery_Tx_PUPG(PUPG_F_PATH     ,"update/stm32103_new.hex", 23);  /* 원격 파일 경로      예: update/testLED2.hex */
	    Rsbery_Tx_PUPG(PUPG_F_USER     ,"sftp_test", 9);  /* SFTP ID             예: sftp_test         */
	    Rsbery_Tx_PUPG(PUPG_F_PWD      ,"dlfwjs3535!", 11);  /* SFTP 비밀번호                             */
	    Rsbery_Tx_PUPG(PUPG_F_NEW_IP   ,"192.168.219.115", 15);  /* 새 통신서버 IP                            */
	    Rsbery_Tx_PUPG(PUPG_F_PRE_TUPG ,"TUPG OLD", 8);  /* 사전 제작 TUPG (롤백 실패 시 RPi가 대신 송신) */
	    Rsbery_Tx_PUPG(PUPG_F_START    ,"start", 5);  /* 수집 완료 신호, 값은 반드시 "start"       */
	}


}



void TimeOut_Msg()
{

    Debug_printf("TimeOut ");
    switch (m_Gcmd.ID)
    {
        case ID_TDAH_1 : Debug_printf("TDAH_1 ");	break;
        case ID_TOFH_2 : Debug_printf("TOFH_2 ");	break;
        case ID_TDDH_3 : Debug_printf("TDDH_3 ");	break;
        case ID_TFDH_4 : Debug_printf("TFDH_4 ");	break;
        case ID_TDUH_5 : Debug_printf("TDUH_5 ");	break;
        case ID_TNOH_6 : Debug_printf("TNOH_6 ");	break;
        case ID_TTIM_9 : Debug_printf("TTIM_9 ");	break;
        case ID_TUPG_10: Debug_printf("TUPG_10");	break;
        case ID_TVER_11: Debug_printf("TVER_11");	break;
        case ID_TFCR_15: Debug_printf("TFCR_15");	break;
        case ID_TFCR_16: Debug_printf("TFCR_16");	break;
        case ID_TCN2_20: Debug_printf("TCN2_20");	break;
        case ID_PDUH_5 : Debug_printf("PDUH_5 ");	break;
        case ID_PFST_7 : Debug_printf("PFST_7 ");	break;
        case ID_PSEP_8 : Debug_printf("PSEP_8 ");	break;
        case ID_PTIM_9 : Debug_printf("PTIM_9 ");	break;
        case ID_PUPG_10: Debug_printf("PUPG_10");	break;
        case ID_PVER_11: Debug_printf("PVER_11");	break;
        case ID_PSET_12: Debug_printf("PSET_12");	break;
        case ID_PFCC_13: Debug_printf("PFCC_13");	break;
        case ID_PAST_14: Debug_printf("PAST_14");	break;
        case ID_PFCR_15: Debug_printf("PFCR_15");	break;
        case ID_PFRS_16: Debug_printf("PFRS_16");	break;
        case ID_PRSI_17: Debug_printf("PRSI_17");	break;
        case ID_PDAT_18: Debug_printf("PDAT_18");	break;
        case ID_PODT_19: Debug_printf("PODT_19");	break;
        case ID_PCN2_20: Debug_printf("PCN2_20");	break;
        case ID_PRBT_22: Debug_printf("PRBT_22");	break;
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
					TxAllBuff_ReSend();
					m_Gcmd.stepNoneMsg = STEP1;
					Debug_printf("[ERR]TxMsg ReSend STEP0\r\n");
				break;

				case STEP1://미송신
				 	TimeOut_Msg();
					RX_Fail_NextStep();
					m_Gcmd.ID = 0;
					m_Gcmd.txMsgFlag = 0;
					m_Gcmd.stepNoneMsg = STEP0;
					Debug_printf("[ERR]TxMsg_ReSend STEP1\r\n");
				break;
			}
			m_Gcmd.txMsgTimeStamp = HAL_GetTick();
		}
	}
}

void ACK_ReSend()
{
	uint8_t msg[1] = {MSG_ACK,};
	if(m_Gcmd.txAckFlag)
	{
		if(HAL_GetTick() - m_Gcmd.txAckTimeStamp>30000 )
		{
			switch (m_Gcmd.stepNoneAck)
			{
				case STEP0:
					Tx_Cmd_Instruction(msg, 1);
					Debug_printf("[ERR]ACK ReSend STEP0 \r\n");
					m_Gcmd.stepNoneAck = STEP1;
				break;

				case STEP1://미송신
					m_Gcmd.txAckFlag = 0;
					m_Gcmd.ID = 0;
					m_Gcmd.stepNoneAck = STEP0;
					Debug_printf("[ERR]ACK ReSend STEP1\r\n");
				break;
			}
			m_Gcmd.txAckTimeStamp = HAL_GetTick();
		}
	}
}


void ReSend_Config()
{
	ACK_ReSend();
	TxMsg_ReSend();
}


#define CHK_RX_CMD(CMD) 	strncmp((char*)m_Gcmd.passingBuff, (CMD), 4)
#define CHK_RX_CTRL_CMD(CMD, NUM)   strncmp((char*)m_Gcmd.passingBuff, (CMD), (NUM))

void Rx_Get_Gateway(uint8_t rxData)
{
	m_Gcmd.rxBuff[m_Gcmd.rxCnt++] = rxData;
	m_Gcmd.rxCnt %= 190;
	m_Gcmd.rxTimeStamp = HAL_GetTick();
}

void Gateway_Init()
{
	m_ch.itemNum = 6;
	m_ch.transferMode = TXMODE_ALL_NUM;
	m_ch.workPlaceCode = 12345;
	m_ch.disposDelTime = 6;
	m_ch.protectDelTime = 6;
	m_Gcmd.soketTStatus = SOKET_T_CLOSE_OK;
	m_Gcmd.soketPStatus = SOKET_P_CLOSE;

	m_ch.manuCode = 35;
	memcpy(m_ch.GWmodel, "Gw modelNum_1", 13);
	memcpy(m_ch.fwVer, "FirmWare Ver 1.0", 16);
	Flash_init();

	memcpy(m_ch.cmd5Buff[IDX_CMD5_FIV], CMD_TDUH_FIV, 6);
	memcpy(m_ch.cmd5Buff[IDX_CMD5_HAF], CMD_TDUH_HAF, 6);
	memcpy(m_ch.cmd5Buff[IDX_CMD6], CMD_TNOH, 4);//[6]
	memcpy(m_ch.cmd5Buff[IDX_CMD2_FIV], CMD_TOFH_FIV, 6);//[2]
	memcpy(m_ch.cmd5Buff[IDX_CMD2_HAF], CMD_TOFH_HAF, 6);//[2]
	memcpy(m_ch.cmd5Buff[IDX_CMD3_FIV], CMD_TDDH_FIV, 6);//[3]
	memcpy(m_ch.cmd5Buff[IDX_CMD3_HAF], CMD_TDDH_HAF, 6);//[3]

	switch (m_ch.transferMode)
	{
		case TXMODE_HAF_NUM:
			m_ch.itemMode = HAF_IDX;
		break;

		case TXMODE_FIV_NUM:
		case TXMODE_ALL_NUM:
			m_ch.itemMode = HAF_IDX;
		break;
	}

	uint32_t timeStamp = HAL_GetTick();
	uint8_t done = 0;

#if 1
	Debug_printf(">skip Passing\r\n");
	return;
#endif

	Debug_printf(">init Passing Start\r\n");
	while ((HAL_GetTick()-timeStamp < 5000) && (done  == 0))
	{
		done = Rsbery_REQ_Config();
		Rx_Gateway_Config();
	}

	if(m_Gcmd.timeGet) Debug_printf(">timeGet OK\r\n");
	else Debug_printf(">[ERR]timeGet\r\n");

	if(m_Gcmd.gwIpGet) Debug_printf(">gwIpGet OK\r\n");
	else Debug_printf(">[ERR]gwIpGet\r\n");

	if(m_Gcmd.bootGet) Debug_printf(">bootGet OK\r\n");
	else Debug_printf(">[ERR]bootGet\r\n");

	Debug_printf("> m_Gcmd.txCmd = %hhu\r\n",m_Gcmd.txCmd);

}




void Rx_Gateway_Config()//
{
	if(HAL_GetTick() - m_Gcmd.rxTimeStamp>30 && m_Gcmd.rxTimeStamp)
	{
		memcpy(m_Gcmd.passingBuff, m_Gcmd.rxBuff, m_Gcmd.rxCnt);
		memset(m_Gcmd.rxBuff, 0, sizeof(m_Gcmd.rxBuff));
		m_Gcmd.passingCnt = m_Gcmd.rxCnt;
		m_Gcmd.passingErr = 0;

            if(CHK_RX_CMD(CMD_PDUH) == 0){Rx_Passing_5_PDUH();}
        else if(CHK_RX_CMD(CMD_PFST) == 0){Rx_Passing_7_PFST();}
        else if(CHK_RX_CMD(CMD_PSEP) == 0){Rx_Passing_8_PSEP();}
        else if(CHK_RX_CMD(CMD_PTIM) == 0){Rx_Passing_9_PTIM();}
        else if(CHK_RX_CMD(CMD_PUPG) == 0){Rx_Passing_10_PUPG();}
        else if(CHK_RX_CMD(CMD_PVER) == 0){Rx_Passing_11_PVER();}
        else if(CHK_RX_CMD(CMD_PSET) == 0){Rx_Passing_12_PSET();}
        else if(CHK_RX_CMD(CMD_PFCC) == 0){Rx_Passing_13_PFCC();}
        else if(CHK_RX_CMD(CMD_PAST) == 0){Rx_Passing_14_PAST();}
        else if(CHK_RX_CMD(CMD_PFCR) == 0){Rx_Passing_15_PFCR();}
        else if(CHK_RX_CMD(CMD_PFRS) == 0){Rx_Passing_16_PFRS();}
        else if(CHK_RX_CMD(CMD_PRSI) == 0){Rx_Passing_17_PRSI();}
        else if(CHK_RX_CMD(CMD_PDAT) == 0){Rx_Passing_18_PDAT();}
        else if(CHK_RX_CMD(CMD_PODT) == 0){Rx_Passing_19_PODT();}
        else if(CHK_RX_CMD(CMD_PCN2) == 0){Rx_Passing_20_PCN2();}
        else if(CHK_RX_CMD(CMD_PRBT) == 0){Rx_Passing_22_PRBT();}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_SVR_IP, 9) == 0){Rsbery_SurverIp_Tx();}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_SVR_ERR, 10) == 0){Rsbery_SurverIp_Err_Tx();}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_SVR_OK, 9) == 0){Rsbery_SurverIp_Ok_Tx();}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_TIME_RX, 7) == 0){Rsbery_Time_Passing(m_Gcmd.passingBuff);}
        else if(CHK_RX_CTRL_CMD(CMD_CTRL_GW_IP_RX, 7) == 0){Rsbery_GwIp_Passing(m_Gcmd.passingBuff);}
        else if(CHK_RX_CTRL_CMD(CMD_CTRL_TUPG, 8) == 0){Rsbery_PUPG_Secsece();}
        else if(CHK_RX_CTRL_CMD(CMD_CTRL_ABORT, 8) == 0){Rsbery_PUPG_Fail(PUPG_ABORT);}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_DOWNFAIL, 12) == 0){Rsbery_PUPG_Fail(PUPG_DOWNFAIL);}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_FLASHFALE, 13) == 0){Rsbery_PUPG_Fail(PUPG_FLASHFALE);}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_SOKET_T_OPEN_OK, 10) == 0){Rsbery_T_Open_Ok();}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_SOKET_T_OPEN_ERR, 11) == 0){Rsbery_T_Open_Err();}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_SOKET_T_CLOSE_OK, 10) == 0){Rsbery_T_Close_Ok();}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_SOKET_P_OPEN, 8) == 0){Rsbery_P_Open();}
		else if(CHK_RX_CTRL_CMD(CMD_CTRL_SOKET_P_CLOSE, 8) == 0){Rsbery_P_Close();}
        else if(m_Gcmd.passingBuff[0] == MSG_ACK && m_Gcmd.passingCnt == 1)Rx_Passing_ACK();
        else if(m_Gcmd.passingBuff[0] == MSG_NAK && m_Gcmd.passingCnt == 1)Rx_Passing_NAK();
        else if(m_Gcmd.passingBuff[0] == MSG_EOT && m_Gcmd.passingCnt == 1)Rx_Passing_EOT();
		else
		{
			Debug_printf("[ERR] NotFind Cmd{\r\n");
			Debug_Buff_Len(m_Gcmd.passingBuff, m_Gcmd.passingCnt);
		}
		if(m_Gcmd.passingErr)
		{
			Debug_printf("[ERR]RxCnt %hhu\r\n", m_Gcmd.passingCnt);
			Debug_Buff_Len(m_Gcmd.passingBuff, m_Gcmd.passingCnt);
			TX_NAK();
		}

		Debug_printf("}\r\n");
		if(m_Gcmd.txCmdEnd)
		{
			m_Gcmd.txCmdEnd = 0;
			Debug_printf("}\r\n");
		}
		memset(m_Gcmd.passingBuff, 0, sizeof(m_Gcmd.passingBuff));
		m_Gcmd.passingCnt = 0;
		m_Gcmd.rxTimeStamp = 0;
		m_Gcmd.rxCnt = 0;
	}
}


void Tx_Gateway_Config()
{
	static uint8_t openErrCnt = 0;
	static uint8_t step = STEP0;
	static uint32_t soketOpenTime;
	static uint32_t soketRetryTime;
	uint32_t soketOpenTimeOut = 5000;
	uint8_t allow = 0;
	if(m_Gcmd.txCmd==0) return;

	switch (step)
	{
		case STEP0:

			if(m_Gcmd.soketPStatus == SOKET_P_CLOSE ||m_Gcmd.soketOpenSkip) allow++;
			if(m_Gcmd.soketTStatus == SOKET_T_CLOSE_OK ||HAL_GetTick() - m_Gcmd.closeTime > 1000) allow++;
			if(allow==2)step = STEP1;
		break;

		case STEP1:
			if(m_Gcmd.txCmd)
			{
				if(!m_Gcmd.soketOpenSkip)
				{
					Rsbery_Tx_CMD(CMD_CTRL_SOKET_T_OPEN);
					soketOpenTime = HAL_GetTick();
//					m_Gcmd.soketTStatus = SOKET_T_OPEN_CLR;
					m_Gcmd.soketTStatus = SOKET_T_OPEN_OK; //임시테스트

				}
				else
				{
					Debug_printf("[<open>] Skip\r\n");
					m_Gcmd.soketOpenSkip = 0;
					m_Gcmd.soketTStatus = SOKET_T_OPEN_OK;
				}
				step = STEP2;
			}
		break;

		case STEP2:
			if(m_Gcmd.soketTStatus == SOKET_T_OPEN_OK)
			{
				openErrCnt = 0;
				switch (m_Gcmd.txCmd)
				{
					case ID_TDAH_1 :  Tx_1_TDAH(m_ch.itemMode, TX_MODE); break;
					case ID_TOFH_2 :  Tx_2_TOFH(m_ch.itemMode, TX_MODE); break;
					case ID_TDDH_3 :  Tx_3_TDDH(m_ch.itemMode, TX_MODE, Get_Pre_YYYYMMDD());	break;
					case ID_TFDH_4 :  Tx_4_TFDH();					break;
					case ID_TDUH_5 :  Tx_5_TDUH();					break;
					case ID_TNOH_6 :  Tx_6_TNOH();					break;
					case ID_TTIM_9 :  Tx_9_TTIM();					break;
					case ID_TUPG_10:  Tx_10_TUPG();  				break;
					case ID_TVER_11:  Tx_11_TVER();  				break;
					case ID_TFCR_15:  Tx_15_TFCR();					break;
					case ID_TCN2_20:  Tx_21_TCN2();  				break;
					case ID_TOFH_2_LAST : Tx_TOFH2_Last(m_ch.itemMode, TX_MODE); break;
					case ID_TDDH_3_LAST: Tx_3_TDDH_Last(m_ch.itemMode);	break;
				}

				if(!m_ch.cmd5ReTry) m_Gcmd.txCmd = 0;
				m_ch.cmd5ReTry = 0;
				step = STEP0;


			}
			else if(m_Gcmd.soketTStatus == SOKET_T_OPEN_ERR)
			{
				openErrCnt++;
				if(openErrCnt >=4)
				{
					openErrCnt = 0;
					Debug_printf("[T][ERR]Open ReTry 5cnt Over \r\n");
					m_Gcmd.txCmd = 0;
					m_Gcmd.soketTStatus = SOKET_T_CLOSE_OK;
					m_Gcmd.soketOpenSkip = 0;
					m_Gcmd.txUse = 0;
					step = STEP0;
				}
				else
				{

					Debug_printf("[T][ERR]Open ReTry \r\n");
					soketRetryTime = HAL_GetTick();
					step = STEP3;
				}

			}
			else if(HAL_GetTick() - soketOpenTime> soketOpenTimeOut)
			{
				openErrCnt = 0;
				Debug_printf("[T][ERR] Open \r\n");
				m_Gcmd.txCmd = 0;
				m_Gcmd.soketTStatus = SOKET_T_CLOSE_OK;
				m_Gcmd.soketOpenSkip = 0;
				m_Gcmd.txUse = 0;
				step = STEP0;
			}
		break;

		case STEP3:
			if(HAL_GetTick() - soketRetryTime > 1000)
			{
				step = STEP1;
			}
		break;



	}

}

#define mainPoint


void Gateway_Config()//main의 while(1)문에서 호출
{
	Five_Sec_GetData();

	Tx_Start_Config();
	ReSend_Config();
	Rx_Gateway_Config();
	Tx_Gateway_Config();
	Flash_Write_All_Word();
}

void Testfunction()
{
	Debug_Ack_Eot();

	Five_Sec_GetData();
	Tx_Start_Config();

	Rx_Gateway_Config();
	Tx_Gateway_Config();
	Flash_Write_All_Word();
}

void User_Setting_Passing_Pop(int cmd, int data)
{
	uint8_t facCodeAddr;
	char facCodeBuff[5] ={'0','E', 'P', 'F'};
	char itemCodeBuff[8] ={'0', 'A', 'D', 'T', 'H', 'a', 'b',};
	char facC, facCcp ;
	uint32_t facN, facNcp;
	uint8_t itemC;
	uint32_t facCode;
	float maxVal, minVal, standardVal;
	uint8_t couple;
	uint8_t flashIdx;
	switch (cmd)
	{
		case CMD_SET_FAC_CODE:
			facCodeAddr = data/100000;
			 facCode = data%100000;
			 facC = facCode/10000;
			 facN = facCode%10000;

			if(facCodeAddr>ITEM_MAX_ADDR) Debug_printf("Addr over size");
			else if(facCode>FACI_CODE_F) Debug_printf("facCode over size");
			else
			{
				m_ch.item[facCodeAddr].facCode = facCode;
				flashIdx = FLASH_GET_IDX_FAC(facCodeAddr);
				Flash_Write_Word(flashIdx, facCode);
				Debug_printf("facCode : [%hhu] %c%u ",facCodeAddr, facCodeBuff[facC],facN);
			}
			//eeprom
		break;

		case CMD_SET_ITEM_CODE:
			facCodeAddr = data/10;
			 itemC = data%10;
			if(facCodeAddr>ITEM_MAX_ADDR) Debug_printf("Addr over size");
			else if(itemC>ITEM_CODE_h) Debug_printf("itemCode over size");
			else
			{
				m_ch.item[facCodeAddr].itemCode = itemC;
				flashIdx = FLASH_GET_IDX_ITEM(facCodeAddr);
				Flash_Write_Word(flashIdx, itemC);
				Debug_printf("itemCode : [%hhu] %c ",facCodeAddr, itemCodeBuff[itemC]);
			}
			//eeprom
		break;

		case CMD_SET_COUPLE:
			facCodeAddr = data/100;
			couple = data%100;
			if(facCodeAddr <= ITEM_MAX_ADDR)
			{
				facCode = m_ch.item[facCodeAddr].facCode;
				facC = GET_FAC_C(facCode);
			}

			if(facCodeAddr>ITEM_MAX_ADDR) Debug_printf("Addr over size");
			else if(facC !=FACI_CODE_E) Debug_printf("Must Ecode!!");
			else if((couple%10)>ITEM_MAX_ADDR) Debug_printf("Couple over size");
			else
			{
				m_ch.item[facCodeAddr].couple = couple;
				flashIdx = FLASH_GET_IDX_COUPLE(facCodeAddr);
				Flash_Write_Word(flashIdx, couple);
				Debug_printf("%hhu couple is  Num %hhu : %hhu",facCodeAddr, couple/10, couple%10);
			}
		break;

		case CMD_SET_MIN_VAL:
			facCodeAddr = data/100000;
			 minVal = (data%100000)/100.0;
			if(facCodeAddr>ITEM_MAX_ADDR) Debug_printf("Addr over size");
			else
			{
				m_ch.item[facCodeAddr].rangeMin = minVal;

				flashIdx = FLASH_GET_IDX_MIN(facCodeAddr);
				Flash_Write_Word(flashIdx, (uint32_t)minVal);
				Debug_printf("minVal : [%hhu] %.2f ",facCodeAddr, minVal);
			}
		break;

		case CMD_SET_MAX_VAL:
			facCodeAddr = data/100000;
			 maxVal = (data%100000)/100.0;
			if(facCodeAddr>ITEM_MAX_ADDR) Debug_printf("Addr over size");
			else
			{
				m_ch.item[facCodeAddr].rangeMax = maxVal;
				flashIdx = FLASH_GET_IDX_MAX(facCodeAddr);
				Flash_Write_Word(flashIdx, (uint32_t)maxVal);
				Debug_printf("maxVal : [%hhu] %.2f ",facCodeAddr, maxVal);
			}
		break;

		case CMD_SET_STAND_VAL:
			facCodeAddr = data/100000;
			 standardVal = (data%100000)/100.0;
			if(facCodeAddr>ITEM_MAX_ADDR) Debug_printf("Addr over size");
			else
			{
				m_ch.item[facCodeAddr].rangeStandard = standardVal;
				flashIdx = FLASH_GET_IDX_STAND(facCodeAddr);
				Flash_Write_Word(flashIdx, (uint32_t)standardVal);
				Debug_printf("standardVal : [%hhu] %.2f ",facCodeAddr, standardVal);
			}
		break;

		case CMD_READ_ITEM:
			for(int i =0 ;i < m_ch.itemNum;i++)
			{
				 facCode = m_ch.item[i].facCode;
				 facC = GET_FAC_C(facCode);
				 facN = GET_FAC_NUM(facCode);
				 itemC = m_ch.item[i].itemCode;
				 minVal = m_ch.item[i].rangeMin;
				 maxVal = m_ch.item[i].rangeMax;
				 standardVal = m_ch.item[i].rangeStandard;


				if (facC == FACI_CODE_E)
				{
					couple = m_ch.item[i].couple%10;
					facCode = m_ch.item[couple].facCode;
					facCcp = GET_FAC_C(facCode);
					facNcp = GET_FAC_NUM(facCode);
					Debug_printf("[%d] :%c%u, %c, min:%.2f, max:%.2f, std:%.2f couple : [%hhu] :%c%u \r\n",
					i, facCodeBuff[facC], facN, itemCodeBuff[itemC], minVal, maxVal, standardVal,
					couple, facCodeBuff[facCcp], facNcp);
				}
				else
				{
					Debug_printf("[%d] :%c%u, %c, min:%.2f, max:%.2f, std:%.2f \r\n",
					i, facCodeBuff[facC], facN, itemCodeBuff[itemC], minVal, maxVal, standardVal);

				}

			}
		break;

		case CMD_SET_IP_0:
			m_ch.IP[0] = data;
			Debug_printf("Server IP %d.%d.%d.%d \r\n",m_ch.IP[0], m_ch.IP[1], m_ch.IP[2], m_ch.IP[3]);
			Flash_Write_Word(FLASH_IDX_IP_OLD_0, data);
		break;

		case CMD_SET_IP_1:
			m_ch.IP[1] = data;
			Debug_printf("Server IP %d.%d.%d.%d \r\n",m_ch.IP[0], m_ch.IP[1], m_ch.IP[2], m_ch.IP[3]);
			Flash_Write_Word(FLASH_IDX_IP_OLD_1, data);
		break;

		case CMD_SET_IP_2:
			m_ch.IP[2] = data;
			Debug_printf("Server IP %d.%d.%d.%d \r\n",m_ch.IP[0], m_ch.IP[1], m_ch.IP[2], m_ch.IP[3]);
			Flash_Write_Word(FLASH_IDX_IP_OLD_2, data);
		break;

		case CMD_SET_IP_3:
			m_ch.IP[3] = data;
			Debug_printf("Server IP %d.%d.%d.%d \r\n",m_ch.IP[0], m_ch.IP[1], m_ch.IP[2], m_ch.IP[3]);
			Flash_Write_Word(FLASH_IDX_IP_OLD_3, data);
		break;

	}

}






