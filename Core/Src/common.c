//common.c
#include "common.h"
BUTTON_T m_button1;
BUTTON_T m_button2;
BUTTON_T m_button3;
BUTTON_T m_button4;



uint8_t shotk,longk;
uint16_t pushTime;


void Button_Inint()
{
#if 0
	m_button1.GPIOx = SW1_GPIO_Port;
	m_button1.PortPins = SW1_Pin;

	m_button2.GPIOx = SW2_GPIO_Port;
	m_button2.PortPins = SW2_Pin;

	m_button3.GPIOx = SW3_GPIO_Port;
	m_button3.PortPins = SW3_Pin;

	m_button4.GPIOx = SW4_GPIO_Port;
	m_button4.PortPins = SW4_Pin;

#else
	m_button1.myKey = KEY_READ_UP;
	m_button2.myKey = KEY_READ_DN;
	m_button3.myKey = KEY_READ_POWER;
	m_button4.myKey = KEY_READ_LAMP;

#endif



	m_button1.initOk = 1;
	m_button2.initOk = 1;
	m_button3.initOk = 1;
	m_button4.initOk = 1;

}
uint8_t Is_Button_Push(BUTTON_T* button, uint8_t status)
{
	if(button->mode == status)
	{
		button->mode = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t touchRead;
void Button_config(BUTTON_T* button)
{
	uint8_t readKey = RELEASE;
	if(button->initOk != 1) return;

	if(HAL_GetTick()-button->timeStamp > 40) // 채터링 방지
	{
		readKey = (touchRead == button->myKey);
		if(button->status == RELEASE && readKey == RELEASE) // 안누름
		{
			button->status = RELEASE;
			button->cnt = 0;
			button->mode = NONE_MODE;
		}
		else if(button->status == RELEASE && readKey == PUSH) // 눌름이 감지
		{
			button->cnt = 0;
			button->status = PUSH;
		}
		else if(button->status == PUSH && readKey == PUSH) // 누르는중
		{
			button->cnt++;
			if(button->cnt>LONG_TIME)
			{
				button->cnt = 0;
				button->mode = LONGKEY_MODE;
				button->longCnt++;
			}
		}
		else if(button->status == PUSH && readKey == RELEASE) //뗌
		{
			button->status = RELEASE;
			if(button->mode != LONGKEY_MODE)
			{
				if(MINSHOT_TIME <= button->cnt && button->cnt <MAXSHOT_TIME)
				{
					button->mode = SHOTKEY_MODE;
					button->shotCnt++;
				}
			}
		}

		button->timeStamp = HAL_GetTick();
	}

}

uint8_t Read_Key()
{

	if(Is_Button_Push(&m_button1,SHOTKEY_MODE))
	{
		return KEY_SHOT1;
	}
	else if(Is_Button_Push(&m_button2,SHOTKEY_MODE))
	{
		return KEY_SHOT2;
	}
	else if(Is_Button_Push(&m_button3,SHOTKEY_MODE))
	{
		return KEY_SHOT3;
	}
	else if(Is_Button_Push(&m_button4,SHOTKEY_MODE))
	{
		return KEY_SHOT4;
	}
	else if(Is_Button_Push(&m_button1,LONGKEY_MODE))
	{
		return KEY_LONG1;
	}
	else if(Is_Button_Push(&m_button2,LONGKEY_MODE))
	{
		return KEY_LONG2;
	}
	else if(Is_Button_Push(&m_button3,LONGKEY_MODE))
	{
		return KEY_LONG3;
	}
	else if(Is_Button_Push(&m_button4,LONGKEY_MODE))
	{
		return KEY_LONG4;
	}

	return KEY_NONE;

}

//버퍼클리어 함수
void ClearIntBuff(int* buff, int*cnt, int size)
{
	memset(buff, 0, size*sizeof(int));
	*cnt = 0;
}

void ClearCharBuff(char* buff, int*cnt, int size)
{
	memset(buff, 0, size);
	*cnt = 0;
}

#define COM_I_BUFF_SIZE	20
int comIbuff1[COM_I_BUFF_SIZE];
int comIcnt1;
void ComIBuff1(int data)
{
	static int buffPre = 0xaaa;
	#if 0
	if(data == buffPre)return; // 중복체크
	#endif

	comIbuff1[comIcnt1] = data;
	#if 0
	if(comIcnt1<DEBUG_IBUFF1_SIZE-1)comIcnt1++;
	#else
	comIcnt1++;
	comIcnt1 %= COM_I_BUFF_SIZE;
	#endif
	buffPre = data;
}



void ComBuff_Insert(int* buff, int* cnt, int size,int data)
{
	buff[*cnt] = data;
	(*cnt)++;
	(*cnt) %= size;
}

//====

//====




int compare_32(const void *a, const void *b)    // 오름차순 비교 함수 (uint32_t 기준)
{
    uint32_t num1 = *(uint32_t *)a;
    uint32_t num2 = *(uint32_t *)b;

    if (num1 < num2)    // a가 b보다 작을 때는
        return -1;      // -1 반환

    if (num1 > num2)    // a가 b보다 클 때는
        return 1;       // 1 반환

    return 0;    // a와 b가 같을 때는 0 반환
}
int compare_8(const void *a, const void *b)    // 오름차순 비교 함수 (uint8_t 기준)
{
    uint8_t num1 = *(uint8_t *)a;
    uint8_t num2 = *(uint8_t *)b;

    if (num1 < num2)    // a가 b보다 작으면
        return -1;      // -1 반환

    if (num1 > num2)    // a가 b보다 크면
        return 1;       // 1 반환

    return 0;           // a와 b가 같으면 0 반환
}

uint8_t rxBuff8[200];
uint32_t rxBuff32[200];
void qsortTest()// 오름차순으로 정리
{
	qsort(rxBuff32, sizeof(rxBuff32) / sizeof(uint32_t), sizeof(uint32_t), compare_32);// u32
//	uint8_t max32 = rxBuff32[200-1];//최대값

    qsort(rxBuff8, sizeof(rxBuff8) / sizeof(uint8_t), sizeof(uint8_t), compare_8);
//    uint8_t max8 = rxBuff8[200 - 1];    // 정렬된 배열의 마지막 요소가 최대값

}


int cDiff1= 0xffff,cDiff2= 0xffff,cDiff3= 0xffff;

int ComDiff1(int data)//if문에 넣어주기
{
	if(cDiff1 != data)
	{
		cDiff1 = data;
		return 1;
	}
	else
	{
		cDiff1 = data;
		return 0;
	}
}
int ComDiff2(int data)//if문에 넣어주기
{
	if(cDiff2 != data)
	{
		cDiff2 = data;
		return 1;
	}
	else
	{
		cDiff2 = data;
		return 0;
	}
}
int ComDiff3(int data)//if문에 넣어주기
{
	if(cDiff3 != data)
	{
		cDiff3 = data;
		return 1;
	}
	else
	{
		cDiff3 = data;
		return 0;
	}
}

void HAL_SYSTICK_Callback(void)
{
    // 여기에 사용자가 원하는 코드를 작성
}

