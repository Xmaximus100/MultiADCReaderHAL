/*
 * adc_mgr.h
 *
 *  Created on: Aug 28, 2025
 *      Author: Celelele
 */

#ifndef INC_ADC_MGR_H_
#define INC_ADC_MGR_H_

#include "string.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"
#include "ringbuffer.h"
#include "at_parser.h"
#include "ltc2368driver.h"

#define MAX_DEVICES 3
#define SPI_AMOUNT 2
#define MAX_EXTI 16
#define REFRENCE_V	2500 	//reference voltage in mV
#define RESOLUTION_BITS	65536

/*
 * ADC_MGR is responsible for handling multiple ltc2368, assigning proper SPI instances
 * and providing configuration and readings via USB CDC
*/

typedef struct {
	volatile uint32_t ready_mask;				//ready mask for quick adc data collection
	LTC2368_SamplingClock clock_handler;		//slave and master clock handlers with sampling frquency defined
	GPIO_Assignment busy_pins[MAX_DEVICES];  	//all exposed busy pins, if empty - NULL
	LTC2368_Handler ltc2368_devs[MAX_DEVICES];	//all connected devices, if empty - NULL
	SPI_Assignment spi_available[SPI_AMOUNT];	//spi available on board
	uint8_t exti_to_dev[MAX_EXTI];				//pin to ID list
	uint8_t ndevs;								//connected devices
	uint16_t ready_to_disp;						//ready to display status for each connected device 1-ready, 0-still collecting
	uint16_t ready_to_disp_mask;				//mask for ready to display check
	uint8_t format; 							//display format 0-raw, 1-readable
	uint32_t refresh_interval; 					//display periods
	uint32_t display_samples; 					//samples to display at once
	uint32_t samples_requested;					//samples requested to collected, common for each device
	AT_CtxT display_func;						//function for displaying data
	bool state; 								//adc on/off
} ADC_Handler;

enum {
	KEEP_BUF=0,
	RESET_BUF
};

bool ADC_Init(ADC_Handler *m, TIM_HandleTypeDef *tim_master, uint32_t tim_master_ch, SPI_HandleTypeDef *spi_handlers[], const GPIO_Assignment busy_pins[], AT_WriteFunc func, void *user);
bool ADC_ManagerInit(ADC_Handler *m, uint8_t dev_amount);
void ADC_Acquire(ADC_Handler *m);
bool ADC_StartSampling(void);
bool ADC_StopSampling(void);
void ADC_MarkReady(ADC_Handler *m, uint8_t id);
uint8_t ADC_PinToIndex(uint16_t GPIO_Pin);
bool ADC_DisplaySamples_Clear(ADC_Handler *m, bool reset_buf, uint32_t custom_samples_requested);
bool ADC_DisplaySamples_Raw(ADC_Handler *m, bool reset_buf, uint32_t custom_samples_requested);
bool ADC_BusyCheck(void);

#endif /* INC_ADC_MGR_H_ */
