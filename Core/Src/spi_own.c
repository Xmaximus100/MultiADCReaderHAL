/*
 * spi.c
 *
 *  Created on: Aug 22, 2025
 *      Author: Celelele
 */

#include <spi_own.h>

//  __IO uint32_t CR1;      /*!< SPI Control register 1,                              Address offset: 0x00 */
//  __IO uint32_t CR2;      /*!< SPI Control register 2,                              Address offset: 0x04 */
//  __IO uint32_t SR;       /*!< SPI Status register,                                 Address offset: 0x08 */
//  __IO uint32_t DR;       /*!< SPI data register,                                   Address offset: 0x0C */
//  __IO uint32_t CRCPR;    /*!< SPI CRC polynomial register,                         Address offset: 0x10 */
//  __IO uint32_t RXCRCR;   /*!< SPI Rx CRC register,                                 Address offset: 0x14 */
//  __IO uint32_t TXCRCR;   /*!< SPI Tx CRC register,                                 Address offset: 0x18 */
//  __IO uint32_t I2SCFGR;  /*!< SPI_I2S configuration register,                      Address offset: 0x1C */
//  __IO uint32_t I2SPR;    /*!< SPI_I2S prescaler register,                          Address offset: 0x20 */

void SPI1_Init(SPI_Handler* hspi){

	RCC->APB2ENR |= 1; //enable clock

}

