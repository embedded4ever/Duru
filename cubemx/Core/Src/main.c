/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "machine.h"
#include "platform_specific.h"


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
bool gsm_reinit_flag;
bool reconnect;
struct command_machine command_machine_t;
extern struct command_window main_all_command_list_t;
int status = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void GSM_TurnON(void)
{
	GSM_pwrkey_off;	 
	GSM_PWR_enable;
	HAL_Delay(100);
	GSM_pwrkey_on; 
	HAL_Delay(100);	 	
	GSM_pwrkey_off; 	
}

static void GSM_Init(void)
{
	GSM_Rst_enable;
	HAL_Delay(200); 
	GSM_PWR_disable;	
	HAL_Delay(2000);
	GSM_PWR_enable;	
	GSM_Rst_disable;  
	GSM_TurnON(); 
}

static bool uart_tx_platform_specific(uint8_t* data, uint16_t size)
{
	HAL_UART_Transmit(&huart5, data, size, 2000);
	
  return true;
}

char get_buffer_for_send[600];

void HAL_UART_RxCpltCallback (UART_HandleTypeDef * huart) 
{
	if (huart == &huart5) 
	{
		HAL_UART_Receive_IT(&huart5 ,(uint8_t *)&gsm_uart.data, 1);

		gsm_uart.buffer[gsm_uart.buffer_index++] = gsm_uart.data;
		
		if (gsm_uart.buffer_index >= 6)
		{
			//if (gsm_uart.buffer[0] == '\r' && gsm_uart.buffer[1] == '\n' && gsm_uart.buffer[gsm_uart.buffer_index - 2] == '\r'
			//	&& gsm_uart.buffer[gsm_uart.buffer_index - 1] == '\n')
			{
				if (strstr(gsm_uart.buffer, "OK\r\n") || strstr(gsm_uart.buffer, "CONNECT\r\n"))
				{   
					//gsm_uart.new_line_flag = true;
				}
			}
		}
		
		if (gsm_uart.buffer_index >= MAX_NUM_OF_BUFFER)
		{
			gsm_uart.buffer_index = 0;
		}
	}
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
  MX_UART5_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_UART_Receive_IT(&huart5, (uint8_t *)&gsm_uart.data, 1);
	
	SMPS_Enable;
	LDO_Enable;
	PER3V3_Enable;
		
	GSM_Init();
	
	init_all_command_window();
	
	command_machine_t.command_index = 0;
	command_machine_t.current_command_window = &main_all_command_list_t;
	command_machine_t.uart_tx_cb = &uart_tx_platform_specific;
	command_machine_t.is_state_transition_available = 0;
	command_machine_t.timer = 0;
	command_machine_t.timer_cnt = 0;
	command_machine_t.command_window_index = 0; 

	machine_start(&command_machine_t);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		machine_loop(&command_machine_t);
		
		if (get_command_window_status(&command_machine_t, "INIT"))
		{
			machine_trans(&command_machine_t, "PUT");
		}
		
		else if (get_command_window_status(&command_machine_t, "PUT"))
		{
			machine_trans(&command_machine_t, "SEND");
		}
		
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
