/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SEG_A  GPIO_PIN_0
#define SEG_B  GPIO_PIN_1
#define SEG_C  GPIO_PIN_2
#define SEG_D  GPIO_PIN_3
#define SEG_E  GPIO_PIN_4
#define SEG_F  GPIO_PIN_5
#define SEG_G GPIO_PIN_6
#define SEG_DP GPIO_PIN_9
#define ALL_SEGMENTS SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DP

#define DIG_1 GPIO_PIN_2
#define DIG_2 GPIO_PIN_3
#define DIG_3 GPIO_PIN_4
#define DIG_4 GPIO_PIN_5

#define MAX_HOURS 24U
#define MAX_MINUTES 60U
#define MAX_SECONDS 60U
#define MAX_MILISECONDS 1000U

#define MAX_NUMBER_OF_DIGIT 4U
#define RIGHT_DOT_DELAY 200U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
const uint8_t segments[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,//0
  SEG_B | SEG_C,// 1
	SEG_A | SEG_E | SEG_B | SEG_D | SEG_G,// 2
	SEG_A | SEG_B | SEG_G | SEG_C | SEG_D,// 3
	SEG_F | SEG_G | SEG_B | SEG_C,// 4
	SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,// 5
  SEG_A | SEG_F | SEG_G | SEG_E | SEG_C | SEG_D,// 6
	SEG_A | SEG_B | SEG_C,// 7
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,// 8
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G// 9
};
static uint8_t seconds = 0U;
static uint8_t minutes = 0U;
static uint8_t hours = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void clear_segment(const uint16_t DIG_number)
{
  HAL_GPIO_WritePin(GPIOG, ALL_SEGMENTS, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, DIG_number, GPIO_PIN_RESET);
}

void send_digit_to_LED(const uint16_t value, const uint16_t DIG_number, const uint8_t position, bool isDot)
{
  uint8_t digit = 0U;
  if (position % 2 != 0)
  {
    digit = (uint8_t)((float)value / 10.0F);
  }
  else
  {
    digit = (uint8_t)(value % 10);
  }    

  clear_segment(DIG_number);
  if(isDot == true){
    HAL_GPIO_WritePin(GPIOG, segments[digit] | SEG_DP, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOG, segments[digit], GPIO_PIN_SET);
  }
  HAL_GPIO_WritePin(GPIOB, DIG_number, GPIO_PIN_SET);
}

void update_led_display(const uint32_t frequency)
{
  static uint32_t counter = 0U;
  static uint32_t max_counter = 0U;
  static uint32_t currentDigitposition = 1U;
  // htim2.Init.Prescaler = 999;
  // htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  // htim2.Init.Period = 4;
  // 4000 000 / 10000 / 4 = 1000
  max_counter = (uint32_t)((float)HAL_RCC_GetPCLK1Freq()/(float)(htim2.Init.Prescaler + 1)/(float)htim2.Init.Period/frequency);
  counter++;
  if(counter >= max_counter)
  {
     //If button is pressed, show minutes:seconds.
    if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_RESET)
    {
      if (currentDigitposition == 1) 
      {
        send_digit_to_LED(minutes, DIG_1, 1, false);
      }
      else if (currentDigitposition == 2) 
      {
        send_digit_to_LED(minutes, DIG_2, 2, true);
      }
      else if (currentDigitposition == 3)
      {
        send_digit_to_LED(seconds, DIG_3, 3, false);
      }
      else if (currentDigitposition == 4)
      {
        send_digit_to_LED(seconds, DIG_4, 4, false);
      }      
    }
    else
    {
      if (currentDigitposition == 1) 
      {
        send_digit_to_LED(hours, DIG_1, 1, false);
      }
      else if (currentDigitposition == 2)
      {
        send_digit_to_LED(hours, DIG_2, 2, true);
      }
      else if (currentDigitposition == 3)
      {
        send_digit_to_LED(minutes, DIG_3, 3, false); 
      }
      else if (currentDigitposition == 4)
      {
        if (uwTick < RIGHT_DOT_DELAY)
        {
          send_digit_to_LED(minutes, DIG_4, 4, true); 
        }
        else
        {
          send_digit_to_LED(minutes, DIG_4, 4, false); 
        }
      }
    }
    currentDigitposition++;
    if(currentDigitposition > MAX_NUMBER_OF_DIGIT)
    {
    currentDigitposition = 1U;
    }
    counter = 0;
  }
}

void update_time(const float frequency)
{
  static uint32_t counter = 0U;
  static uint32_t max_counter = 0U;
  max_counter = (uint32_t)((float)HAL_RCC_GetPCLK1Freq()/(float)(htim2.Init.Prescaler + 1)/(float)htim2.Init.Period/frequency);
  counter++;
  if (counter >= max_counter)
  {
    if (uwTick >= MAX_MILISECONDS)
    {
      uwTick = 0U;
      seconds++;
    }
    if (seconds >= MAX_SECONDS)
    {
      seconds = 0U;
      minutes++;
    }
    if (minutes >= MAX_MINUTES)
    {
      minutes = 0U;
      hours++;
    }
    if (hours >= MAX_HOURS)
    {
      seconds = 0U;
      minutes = 0U;
      hours = 0U;
    }
    counter = 0;  
  }
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);

  GPIO_InitTypeDef conf = {
  			.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_9,
  			.Mode = GPIO_MODE_OUTPUT_PP,
  			.Pull = GPIO_NOPULL,
  			.Speed = GPIO_SPEED_FREQ_LOW
  };
  HAL_GPIO_Init(GPIOG, &conf);

  conf.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &conf);

  conf.Pin = GPIO_PIN_15;
  conf.Mode = GPIO_MODE_INPUT;
  conf.Pull = GPIO_NOPULL;
  conf.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOE, &conf);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  update_time(1000);
  update_led_display(1000);
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
