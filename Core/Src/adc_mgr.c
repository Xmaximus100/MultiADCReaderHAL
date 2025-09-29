/*
 * adc_mgr.c
 *
 *  Created on: Aug 28, 2025
 *      Author: Celelele
 */

#include "adc_mgr.h"


ADC_Handler g_adc;
ADC_Handler *g_adc_mgr = &g_adc;


uint8_t ADC_PinToIndex(uint16_t GPIO_Pin){
	#if defined(__GNUC__)
		return (uint8_t)__builtin_ctz((unsigned)GPIO_Pin);
	#else
		return (uint8_t)(__CLZ(__RBIT(GPIO_Pin))); // CMSIS fallback
	#endif
}

void ADC_MarkReady(ADC_Handler *m, uint8_t id){
	//32-bit save is atomic in Cortex-M
//	__DMB(); //uncomment when static
	m->ready_mask |= (1u << id);
}

static inline int NextSetBit(uint32_t *msk) {
    if (*msk == 0) return -1;
    uint32_t m = *msk;
    // indeks najmłodszego ustawionego bitu (0..31)
	#if defined(__GNUC__)
		int idx = __builtin_ctz(m);
	#else
		int idx = (int)__CLZ(__RBIT(m));     // CMSIS: reverse bits + count leading zeros
	#endif
		*msk &= (m - 1); // trik: kasuje najmłodszy ustawiony bit w O(1)
    return idx;
}

static inline uint32_t ADC_FetchReady(ADC_Handler *m) {
    __disable_irq();
    uint32_t msk = m->ready_mask;  // zrzut
    m->ready_mask = 0;             // czyścimy hurtem
    __enable_irq();
    return msk;
}

bool ADC_Init(ADC_Handler *m, TIM_HandleTypeDef *tim_master, uint32_t tim_master_ch, SPI_HandleTypeDef *spi_handlers[], const GPIO_Assignment busy_pins[], AT_WriteFunc func, void *user){
	memset(m, 0, sizeof(*m));
	m->ready_mask = 0;
	m->clock_handler.tim_master = tim_master;
	m->clock_handler.tim_master_ch = tim_master_ch;
	m->ready_to_disp = 0;
	m->samples_requested = 0xFFFF;
	m->display_func.write = func;
	m->display_func.user = user;
	for (uint8_t i=0;i<MAX_EXTI;i++) m->exti_to_dev[i] = 0xFF; //0xFF indicates empty line
	for (uint8_t i=0;i<SPI_AMOUNT;i++){
		m->spi_available[i].spi_handler = spi_handlers[i];
		m->spi_available[i].spi_busy = 0;
	}
	for (uint8_t i=0;i<MAX_DEVICES;i++) {
	    m->busy_pins[i] = busy_pins[i];
	}
	return true;
}

bool ADC_ManagerInit(ADC_Handler *m, uint8_t dev_amount){
	static uint8_t modulo = 0;
	m->ndevs = dev_amount;
	for(uint8_t i=0;i<dev_amount;i++){
		modulo = i%SPI_AMOUNT;
		switch(modulo){
			case 0: m->ltc2368_devs[i].spi_assinged = &m->spi_available[0]; break;
			case 1: m->ltc2368_devs[i].spi_assinged = &m->spi_available[1]; break;
			case 2: m->ltc2368_devs[i].spi_assinged = &m->spi_available[2]; break;
			case 3: m->ltc2368_devs[i].spi_assinged = &m->spi_available[3]; break;
			case 4: m->ltc2368_devs[i].spi_assinged = &m->spi_available[4]; break;
		}
		m->ltc2368_devs[i].spi_assinged->spi_busy = 0;
		m->ltc2368_devs[i].buf_ptr = 0;
		m->ltc2368_devs[i].samples_requested = &m->samples_requested;
		m->ltc2368_devs[i].device_id = i;
		m->ltc2368_devs[i].busy_pin = &m->busy_pins[i];
		uint8_t exti_pin = ADC_PinToIndex(m->busy_pins[i].pin);
		m->exti_to_dev[exti_pin] = i;
		m->ready_to_disp_mask |= 1<<i;
		m->ready_to_disp &= 0<<i;
		m->ltc2368_devs[i].ready_to_disp = &m->ready_to_disp;
	}
	return true;
}

void ADC_Acquire(ADC_Handler *m){
//	for(uint8_t dev_nr=0; dev_nr<ADC_Handler.ndevs; dev_nr++){
//		if (!ADC_Handler.ltc2368_devs[dev_nr].busy)
//			  LTC2368_Read(&ADC_Handler.ltc2368_devs[dev_nr]);
//	}
	// We skim through ready_mask and operate on every id that has been reported
	uint32_t ready = ADC_FetchReady(m);
	for(int id=NextSetBit(&ready); id>=0; id=NextSetBit(&ready)){
		LTC2368_Handler *dev = &m->ltc2368_devs[id];
		if (dev->spi_assinged->spi_busy){
			// if SPI is busy, leave operation for next time
			ADC_MarkReady(m, id);
			continue;
		}
		dev->spi_assinged->spi_busy = 1;
		// until SPI DMA is not used
		if (LTC2368_Read(dev) == LTC2368_OK){
			dev->spi_assinged->spi_busy = 0;
			continue;
		}
		if ((m->ready_to_disp & m->ready_to_disp_mask) == m->ready_to_disp_mask){
			ADC_DisplaySamples_Clear(m);
		}
	}

}

static int ADC_Calculate_Output(int32_t sample){
	int sample_voltage = (sample*REFRENCE_V)/RESOLUTION_BITS;
	return sample_voltage;  //assuming range is +/-10V and REF is internal 2,5V datasheet p.23
}

//static void ADC_Display(AT_CtxT *ctx, const char *s){
//	if (!ctx || !ctx->write) return;
//	size_t len = strlen(s);
//	ctx->write(s, (uint16_t) len);
//}

bool ADC_DisplaySamples_Clear(ADC_Handler *m){
	char buffer[256] = {0};
	size_t used = 0;
	int v;
	used += snprintf(buffer+used, sizeof(buffer)-used, "Collected samples:%d\n\r", m->samples_requested);
//	m->display_func.write(s, (uint16_t) len);
	for (uint8_t i=0; i<m->ndevs; i++){
		used += snprintf(buffer+used, sizeof(buffer)-used, "CHANNEL%d", i);
		m->ltc2368_devs[i].buf_ptr = 0;
	}
	m->display_func.write(buffer, used);
	used = 0;
	for(uint32_t i=0; i<m->samples_requested; i++){
		for(uint8_t k=0; k<m->ndevs; k++){
			v = ADC_Calculate_Output(m->ltc2368_devs[k].buf[i]);
			used += snprintf(buffer+used, sizeof(buffer)-used, "%d.%dV", v/1000,abs(v%1000));
		}
		m->display_func.write(buffer, used);
		used = 0;
	}
	return true;
}

/* TODO
bool ADC_DisplaySamples_Raw(ADC_Handler *m){
	for (uint8_t i=0; i<m->ndevs; i++){
		used += snprintf(buffer+used, sizeof(buffer-used), "CHANNEL%d", i);
		m->ltc2368_devs[i].buf_ptr = 0;
	}
	for(uint32_t i=0; i<m->samples_requested; i++){
		for(uint8_t k=0; k<m->ndevs; k++){
			v = ADC_Calculate_Output(m->ltc2368_devs[k].buf_ptr[i]);
			used += snprintf(buffer+used, sizeof(buffer-used), "%d.%dV", v/1000,abs(v%1000));
		}
		m->display_func.write(buffer, used);
		used = 0;
	}
	uint8_t frame[11];
	memcpy(frame, received_samples, 3); //first 2 bytes for sample index
	memcpy(frame+3, &(ad7606b_data->data_buf[(ad7606b_data->data_ptr) % ad7606b_data->data_ptr_max]), 8); //bytes order frame[2] low_byte, frame[3] high_byte
//	if ((received_samples-1)%10000 == 0) uint8_t tmp = 0;
	HAL_UART_Transmit(huart, frame, 11, 1000);
}
*/

bool ADC_BusyCheck(void){
	return g_adc.state;
}

bool ADC_StartSampling(void){
	return true;
}

bool ADC_StopSampling(void){
	return true;
}
