#include "bmp585.h"

extern SPI_HandleTypeDef hspi1;

uint8_t BMP585_ReadReg(uint8_t reg_addr)
{
  uint8_t tx_byte = (1 << 7) | (reg_addr & ~(1 << 7));
  uint8_t rx_byte = 0;

  HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &tx_byte, 1, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi1, &rx_byte, 1, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_SET);

  return rx_byte;
}

void BMP585_WriteReg(uint8_t reg_addr, uint8_t data)
{
  // Write bit = 0 in bit7 (datasheet 5.5.2 SPI write operation)
  uint8_t tx_byte = reg_addr & ~(1 << 7);

  HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &tx_byte, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_SET);
}

void BMP585_Init(void)
{
  // Dummy read to force BMP585 into SPI mode before real communication
  // (required after power-up or reset per datasheet section 5.1)
  BMP585_ReadReg(0x01); // discard result

  BMP585_WriteReg(0x7E, 0xB6);   // soft reset
  HAL_Delay(3);

  BMP585_ReadReg(0x01);          // re-establish SPI mode after reset reverted it to I2C/I3C

  BMP585_WriteReg(0x36, 0x58);   // OSR_CONFIG
  BMP585_WriteReg(0x37, 0x5D);   // ODR_CONFIG
}

void BMP585_Extract_Data(BMP585_Data_t* data)
{
  // Temperature bytes (must be uint8_t, not int8_t!)
  uint8_t TEMP_XLSB = BMP585_ReadReg(0x1D);
  uint8_t TEMP_LSB  = BMP585_ReadReg(0x1E);
  uint8_t TEMP_MSB  = BMP585_ReadReg(0x1F);

  // Pressure bytes
  uint8_t PRES_XLSB = BMP585_ReadReg(0x20);
  uint8_t PRES_LSB  = BMP585_ReadReg(0x21);
  uint8_t PRES_MSB  = BMP585_ReadReg(0x22);

  // Combine into 24-bit unsigned counts first
  uint32_t temp_u24 = ((uint32_t)TEMP_MSB << 16) | ((uint32_t)TEMP_LSB << 8) | TEMP_XLSB;
  uint32_t pres_u24 = ((uint32_t)PRES_MSB << 16) | ((uint32_t)PRES_LSB << 8) | PRES_XLSB;

  // Temperature is signed 24-bit two's complement -> sign-extend to 32-bit
  int32_t temp_s32 = (temp_u24 & 0x00800000) ? (int32_t)(temp_u24 | 0xFF000000) : (int32_t)temp_u24;

  data->temp_raw = temp_s32;
  data->pres_raw = pres_u24;

  // Apply datasheet scaling: T[C] = raw / 2^16, P[Pa] = raw / 2^6
  data->temperature = (float)temp_s32 / 65536.0f;
  data->pressure     = (float)pres_u24 / 64.0f;
}


