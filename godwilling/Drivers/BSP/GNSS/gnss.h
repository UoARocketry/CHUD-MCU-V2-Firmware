/*
 * gnss.h
 *
 *  Created on: Aug 10, 2026
 *      Author: crook
 */

#ifndef GNSS_H
#define GNSS_H

#include "main.h"

#define GNSS_BUFFER_SIZE 128

void GNSS_Init(void);
uint8_t GNSS_Poll(void);       // returns 1 when a new complete sentence is ready
const char* GNSS_GetSentence(void);

#endif /* GNSS_H */
