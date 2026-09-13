#include "bmp585.h"
#include <stdbool.h>
#include <stdio.h>

#define BMP585_STATUS_NVM_OK        0x02  // bits[2:1] = nvm_err=0, nvm_rdy=1
#define BMP585_STATUS_NVM_MASK      0x06
#define BMP585_INT_STATUS_POR_MASK  0x10  // bit 4
#define BMP585_REG_STATUS 0x28
#define BMP585_REG_INT_STATUS 0x27
#define BMP585_PRESS_EN (1 << 6)  // 0x40

extern SPI_HandleTypeDef hspi1;

uint8_t BMP585_ReadReg(uint8_t reg_addr)
{
  // MSB = R/W, following 7 bits are address
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
  // Ensure SPI interface selection (CSB low) before any transaction
  HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_SET);

  uint8_t chip_id = BMP585_ReadReg(0x01);
  if (chip_id != 0x51) {
      printf("BAD CHIP ID: 0x%02X (expected 0x51)\r\n", chip_id);
  }

  BMP585_WriteReg(0x7E, 0xB6);   // soft reset
  HAL_Delay(5);                  // t_soft_res = 2ms, allow margin

  uint8_t status     = BMP585_ReadReg(BMP585_REG_STATUS);
  uint8_t int_status = BMP585_ReadReg(BMP585_REG_INT_STATUS);  // clear-on-read

  if ((status & BMP585_STATUS_NVM_MASK) != BMP585_STATUS_NVM_OK) {
      printf("NVM ERROR: status=0x%02X\r\n", status);
  }
  if ((int_status & BMP585_INT_STATUS_POR_MASK) == 0) {
      printf("POR NOT SET: int_status=0x%02X\r\n", int_status);
  }

  BMP585_WriteReg(0x30, 0x03);   // comp_pt_en = 0b11
  BMP585_WriteReg(0x36, 0x60);   // press_en, osr_p x16, osr_t x1
  BMP585_WriteReg(0x37, 0xBD);   // deep_dis, 50 Hz, NORMAL

  printf("DSP=0x%02X OSR=0x%02X ODR=0x%02X OSR_EFF=0x%02X\r\n",
         BMP585_ReadReg(0x30), BMP585_ReadReg(0x36),
         BMP585_ReadReg(0x37), BMP585_ReadReg(0x38));

  HAL_Delay(50);                 // let the first conversions complete
}

void BMP585_BurstReadData(uint8_t* buf)
{
  uint8_t tx_buf[7] = {0};
  uint8_t rx_buf[7] = {0};

  tx_buf[0] = (1 << 7) | (0x1D & ~(1 << 7)); // address byte, rest are dummy

  HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 7, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_SET);

  for (uint8_t i = 0; i < 6; i++)
  {
      buf[i] = rx_buf[i + 1]; // skip first byte (received during address tx)
  }
}

void BMP585_Extract_Data(BMP585_Data_t* data)
{
  uint8_t raw[6] = {0};

  // raw[0..2] = TEMP_XLSB, TEMP_LSB, TEMP_MSB
  // raw[3..5] = PRES_XLSB, PRES_LSB, PRES_MSB
  BMP585_BurstReadData(raw);

  uint32_t temp_u24 = ((uint32_t)raw[2] << 16) | ((uint32_t)raw[1] << 8) | raw[0];
  uint32_t pres_u24 = ((uint32_t)raw[5] << 16) | ((uint32_t)raw[4] << 8) | raw[3];

  int32_t temp_s32 = (temp_u24 & 0x00800000) ? (int32_t)(temp_u24 | 0xFF000000) : (int32_t)temp_u24;

  data->temp_raw = temp_s32;
  data->pres_raw = pres_u24;

  data->temperature = (float)temp_s32 / 65536.0f;
  data->pressure     = (float)pres_u24 / 64.0f;
}


