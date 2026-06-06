/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 그린링크 규격서 요구사항: 실행파일 해시코드 보관용 전역 변수 */
BYTE g_firmware_sha256[SHA256_DIGEST_VALUELEN];

/* IAR 링커에게 현재 실행 파일의 섹션 위치를 알려달라고 선언하는 매크로 */
#pragma section=".text"     // 코드 영역
#pragma section=".rodata"   // 읽기 전용 상수 영역

static SHA256_INFO s_sha_info;

void Calculate_Firmware_Hash(void)
{
    // 내부의 SHA256_INFO sha_info; 선언문을 지우고 전역 변수를 사용합니다.

    uint8_t *text_start   = (uint8_t *)__section_begin(".text");
    uint8_t *text_end     = (uint8_t *)__section_end(".text");
    uint8_t *rodata_start = (uint8_t *)__section_begin(".rodata");
    uint8_t *rodata_end   = (uint8_t *)__section_end(".rodata");

    uint32_t text_size   = (uint32_t)text_end - (uint32_t)text_start;
    uint32_t rodata_size = (uint32_t)rodata_end - (uint32_t)rodata_start;

    // 구조체 변수명만 s_sha_info로 매칭
    SHA256_Init(&s_sha_info);

    if (text_size > 0)
    {
        SHA256_Process(&s_sha_info, (const BYTE *)text_start, text_size);
    }

    if (rodata_size > 0)
    {
        SHA256_Process(&s_sha_info, (const BYTE *)rodata_start, rodata_size);
    }

    SHA256_Close(&s_sha_info, g_firmware_sha256);
}

#if 0
void Calculate_Firmware_Hash(void)
{
    SHA256_INFO sha_info;

    // IAR 세그먼트 시작/끝 주소 추출
    uint8_t *text_start   = (uint8_t *)__section_begin(".text");
    uint8_t *text_end     = (uint8_t *)__section_end(".text");
    uint8_t *rodata_start = (uint8_t *)__section_begin(".rodata");
    uint8_t *rodata_end   = (uint8_t *)__section_end(".rodata");

    // 각 섹션의 실제 크기 계산
    uint32_t text_size   = (uint32_t)text_end - (uint32_t)text_start;
    uint32_t rodata_size = (uint32_t)rodata_end - (uint32_t)rodata_start;

    // 1. SHA-256 구조체 초기화 (KISA API)
    SHA256_Init(&sha_info);

    // 2. 실행파일 코드 영역(.text) 누적 연산
    if (text_size > 0)
    {
        SHA256_Process(&sha_info, (const BYTE *)text_start, text_size);
    }

    // 3. 실행파일 상수 영역(.rodata) 누적 연산
    if (rodata_size > 0)
    {
        SHA256_Process(&sha_info, (const BYTE *)rodata_start, rodata_size);
    }

    // 4. 최종 해시값 마감 및 전역 변수에 저장 (KISA API)
    SHA256_Close(&sha_info, g_firmware_sha256);
}
#endif





/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  /* [추가] 부팅 직후 최초 1회 메인 실행 파일 영역의 해시코드 실시간 연산 */
  Calculate_Firmware_Hash();

  /* 이제 g_firmware_sha256 전역 변수 배열에 고유 해시 32바이트가 장착되었습니다. */
  /* 이 값은 프로그램 코드를 단 1글자라도 수정하고 다시 빌드하면 알아서 변합니다. */

  Uart_Init();
  Test_Config();//

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Testfunction();
//    TxTest();//~~~~~~
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
