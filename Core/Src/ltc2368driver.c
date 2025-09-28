/*
 * ltc2368driver.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Celelele
 */

#include "ltc2368driver.h"


LTC2368_StatusTypeDef LTC2368_Read(LTC2368_Handler *ltc2368_dev){
	LTC2368_StatusTypeDef status = HAL_SPI_Receive(ltc2368_dev->spi_assinged->spi_handler, (uint8_t*)&ltc2368_dev->buf[ltc2368_dev->buf_ptr], 1, HAL_MAX_DELAY);
	ltc2368_dev->buf_ptr = (ltc2368_dev->buf_ptr+1)%LTC2368_MAX_MEMORY;
	if (ltc2368_dev->buf_ptr == ltc2368_dev->samples_requested) return LTC2368_DONE;
	return status;
}

LTC2368_StatusTypeDef LTC2368_ConfigSampling(LTC2368_SamplingClock *sampling_conf, uint32_t frequency){

	__HAL_TIM_DISABLE(sampling_conf->tim_master); //

	uint32_t period = 1000;
	uint32_t prescaler = 1;
	while((prescaler*period*frequency)<SYSTEM_FREQ){
		prescaler++;
	}
	while((prescaler*period*frequency)>SYSTEM_FREQ){
		period--;
	}
	sampling_conf->tim_master->Instance->ARR = period-1;

	/* Set the Prescaler value */
	sampling_conf->tim_master->Instance->PSC = prescaler-1;
	/* Generate an update event to reload the Prescaler
	 and the repetition counter (only for advanced timer) value immediately */
	sampling_conf->tim_master->Instance->EGR = TIM_EGR_UG;
	/* Check if the update flag is set after the Update Generation, if so clear the UIF flag */
	if (HAL_IS_BIT_SET(sampling_conf->tim_master->Instance->SR, TIM_FLAG_UPDATE))
	{
	/* Clear the update flag */
		CLEAR_BIT(sampling_conf->tim_master->Instance->SR, TIM_FLAG_UPDATE);
	}

	__HAL_TIM_ENABLE(sampling_conf->tim_master); //
	return LTC2368_OK;
}

LTC2368_StatusTypeDef LTC2368_StartSampling(LTC2368_SamplingClock *sampling_conf){
	return HAL_TIM_OC_Start(sampling_conf->tim_master, sampling_conf->tim_master_ch);
}

LTC2368_StatusTypeDef LTC2368_StopSampling(LTC2368_SamplingClock *sampling_conf){
	return HAL_TIM_OC_Stop(sampling_conf->tim_master, sampling_conf->tim_master_ch);
}



