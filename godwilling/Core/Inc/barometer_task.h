/**
 * @file    baro_task.h
 * @brief   Public interface for the FreeRTOS barometer task.
 */

#ifndef BARO_TASK_H
#define BARO_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "bmp585.h"

/**
 * @brief  Initialise the BMP585 driver and create the barometer FreeRTOS task.
 *         Call once before vTaskStartScheduler().
 *
 * @param  sea_level_pa  Reference sea-level pressure in Pascals for altitude
 *                       calculation.  Pass BMP585_SEA_LEVEL_PA_DEFAULT
 *                       (101325.0f) if unknown, and update later with
 *                       BMP585_SetSeaLevelPa().
 * @return HAL_OK on success, HAL_ERROR if sensor init or task creation fails.
 */
HAL_StatusTypeDef BaroTask_Init(float sea_level_pa);

/**
 * @brief  Return the queue handle that receives BMP585_Data_t samples.
 *         Other tasks use this to read sensor data.
 *
 * @return FreeRTOS QueueHandle_t, or NULL if BaroTask_Init() hasn't been called.
 */
QueueHandle_t BaroTask_GetQueue(void);

#ifdef __cplusplus
}
#endif

#endif /* BARO_TASK_H */
