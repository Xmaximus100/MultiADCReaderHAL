/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ringbuffer.h"
#include "at_parser.h"
#include "ltc2368driver.h"
#include "adc_mgr.h"
#include "adc_cli.h"
#include "usbd_cdc_if.h"
#include "string.h"
#include "stdio.h"
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

uint32_t OC_Modes[] = {
	TIM_OCMODE_ACTIVE, /*!< Set channel to active level on match   */
	TIM_OCMODE_INACTIVE,/*!< Set channel to inactive level on match */
	TIM_OCMODE_TOGGLE,/*!< Toggle                                 */
	TIM_OCMODE_PWM1,/*!< PWM mode 1                             */
	TIM_OCMODE_PWM2,/*!< PWM mode 2                             */
	TIM_OCMODE_FORCED_ACTIVE,/*!< Force active level                     */
	TIM_OCMODE_FORCED_INACTIVE,
	TIM_OCMODE_RETRIGERRABLE_OPM1, /*!< Retrigerrable OPM mode 1               */
	TIM_OCMODE_RETRIGERRABLE_OPM2
};

uint16_t buf[100];
uint16_t buf_ptr = 0;
char msg_buf[50];
char receive_buf[64];
uint8_t button_pressed = 0;
uint8_t timer_irq = 0;
uint8_t busy_irq = 0;
uint8_t sampling_on = 0;
uint8_t iter = 0;
const char *data[] = {
		"TIM_OCMODE_ACTIVE\r\n",
		"TIM_OCMODE_INACTIVE\r\n",
		"TIM_OCMODE_TOGGLE\r\n",
		"TIM_OCMODE_PWM1\r\n",/*!< PWM mode 1                             */
		"TIM_OCMODE_PWM2\r\n",/*!< PWM mode 2                             */
		"TIM_OCMODE_FORCED_ACTIVE\r\n",/*!< Force active level                     */
		"TIM_OCMODE_FORCED_INACTIVE\r\n",
		"TIM_OCMODE_RETRIGERRABLE_OPM1\r\n", /*!< Retrigerrable OPM mode 1               */
		"TIM_OCMODE_RETRIGERRABLE_OPM2\r\n"
};

SPI_HandleTypeDef *spi_available[] = {&hspi1, &hspi2};
//
//LTC2368_Handler ltc2368_1 = {
//		.busy_pin = {LTC_BUSY_GPIO_Port, LTC_BUSY_Pin},
//		.spi_assinged = &hspi1,
//		.device_id = 0,
//		.busy = true,
//		.buf_ptr = 0
//};
extern RingBuffer rb_tx;
extern RingBuffer rb_rx;

extern bool data_collected;
extern ADC_Handler g_adc;
extern ADC_Handler *g_adc_mgr;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Toggle_Sampling(void);
void Change_TIM_Mode(void);
AT_StatusTypeDef USB_Write(const char* buf, size_t len);
void App_ADC_Init(void);
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_USB_DEVICE_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  App_ADC_Init();
//  ADC_Init(&htim4, {&hspi1, })
  // Slave: 1 MHz, ARR=999, CH1 = Retriggerable OPM1/OPM2, CCR1=300
  HAL_TIM_Base_Start(&htim8);
  HAL_TIM_OC_Start(&htim8, TIM_CHANNEL_1);  // uruchamia wyjście kanału

  // Master: 1 MHz, ARR=999, CH1 OC Frozen, CCR1=100, TRGO=OC1REF
  __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_CC1);
//  HAL_TIM_Base_Start(&htim4);
  //HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_1);  // żeby OC1REF działał jako źródło TRGO
//  ADC_ManagerInit(g_adc_mgr, 1);
  char init_txt[25];
  char stop_txt[25];
//  sprintf(init_txt, "AT+ADC:SETUP=100,1\n");
//  RingBuffer_WriteString(&rb_rx, init_txt, strlen(init_txt));
//  memset(init_txt, '\0', 25);
//  sprintf(init_txt, "AT+ADC:START\n");
//  RingBuffer_WriteString(&rb_rx, init_txt, strlen(init_txt));
//  sprintf(stop_txt, "AT+ADC:STOP\n");
//  data_collected = true;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (timer_irq == 1 && data_collected == false){
//		  RingBuffer_WriteString("AT\r\n", 4);
//		  data_collected = true;
//		  timer_irq = 0;
	  }
	  if (data_collected){
		  uint8_t msg;
		  while (RingBuffer_Read(&rb_rx, &msg) == RB_OK){
			  AT_ReadChar(msg);
		  }
		  data_collected = false;
	  }
//	  if (button_pressed == 1){
//		  /* TIMER TESTS */
////		  HAL_Delay(1);
////		  Change_TIM_Mode();
//		  /* CONVERSION TESTS */
////		  HAL_GPIO_WritePin(LTC_CNV_GPIO_Port, LTC_CNV_Pin, GPIO_PIN_SET);
////		  HAL_Delay(1);
////		  HAL_GPIO_WritePin(LTC_CNV_GPIO_Port, LTC_CNV_Pin, GPIO_PIN_RESET);
//		  /* SAMPLING TOGGLE */
////		  Toggle_Sampling();
//		  /* AT TEST */
//		  if (g_adc_mgr->state)
//			  RingBuffer_WriteString(&rb_rx, stop_txt, strlen(stop_txt));
//		  else  RingBuffer_WriteString(&rb_rx, init_txt, strlen(init_txt));
//		  data_collected = true;
//		  button_pressed = 0;
//	  }
//	  if (!ltc2368_1.busy){
//		  LTC2368_Read(&ltc2368_1);
////		  ltc2368_1->busy = true;
////		  sprintf(msg_buf, "Collected: %d\r\n", buf[buf_ptr]);
//		  buf_ptr = (buf_ptr+1)%100;
////		  CDC_Transmit_FS((uint8_t*)msg_buf, strlen(msg_buf));
////		  busy_irq = 0;
//	  }
	  ADC_Acquire(g_adc_mgr);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_TIM8;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
  PeriphClkInit.Tim8ClockSelection = RCC_TIM8CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim == &htim4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
		timer_irq = 1;
		NVIC_ClearPendingIRQ(TIM4_IRQn);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim == &htim4){
		timer_irq = 1;
		NVIC_ClearPendingIRQ(TIM4_IRQn);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t pin){
	if (pin == GPIO_PIN_0){
		button_pressed = 1;
		NVIC_ClearPendingIRQ(EXTI0_IRQn);
	}
//	else if (pin == ltc2368_1.busy_pin.pin){
//		ltc2368_1.busy = false;
//	}
    uint8_t line = ADC_PinToIndex(pin);
    uint8_t id   = g_adc.exti_to_dev[line];
    if (id != 0xFF && id < g_adc.ndevs) {
    	ADC_MarkReady(g_adc_mgr, id);
    }
}

void Toggle_Sampling(void){
	if (sampling_on){
		HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_1);
		sampling_on = 0;
	}
	else {
		HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_1);
		sampling_on = 1;
	}
}

void Change_TIM_Mode(void){

	static uint8_t mode_sel = 0;
	size_t mode_size = sizeof(OC_Modes)/sizeof(OC_Modes[0]);
	HAL_TIM_OC_Stop(&htim8, TIM_CHANNEL_1);
	TIM_OC_InitTypeDef sConfigOC = {0};
//	uint32_t tmp_pulse = htim8->Instance->CCR1;
	sConfigOC.OCMode = OC_Modes[mode_sel];
	mode_sel = (mode_sel + 1)%mode_size;
	sConfigOC.Pulse = 300;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
	Error_Handler();
	}

	CDC_Transmit_FS((uint8_t*)data[iter], strlen(data[iter]));
	iter = (iter+1)%9;

	HAL_TIM_OC_Start(&htim8, TIM_CHANNEL_1);
}

AT_StatusTypeDef USB_Write(const char* buf, size_t len){
    size_t n = RingBuffer_WriteString(&rb_tx, buf, len);
    // opcjonalnie: doraźnie szturchnij pompę (poza SOF)
    USB_TxPumpFromRing(&rb_tx);
    return (n == len) ? AT_OK : AT_ERROR;
}

void App_ADC_Init(void)
{
    SPI_HandleTypeDef *spi_handlers[SPI_AMOUNT] = {
        &hspi1,
        &hspi2
    };

    GPIO_Assignment busy_pins[MAX_DEVICES] = {
        { GPIOB, GPIO_PIN_15 },  // dev 0
        { GPIOB, GPIO_PIN_4 },  // dev 1
        { GPIOB, GPIO_PIN_7 },  // dev 2
    };
    for (uint8_t i = 3; i < MAX_DEVICES; ++i) {
        busy_pins[i].port = NULL;
        busy_pins[i].pin  = 0;
    }

    AT_Init(USB_Write, NULL);
    ADC_CommandInit();

    if (!ADC_Init(&g_adc, &htim4, TIM_CHANNEL_1, spi_handlers, busy_pins, USB_Write, NULL)) {
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
