/* USER CODE BEGIN Header */
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
#include "eth.h"
#include "i2c.h"
#include "rtc.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
RTC_DateTypeDef sDate;
RTC_TimeTypeDef sTime;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_ADDR (0x27 << 1)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char uart_buf[70];
char showTime[30] = {0};
char showDate[30] = {0};
char ampm[2][3] = {"AM", "PM"};

static uint8_t selection = 0; // hour, minute, second select
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void LCD_Init(uint8_t lcd_addr);
void LCD_SendCommand(uint8_t lcd_addr, uint8_t cmd);
void LCD_SendString(uint8_t lcd_addr, char *str);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void get_time(void)
{
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    if (sTime.Hours == 0)
    {
        if (sTime.TimeFormat == RTC_HOURFORMAT_12)
            strcpy(ampm[sTime.TimeFormat >> 6], "AM");
    }
    else if (sTime.Hours == 12)
    {
        if (sTime.TimeFormat == RTC_HOURFORMAT_12)
            strcpy(ampm[sTime.TimeFormat >> 6], "PM");
    }
    else
    {
        if (sTime.TimeFormat == RTC_HOURFORMAT_12)
        {
            if (sTime.Hours < 12)
                strcpy(ampm[sTime.TimeFormat >> 6], "AM");
            else
                strcpy(ampm[sTime.TimeFormat >> 6], "PM");
        }
    }


	if (sTime.Hours == 12 && strcmp(ampm[sTime.TimeFormat >> 6], "AM") == 0)
	{
		sTime.Hours = 0;
	}


	if (selection == 0)
		sprintf((char*)showDate, "%04d-%02d-%02d      ", 2000+sDate.Year, sDate.Month, sDate.Date);
	else if (selection == 1)
		sprintf((char*)showDate, "%04d-%02d-%02d[HOUR]", 2000+sDate.Year, sDate.Month, sDate.Date);
	else if (selection == 2)
	    sprintf((char*)showDate, "%04d-%02d-%02d[MIN] ", 2000+sDate.Year, sDate.Month, sDate.Date);
	else if (selection == 3)
	    sprintf((char*)showDate, "%04d-%02d-%02d[SEC] ", 2000+sDate.Year, sDate.Month, sDate.Date);


	sprintf((char *)showTime, "%s %02d:%02d:%02d", ampm[sTime.TimeFormat >> 6], sTime.Hours, sTime.Minutes, sTime.Seconds);
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
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_RTC_Init();
  MX_I2C1_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init(LCD_ADDR);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  get_time();

	  memset(uart_buf, 0, sizeof(uart_buf));
	  sprintf(uart_buf, "%s\t\r\n%s\t\r\n", showDate, showTime);
	  HAL_UART_Transmit(&huart3, (uint8_t *)uart_buf, strlen(uart_buf), 10000);

	  LCD_SendCommand(LCD_ADDR, 0b10000000);
	  LCD_SendString(LCD_ADDR, showDate);

	  LCD_SendCommand(LCD_ADDR, 0b11000000);
	  LCD_SendString(LCD_ADDR, showTime);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  /* EXTI9_5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  /* EXTI15_10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint32_t before_tick = 0;

	if (GPIO_Pin == GPIO_PIN_1)
	{
		if (HAL_GetTick() - before_tick >= 300)
		{
			before_tick = HAL_GetTick();

			selection++;

			if (selection > 3)
				selection = 0;
		}
	}

	else if (GPIO_Pin == GPIO_PIN_6)
	{
		if (HAL_GetTick() - before_tick >= 300)
		{
			before_tick = HAL_GetTick();

			if (selection == 1) // hour select
			{
				sTime.Hours++;

				if (sTime.Hours == 12)
				{
					if (strcmp(ampm[sTime.TimeFormat >> 6], "AM") == 0)
					{
						strcpy(ampm[sTime.TimeFormat >> 6], "PM");
						sTime.Hours = 12;
					}
					else
					{
						strcpy(ampm[sTime.TimeFormat >> 6], "AM");
						sTime.Hours = 0;
					}
				}
				else if (sTime.Hours > 12)
				{
					sTime.Hours = 1;
				}
			}
			else if (selection == 2) // minutes select
			{
				sTime.Minutes++;

				if (sTime.Minutes > 59)
					sTime.Minutes = 0;
			}
			else if (selection == 3) // second select
			{
				sTime.Seconds++;

				if (sTime.Seconds > 59)
					sTime.Seconds = 0;
			}
			HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		}
	}

	else if (GPIO_Pin == GPIO_PIN_5)
	{
		if (HAL_GetTick() - before_tick >= 300)
		{
			before_tick = HAL_GetTick();

			if (selection == 1)
			{
				if (sTime.Hours == 0)
				{
					if (strcmp(ampm[sTime.TimeFormat >> 6], "AM") == 0)
					{
						strcpy(ampm[sTime.TimeFormat >> 6], "PM");
						sTime.Hours = 11;
					}
					else
					{
						strcpy(ampm[sTime.TimeFormat >> 6], "AM");
						sTime.Hours = 11;
					}
				}
				else if (strcmp(ampm[sTime.TimeFormat >> 6], "PM") == 0 && sTime.Hours == 1)
				{
					sTime.Hours = 12;
				}
				else if (strcmp(ampm[sTime.TimeFormat >> 6], "PM") == 0 && sTime.Hours == 12)
				{
					strcpy(ampm[sTime.TimeFormat >> 6], "AM");
					sTime.Hours = 11;
				}
				else
					sTime.Hours--;
			}
			else if (selection == 2)
			{
				if (sTime.Minutes == 0)
					sTime.Minutes = 59;
				else
					sTime.Minutes--;
			}
			else if (selection == 3)
			{
				if (sTime.Seconds == 0)
					sTime.Seconds = 59;
				else
					sTime.Seconds--;
			}
			HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		}
	}
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
