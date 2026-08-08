/*
 * lora.c
 *
 *  Created on: Aug 8, 2026
 *      Author: crook
 */
#include "ra02.h"

extern SPI_HandleTypeDef hspi3; // defined in main.c

uint8_t Ra02_ReadReg(uint8_t reg_addr)
{
  uint8_t tx_byte = reg_addr & 0x7F; // bit7 = 0 selects read (opposite convention to BMP585)
  uint8_t rx_byte = 0;

  HAL_GPIO_WritePin(LORA_CS_GPIO_Port, LORA_CS_Pin, GPIO_PIN_RESET); // CS low
  HAL_SPI_Transmit(&hspi3, &tx_byte, 1, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi3, &rx_byte, 1, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(LORA_CS_GPIO_Port, LORA_CS_Pin, GPIO_PIN_SET);  // CS high

  return rx_byte;
}

