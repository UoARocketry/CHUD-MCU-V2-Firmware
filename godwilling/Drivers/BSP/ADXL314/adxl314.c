/*
 * adxl.c
 *
 *  Created on: Aug 8, 2026
 *      Author: crook
 */

#include "adxl314.h"

extern SPI_HandleTypeDef hspi1; // shared bus with BMP585

uint8_t ADXL314_ReadReg(uint8_t reg_addr)
{
  uint8_t tx_byte = 0x80 | (reg_addr & 0x3F); // bit7 = read, bit6 = 0 (single byte), bits5-0 = address
  uint8_t rx_byte = 0;

  HAL_GPIO_WritePin(ADXL_CS_GPIO_Port, ADXL_CS_Pin, GPIO_PIN_RESET); // CS low
  HAL_SPI_Transmit(&hspi1, &tx_byte, 1, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi1, &rx_byte, 1, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(ADXL_CS_GPIO_Port, ADXL_CS_Pin, GPIO_PIN_SET);  // CS high

  return rx_byte;
}
