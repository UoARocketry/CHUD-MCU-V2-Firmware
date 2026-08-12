/*
 * imu.c
 *
 *  Created on: Aug 10, 2026
 *      Author: crook
 */


#include "imu.h"

extern SPI_HandleTypeDef hspi1; // shared bus with BMP585, ADXL314

uint8_t IMU_ReadReg(uint8_t reg_addr)
{
  uint8_t tx_byte = 0x80 | (reg_addr & 0x7F); // bit7 = 1 selects read
  uint8_t rx_byte = 0;

  HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET); // CS low
  HAL_SPI_Transmit(&hspi1, &tx_byte, 1, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi1, &rx_byte, 1, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);  // CS high

  return rx_byte;
}
