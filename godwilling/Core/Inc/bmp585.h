/**
 * @file    bmp585.h
 * @brief   BMP585 barometric pressure sensor driver for STM32 (SPI, HAL)
 */

#ifndef BMP585_H
#define BMP585_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* --------------------------------------------------------------------------
 * Hardware binding — edit to match your board
 * -------------------------------------------------------------------------- */
extern SPI_HandleTypeDef hspi1;
#define BMP585_SPI_HANDLE   hspi1

#define BMP585_CS_PORT      GPIOA
#define BMP585_CS_PIN       GPIO_PIN_4

/* --------------------------------------------------------------------------
 * Register map
 * -------------------------------------------------------------------------- */
#define BMP585_REG_CHIP_ID          0x01
#define BMP585_REG_REV_ID           0x02
#define BMP585_REG_CHIP_STATUS      0x11
#define BMP585_REG_INT_CONFIG       0x14
#define BMP585_REG_INT_SOURCE       0x15
#define BMP585_REG_TEMP_DATA_XLSB   0x1D  /* 3 consecutive: XLSB, LSB, MSB */
#define BMP585_REG_PRESS_DATA_XLSB  0x20  /* 3 consecutive: XLSB, LSB, MSB */
#define BMP585_REG_INT_STATUS       0x27
#define BMP585_REG_STATUS           0x28
#define BMP585_REG_DSP_CONFIG       0x30
#define BMP585_REG_OSR_CONFIG       0x36
#define BMP585_REG_ODR_CONFIG       0x37
#define BMP585_REG_CMD              0x7E

/* --------------------------------------------------------------------------
 * Register values / bit masks
 * -------------------------------------------------------------------------- */
#define BMP585_CHIP_ID              0x51

/* CMD register */
#define BMP585_CMD_SOFT_RESET       0xB6

/* OSR_CONFIG (0x36)
 *   bits [2:0]  osr_t  — temperature oversampling
 *   bits [5:3]  osr_p  — pressure oversampling
 *   bit  [6]    press_en
 */
#define BMP585_OSR_1X               0x00
#define BMP585_OSR_2X               0x01
#define BMP585_OSR_4X               0x02
#define BMP585_OSR_8X               0x03
#define BMP585_OSR_16X              0x04
#define BMP585_OSR_32X              0x05
#define BMP585_OSR_64X              0x06
#define BMP585_OSR_128X             0x07

/* Build OSR_CONFIG byte:  press_en=1, osr_p=8x, osr_t=1x */
#define BMP585_OSR_CONFIG_VAL   ( (1 << 6)                        \
                                | (BMP585_OSR_8X  << 3)           \
                                | (BMP585_OSR_1X  << 0) )

/* ODR_CONFIG (0x37)
 *   bits [4:0]  odr       — output data rate selection
 *   bits [6:5]  pwr_mode  — 00=STANDBY, 01=NORMAL, 10=FORCED, 11=CONTINOUS
 */
#define BMP585_PWR_STANDBY          0x00
#define BMP585_PWR_NORMAL           0x01

/* ODR 0x15 → 25 Hz (safe for OSR_P x8).  See datasheet Table 9. */
#define BMP585_ODR_25HZ             0x15

/* Build ODR_CONFIG byte: NORMAL mode, 25 Hz */
#define BMP585_ODR_CONFIG_VAL   ( (BMP585_PWR_NORMAL << 5) | BMP585_ODR_25HZ )

/* STATUS register (0x28) */
#define BMP585_STATUS_NVM_RDY       (1 << 1)
#define BMP585_STATUS_NVM_ERR       (1 << 2)

/* INT_STATUS register (0x27) */
#define BMP585_INT_STATUS_POR       (1 << 4)

/* --------------------------------------------------------------------------
 * Physical constants
 * -------------------------------------------------------------------------- */
/* Standard sea-level pressure in Pa — override at runtime via bmp585_set_sea_level_pa() */
#define BMP585_SEA_LEVEL_PA_DEFAULT  101325.0f

/* --------------------------------------------------------------------------
 * Data types
 * -------------------------------------------------------------------------- */

/** Processed sensor output shared with the rest of the system. */
typedef struct {
    float pressure_pa;      /**< Pressure in Pascals                  */
    float temperature_c;    /**< Temperature in degrees Celsius        */
    float altitude_m;       /**< Altitude above sea level in metres    */
} BMP585_Data_t;

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

/**
 * @brief  Initialise the BMP585: soft-reset, verify chip ID, configure OSR/ODR,
 *         then start NORMAL mode.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_StatusTypeDef BMP585_Init(void);

/**
 * @brief  Read pressure and temperature registers, convert to physical units,
 *         and calculate altitude.
 * @param  out  Pointer to data struct to populate.
 * @return HAL_OK on success.
 */
HAL_StatusTypeDef BMP585_ReadData(BMP585_Data_t *out);

/**
 * @brief  Update the sea-level reference pressure used for altitude calculation.
 * @param  sea_level_pa  Sea-level pressure in Pascals (e.g. from ground station).
 */
void BMP585_SetSeaLevelPa(float sea_level_pa);

#ifdef __cplusplus
}
#endif

#endif /* BMP585_H */
