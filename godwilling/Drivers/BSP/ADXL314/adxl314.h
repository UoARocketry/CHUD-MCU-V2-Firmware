/*
 * adxl.h
 *
 *  Created on: Aug 8, 2026
 *      Author: crook
 */

#ifndef ADXL314_H
#define ADXL314_H

#include "main.h"

typedef struct
{
  float x_g;
  float y_g;
  float z_g;
} ADXL314_Data_t;

uint8_t ADXL314_ReadReg(uint8_t reg_addr);
void ADXL314_ReadAccel(ADXL314_Data_t* data);
void ADXL314_Init(void);

#endif /* ADXL314_H */
