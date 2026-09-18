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

  if (res != FR_OK){
	  printf("mounting ERROR, FRESULT=%d\r\n", res);


	  return 0;
  };
  printf("SD: mount OK\r\n");


  // now safe to switch to full speed for normal data transfer
  hspi2.Instance->CR1 &= ~SPI_CR1_BR;
  hspi2.Instance->CR1 |= SPI_BAUDRATEPRESCALER_4;

  FRESULT open_res = f_open(&log_file, "log.txt", FA_OPEN_APPEND | FA_WRITE);
  if (open_res != FR_OK) {
	  printf("file opening ERROR, FRESULT=%d\r\n", open_res);

	  return 0;
  };

  printf("SD: file open OK, current size=%lu bytes\r\n", (unsigned long)f_size(&log_file));

  sd_ready = 1;
  return 1;
}

uint8_t SDLogger_WriteLine(const char* text)
{
  if (!sd_ready) {
//      printf("SD aint ready\r\n");
      return 0;
  }

  UINT bytes_written;
  FRESULT res = f_write(&log_file, text, strlen(text), &bytes_written);
  FRESULT sync_res = f_sync(&log_file);   // <-- capture it

  printf("SD: write res=%d wrote=%u/%u bytes, sync res=%d, file size now=%lu\r\n",
         res, bytes_written, (unsigned)strlen(text), sync_res,
         (unsigned long)f_size(&log_file));

  return (res == FR_OK);
}


