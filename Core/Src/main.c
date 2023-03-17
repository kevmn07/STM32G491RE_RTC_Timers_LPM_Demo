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

/**
  ******************************************************************************
  * @file       : main.c
  * @brief      : RTC, Timers, and LPM demo, Baremetal implementation
  ******************************************************************************
  * @author     : Eugene Kalosha <kevmn07@gmail.com>
  * @version    : 1.0.0
  ******************************************************************************
  * @section License
  * SPDX-License-Identifier: GPL-3.0-or-later
  *
  * Copyright (C) 2022 Eugene Kalosha. All rights reserved.
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 3
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32g4xx_ll_rtc.h"
#include "stm32g4xx_ll_utils.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum  {
	PW_OFF = 0,
	PW_1S,
	PW_1SINV,
	PW_2S,
	PW_2SINV,
	PW_4S,
	PW_4SINV,
	PW_ON,

	PW_MAX
} PW_PRM;

typedef struct {
	bool     run;
	uint32_t polarity;
	uint16_t cmp;
	uint16_t arl;
} TIMER_PRM;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define SLEEP_SPAN 				9U 	// 10 sec, max = 19 sec because of IWDT

#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
  #define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
  #define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SEND_GOTOSLEEP()		do {\
									printf("NUCLEO-G491RE board enters Sleep mode\r\n");\
								} while(0)

#define SEND_WAKEUP()			do {\
    								printf("NUCLEO-G491RE board came out of Sleep mode\r\n");\
								} while (0)

#define SEND_SLEEP_ERR()		do {\
									printf("\r\nWrong command\r\n");\
									printf("Type \"S\" or \"s\" to Sleep\r\n");\
								} while (0)

#define SEND_INVIT()			do {printf("\r\n*\t*\t*\r\n");\
							    	printf("NUCLEO-G491RE board launched, ID: ");\
							    	printf("%#010lx%08lx%08lx\r\n",\
							    			LL_GetUID_Word0(),\
							    			LL_GetUID_Word1(),\
							    			LL_GetUID_Word2());\
								} while (0)

#define SEND_NOTE()				do {\
									printf("\r\nStart Main Loop\r\n");\
									printf("Type \"S\" or \"s\" to Sleep\r\n");\
								} while (0)

#define GET_D_T(pt, pd)			do {\
									HAL_RTC_GetTime(&hrtc, (pt), RTC_FORMAT_BIN);\
									HAL_RTC_GetDate(&hrtc, (pd), RTC_FORMAT_BIN);\
								} while (0)


#define SEND_D_T(pt, pd)			do {\
									printf("\r\nBoard Date and Time: %02x\\%02d\\%02d %02d:%02d:%02d\r\n",\
										(pd)->Month, (pd)->Date, (pd)->Year,\
										(pt)->Hours, (pt)->Minutes, (pt)->Seconds);\
								} while (0)

#define SEND_TIME(pt)			do {\
									printf("\r\nTime: %02d:%02d:%02d.%03lu\r\n",\
										(pt)->Hours, (pt)->Minutes, (pt)->Seconds,\
										(((pt)->SecondFraction - (pt)->SubSeconds) * 1000)/\
										((pt)->SecondFraction + 1));\
								} while (0)

#define RECEIVE_D_T(pt, pd)	do {\
									printf("\r\nSet Month in MM format: ");\
									scanf("%2hhx", &((pd)->Month));\
									printf("\r\nSet Day in DD format: ");\
									scanf("%2hhu", &((pd)->Date));\
									printf("\r\nSet Year in YY format: ");\
									scanf("%2hhu", &((pd)->Year));\
									printf("\r\nSet Hour in hh format: ");\
									scanf("%2hhx", &((pt)->Hours));\
									printf("\r\nSet Minute in mm format: ");\
									scanf("%2hhu", &((pt)->Minutes));\
									printf("\r\nSet Seconds in ss format: ");\
									scanf("%2hhu", &((pt)->Seconds));\
								} while (0)

#define SET_D_T(pt, pd)			do {\
									HAL_RTC_SetTime(&hrtc, (pt), RTC_FORMAT_BIN);\
									HAL_RTC_SetDate(&hrtc, (pd), RTC_FORMAT_BIN);\
								} while (0)

#define SEND_TMR_PRM(wf)		do {\
									switch (wf) {\
										case PW_OFF:\
											printf("Timer: Always \"OFF\"\r\n");\
											break;\
										case PW_1S:\
											printf("Timer: D=1/3, T=1 sec\r\n");\
											break;\
										case PW_1SINV:\
											printf("Timer: D=2/3, T=1 sec\r\n");\
											break;\
										case PW_2S:\
											printf("Timer: D=1/4, T=2 sec\r\n");\
											break;\
										case PW_2SINV:\
											printf("Timer: D=3/4, T=2 sec\r\n");\
											break;\
										case PW_4S:\
											printf("Timer: D=1/5, T=4 sec\r\n");\
											break;\
										case PW_4SINV:\
											printf("Timer: D=4/5, T=4 sec\r\n");\
											break;\
										case PW_ON:\
											printf("Timer: Always \"ON\"\r\n");\
											break;\
										default:\
											Error_Handler();\
											break;\
									}\
								} while (0)

#define SEND_TMR_OUTPUT(state)	do {\
									printf("Timer: Output \"%s\"\r\n",\
											(state) == GPIO_PIN_SET ? "ON" : "OFF");\
								} while (0)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

LPTIM_HandleTypeDef hlptim1;

UART_HandleTypeDef hlpuart1;

RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */
const static TIMER_PRM lptmr_prm[PW_MAX] = {
		{.run = false, .polarity = LPTIM_OUTPUTPOLARITY_HIGH, .cmp = 0,    .arl = 0},
		{.run = true,  .polarity = LPTIM_OUTPUTPOLARITY_LOW,  .cmp = 341,  .arl = 1023},
		{.run = true,  .polarity = LPTIM_OUTPUTPOLARITY_HIGH, .cmp = 341,  .arl = 1023},
		{.run = true,  .polarity = LPTIM_OUTPUTPOLARITY_LOW,  .cmp = 512,  .arl = 2047},
		{.run = true,  .polarity = LPTIM_OUTPUTPOLARITY_HIGH, .cmp = 512,  .arl = 2047},
		{.run = true,  .polarity = LPTIM_OUTPUTPOLARITY_LOW,  .cmp = 819,  .arl = 4095},
		{.run = true,  .polarity = LPTIM_OUTPUTPOLARITY_HIGH, .cmp = 819,  .arl = 4095},
		{.run = false, .polarity = LPTIM_OUTPUTPOLARITY_LOW,  .cmp = 0,    .arl = 0}
};

static bool allow_wdt_init = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_IWDG_Init(void);
static void MX_LPTIM1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

GETCHAR_PROTOTYPE
{
  uint8_t ch = 0;

  /* Clear the Overrun flag just before receiving the first character */
  __HAL_UART_CLEAR_OREFLAG(&hlpuart1);

  /* Wait for reception of a character on the USART RX line and echo this
   * character on console */
  HAL_UART_Receive(&hlpuart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

static void update_pw_tmr(PW_PRM prm);

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
	RTC_TimeTypeDef brdtime;
	RTC_DateTypeDef brddate;
	GPIO_PinState blue_btn = GPIO_PIN_RESET;
	GPIO_PinState t_btn = GPIO_PIN_RESET;

#ifdef DEBUG_TMR
	GPIO_PinState tmr_out;
	GPIO_PinState t_out;
	int t_msg = 0;
#endif /* DEBUG_TMR */

	PW_PRM tmr_pw_prm = PW_OFF;

	uint8_t tmin;
	uint8_t tch;
	bool sleep = false;

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
  MX_LPUART1_UART_Init();
  MX_IWDG_Init();
  MX_LPTIM1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  setvbuf(stdin, NULL, _IONBF, 0);

  SEND_INVIT();

  RECEIVE_D_T(&brdtime, &brddate);
  SET_D_T(&brdtime, &brddate);
  tmin = brdtime.Minutes;
  update_pw_tmr(tmr_pw_prm);

  GET_D_T(&brdtime, &brddate);
  SEND_D_T(&brdtime, &brddate);
  SEND_TMR_PRM(tmr_pw_prm);

#ifdef DEBUG_TMR
  tmr_out = t_out = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);
  SEND_TMR_OUTPUT(tmr_out);
#endif /* DEBUG_TMR */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  allow_wdt_init = true;
  MX_IWDG_Init();

  SEND_NOTE();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  GET_D_T(&brdtime, &brddate);
	  if (brdtime.Minutes ^ tmin) {
		  SEND_D_T(&brdtime, &brddate);
		  tmin = brdtime.Minutes;
	  }

	  blue_btn =  HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
	  if (t_btn != blue_btn) {
		  t_btn = blue_btn;
		  if (blue_btn == GPIO_PIN_SET) {
			  tmr_pw_prm = (++tmr_pw_prm < PW_MAX) ? tmr_pw_prm : PW_OFF;
			  update_pw_tmr(tmr_pw_prm);
			  SEND_TIME(&brdtime);
			  SEND_TMR_PRM(tmr_pw_prm);

#ifdef DEBUG_TMR
			  t_msg = 0;
#endif /* DEBUG_TMR */
		  }
	  }

#ifdef DEBUG_TMR
	  tmr_out =  HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);
	  if (t_out != tmr_out) {
		  t_out = tmr_out;
		  if (t_msg++ < 10) {
			  SEND_TIME(&brdtime);
			  SEND_TMR_OUTPUT(tmr_out);
		  }
	  }
#endif /* DEBUG_TMR */

	  if (__HAL_UART_GET_FLAG(&hlpuart1, UART_FLAG_RXNE)) {
		  HAL_UART_Receive(&hlpuart1, (uint8_t *)&tch, 1, 0);
		  if (tch == 'S' || tch == 's') { sleep = true; }
		  else { SEND_SLEEP_ERR(); }
	  }

	  if (sleep) {
		  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, SLEEP_SPAN, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK) {
			  Error_Handler();
		  }
		  SEND_D_T(&brdtime, &brddate);
		  SEND_GOTOSLEEP();

  		  HAL_SuspendTick();
  		  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
		  HAL_ResumeTick();
		  GET_D_T(&brdtime, &brddate);
		  SEND_D_T(&brdtime, &brddate);
		  SEND_WAKEUP();

		  sleep = false;
	  }

	  HAL_IWDG_Refresh(&hiwdg);
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */
  if (!allow_wdt_init) { return; }
  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Window = 2500;
  hiwdg.Init.Reload = 2500;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief LPTIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPTIM1_Init(void)
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.SubSeconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_ADD1H;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 10, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  if (HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) != HAL_OK) {
	  Error_Handler();
  }
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief Update parameters of LPTIM1 running  Function
  * @param fl_led - enum, the parameter set number
  * @retval None
  */
static void update_pw_tmr(PW_PRM prm) {

  hlptim1.Init.OutputPolarity = lptmr_prm[prm].polarity;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  if (lptmr_prm[prm].run) {
	  if (HAL_LPTIM_PWM_Start(&hlptim1,
			  	  	  	  	  (uint32_t)lptmr_prm[prm].arl,
							  (uint32_t)lptmr_prm[prm].cmp) != HAL_OK) {
		  Error_Handler();
	  }
  }
  else {
	  if (HAL_LPTIM_PWM_Stop(&hlptim1) != HAL_OK) {
		  Error_Handler();
	  }
  }
}

/**
  * @brief  Wake Up Timer callback.
  * @param  hrtc RTC handle
  * @retval None
  */
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
   if (HAL_RTCEx_DeactivateWakeUpTimer(hrtc) != HAL_OK) {
	  Error_Handler();
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
  printf("\r\nError Handler\r\n");
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
