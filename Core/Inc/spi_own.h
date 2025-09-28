/*
 * spi.h
 *
 *  Created on: Aug 22, 2025
 *      Author: Celelele
 */

#ifndef INC_SPI_OWN_H_
#define INC_SPI_OWN_H_

#include "main.h"

typedef struct SPI_Handler {
	SPI_TypeDef periph;
	uint64_t prescaler;
	bool master;
	bool clock_polarity; //0 clock low in idle; 1 clock high in idle
	bool clock_phase; //0 clock capture at first edge; 1 clock capture at second edge
	uint8_t data_size;
} SPI_Handler;

void SPI1_Init(SPI_Handler* hspi);
void SPI1_Read(SPI_Handler* hspi, uint16_t* rx_buf, size_t buf_len);
void SPI1_Enable(SPI_Handler* hspi);
void SPI1_Disable(SPI_Handler* hspi);


#endif /* INC_SPI_OWN_H_ */
