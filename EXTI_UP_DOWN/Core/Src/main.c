/* USER CODE BEGIN Header */
#include "stm32f4xx_hal.h"
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
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
int counter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void DisplayFND(uint8_t fnd, uint8_t location);
uint8_t ttl7447(uint8_t num);
void counter0to9999(uint8_t num[4], uint32_t tick);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
	HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, 500);
	return len;
}

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 1000);
	return ch;
}
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
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  uint8_t count[4] = {0,};
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  counter0to9999(count, counter);

	  DisplayFND(ttl7447(count[0]), 1);	// thousands
	  DisplayFND(ttl7447(count[1]), 2);	// hundreds
	  DisplayFND(ttl7447(count[2]), 3);	// tens
	  DisplayFND(ttl7447(count[3]), 4);	// ones

	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, 0);
	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 0);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* EXTI15_10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  /* USART3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint32_t before_tick = 0;

	if(GPIO_Pin == GPIO_PIN_13)
	{
		if (HAL_GetTick() - before_tick >= 300)
		{
			before_tick = HAL_GetTick();

			counter++;

			if (counter > 9999)
			{
				counter = 0;
			}

			printf("USART print 7segment number=%d\r\n", counter);
		}
	}
	else if (GPIO_Pin == GPIO_PIN_3)
	{
		if (HAL_GetTick() - before_tick >= 300)
		{
			before_tick = HAL_GetTick();

			counter--;

			if (counter < 0)
			{
				counter = 9999;
			}

			printf("USART print 7segment number=%d\r\n", counter);
		}
	}
}

uint8_t ttl7447(uint8_t num){
	// for anode type
	switch(num){
	case 0:
		return 0xc0;
	case 1:
		return 0xf9;
	case 2:
		return 0xa4;
	case 3:
		return 0xb0;
	case 4:
		return 0x99;
	case 5:
		return 0x92;
	case 6:
		return 0x82;
	case 7:
		return 0xd8;
	case 8:
		return 0x80;
	case 9:
		return 0x90;
	default:
		return 0xff;
	}
}

void DisplayFND(uint8_t fnd, uint8_t location){

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, 0);	// latch clock pin off

	uint16_t data = 0;
	data = fnd << 8;
	data |= 1 << (location-1);


	// for example, to display 1 to first FND
	// MSB 1111 1001 dddd 0001 LSB	(d means don't care)

	for(int i=0 ; i<16; i++){

		if(data >> (15-i) & 1){	// MSB first in
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, 1);	// Data line
		}else {
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, 0);	// Data line
		}

		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 1);	// shift clock pin on
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 0);	// shift clock pin off
	}


	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, 1);	// latch clock pin on
}

void counter0to9999(uint8_t num[4], uint32_t counter){
    num[0] = counter / 1000;
    num[1] = (counter / 100) % 10;
    num[2] = (counter / 10) % 10;
    num[3] = counter % 10;
}
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