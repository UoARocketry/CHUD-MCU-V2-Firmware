#define BMP585_CS_PORT   GPIOA
#define BMP585_CS_PIN    GPIO_PIN_4

#include <math.h>
#include <bmp585.h>


static float s_sea_level_pa = BMP585_SEA_LEVEL_PA_DEFAULT;


static void spi_write_read(uint8_t *tx, uint8_t *rx, uint16_t len)
{
    // Pull CS pin LOW to begin transaction
    HAL_GPIO_WritePin(BMP585_CS_PORT, BMP585_CS_PIN , GPIO_PIN_RESET);
    // transmit/receive
     HAL_SPI_TransmitReceive(&hspi1, tx, rx, len, HAL_MAX_DELAY);
    // CS high
     HAL_GPIO_WritePin(BMP585_CS_PORT, BMP585_CS_PIN , GPIO_PIN_SET);
}

static HAL_StatusTypeDef reg_read(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t tx[len + 1];
    uint8_t rx[len + 1];

    tx[0] = reg | 0x80;   /* R bit set, A6:A0 = register address */
    for (uint8_t i = 1; i < len + 1; i++) tx[i] = 0x00;

    spi_write_read(tx, rx, len + 1);

    /* rx[0] is received during the address byte — ignore it.
       Real data starts at rx[1]. */
    for (uint8_t i = 0; i < len; i++) buf[i] = rx[i + 1];

    return HAL_OK;
}

static HAL_StatusTypeDef reg_write(uint8_t reg, uint8_t value)
{
    uint8_t tx[2] = { reg & 0x7F, value };
    uint8_t rx[2];
    spi_write_read(tx, rx, 2);
    return HAL_OK;
}

static float convert_temperature(uint8_t xlsb, uint8_t lsb, uint8_t msb)
{
    int32_t raw = ((int32_t)msb << 16) | ((int32_t)lsb << 8) | xlsb;
    if (raw & 0x800000) raw |= 0xFF000000;
    return (float)raw / 65536.0f;
}

static float convert_pressure(uint8_t xlsb, uint8_t lsb, uint8_t msb)
{
    uint32_t raw = ((uint32_t)msb << 16) | ((uint32_t)lsb << 8) | xlsb;
    return (float)raw / 64.0f;
}

static float calculate_altitude(float pressure_pa, float sea_level_pa)
{
    return 44330.0f * (1.0f - powf(pressure_pa / sea_level_pa, 0.1902949f));
}

void BMP585_SetSeaLevelPa(float sea_level_pa)
{
    s_sea_level_pa = sea_level_pa;
}

HAL_StatusTypeDef BMP585_Init(void)
{
    uint8_t val;

    /* 0. Force SPI mode: dummy read, ignore the result --------------------- */

	uint8_t dummy;

	reg_read(BMP585_REG_CHIP_ID, &dummy, 1);

    /* 1. Soft reset ------------------------S-------------------------------- */
    reg_write(BMP585_REG_CMD, BMP585_CMD_SOFT_RESET);
    HAL_Delay(5);   /* datasheet: wait ≥ 2 ms after reset */

    /* 2. Verify chip ID ---------------------------------------------------- */
    reg_read(BMP585_REG_CHIP_ID, &val, 1);
    if (val != BMP585_CHIP_ID) return HAL_ERROR;

    /* 3. Wait for NVM ready and confirm POR complete ----------------------- */
    reg_read(BMP585_REG_STATUS, &val, 1);
    if (!(val & BMP585_STATUS_NVM_RDY)) return HAL_ERROR;
    if   (val & BMP585_STATUS_NVM_ERR)  return HAL_ERROR;

    reg_read(BMP585_REG_INT_STATUS, &val, 1);
    if (!(val & BMP585_INT_STATUS_POR)) return HAL_ERROR;

    /* 4. Configure OSR (must be done in STANDBY) --------------------------- */
    reg_write(BMP585_REG_OSR_CONFIG, BMP585_OSR_CONFIG_VAL);

    /* 5. Configure ODR and enter NORMAL mode ------------------------------- */
    reg_write(BMP585_REG_ODR_CONFIG, BMP585_ODR_CONFIG_VAL);

    /* 6. Allow at least one measurement cycle before first read ------------ */
    HAL_Delay(50);

    return HAL_OK;
}

HAL_StatusTypeDef BMP585_ReadData(BMP585_Data_t *out)
{
    if (out == NULL) return HAL_ERROR;

    uint8_t buf[6];

    /*
     * Burst-read 6 bytes starting at TEMP_DATA_XLSB (0x1D):
     *   buf[0] = TEMP_XLSB  (0x1D)
     *   buf[1] = TEMP_LSB   (0x1E)
     *   buf[2] = TEMP_MSB   (0x1F)
     *   buf[3] = PRESS_XLSB (0x20)
     *   buf[4] = PRESS_LSB  (0x21)
     *   buf[5] = PRESS_MSB  (0x22)
     *
     * The BMP585 auto-increments the address on burst reads, and data
     * shadowing ensures all 6 bytes belong to the same measurement snapshot.
     */
    HAL_StatusTypeDef status = reg_read(BMP585_REG_TEMP_DATA_XLSB, buf, 6);
    if (status != HAL_OK) return status;

    out->temperature_c = convert_temperature(buf[0], buf[1], buf[2]);
    out->pressure_pa   = convert_pressure(buf[3], buf[4], buf[5]);
    out->altitude_m    = calculate_altitude(out->pressure_pa, s_sea_level_pa);

    return HAL_OK;
}









