/*
 * sd.c
 *
 *  Created on: Aug 24, 2026
 *      Author: crook
 */
#include "sd.h"
#include "fatfs.h"
#include <string.h>

extern SPI_HandleTypeDef hspi2;

static FIL log_file;
static uint8_t sd_ready = 0;

uint8_t SDLogger_Init(void)
{
  // force slow prescaler BEFORE any SD-card SPI traffic begins
  hspi2.Instance->CR1 &= ~SPI_CR1_BR;
  hspi2.Instance->CR1 |= SPI_BAUDRATEPRESCALER_256;

  FRESULT res = f_mount(&USERFatFS, USERPath, 1);
  if (res != FR_OK) return 0;

  // now safe to switch to full speed for normal data transfer
  hspi2.Instance->CR1 &= ~SPI_CR1_BR;
  hspi2.Instance->CR1 |= SPI_BAUDRATEPRESCALER_4;

  FRESULT open_res = f_open(&log_file, "log.txt", FA_OPEN_APPEND | FA_WRITE);
  if (open_res != FR_OK) return 0;

  sd_ready = 1;
  return 1;
}

uint8_t SDLogger_WriteLine(const char* text)
{
  if (!sd_ready) return 0;

  UINT bytes_written;
  FRESULT res = f_write(&log_file, text, strlen(text), &bytes_written);
  f_sync(&log_file);

  return (res == FR_OK);
}


