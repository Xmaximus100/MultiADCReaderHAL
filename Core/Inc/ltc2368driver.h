/*
 * ltc2368driver.h
 *
 *  Created on: Aug 23, 2025
 *      Author: Celelele
 */

#ifndef INC_LTC2368DRIVER_H_
#define INC_LTC2368DRIVER_H_

#include "main.h"

#define LTC2368_MAX_MEMORY 1024
#define SYSTEM_FREQ 48000000
#define MAX_SAMPLING_FREQ 500000

typedef enum
{
	LTC2368_OK    	= 0x00U,
	LTC2368_ERROR	= 0x01U,
	LTC2368_BUSY    = 0x02U,
	LTC2368_TIMEOUT = 0x03U,
	LTC2368_DONE 	= 0x04U
} LTC2368_StatusTypeDef;

typedef struct GPIO_Assignment {
	GPIO_TypeDef *port;
	uint16_t pin;
} GPIO_Assignment;

typedef struct {
	SPI_HandleTypeDef *spi_handler;
	uint8_t spi_busy;
} SPI_Assignment;

typedef struct LTC2368_Handler {
	SPI_Assignment *spi_assinged;
	uint8_t device_id;
	bool busy;
	//GPIO_Assignment *cnv;
	GPIO_Assignment *busy_pin;
	uint16_t buf[LTC2368_MAX_MEMORY];
	uint16_t buf_ptr;
	uint16_t samples_requested;
} LTC2368_Handler;

typedef struct LTC2368_SamplingClock {
	GPIO_Assignment cnv_pin;
	uint32_t freq;
	TIM_HandleTypeDef *tim_master;
	uint32_t tim_master_ch;
	TIM_HandleTypeDef *tim_slave;
	uint32_t tim_slave_ch;
} LTC2368_SamplingClock;

LTC2368_StatusTypeDef LTC2368_Read(LTC2368_Handler *ltc2368_dev);
LTC2368_StatusTypeDef LTC2368_Convert(LTC2368_Handler *ltc2368_dev);
LTC2368_StatusTypeDef LTC2368_Unclock(LTC2368_Handler *ltc2368_dev);
LTC2368_StatusTypeDef LTC2368_Lock(LTC2368_Handler *ltc2368_dev);
LTC2368_StatusTypeDef LTC2368_ConfigSampling(LTC2368_SamplingClock *sampling_conf, uint32_t frequency);
LTC2368_StatusTypeDef LTC2368_StartSampling(LTC2368_SamplingClock *sampling_conf);
LTC2368_StatusTypeDef LTC2368_StopSampling(LTC2368_SamplingClock *sampling_conf);
LTC2368_StatusTypeDef LTC2368_DisplaySamples(LTC2368_Handler *ltc2368_dev, uint16_t requested_samples, char *text_buf);

#endif /* INC_LTC2368DRIVER_H_ */
