/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   SD card SPI-mode low-level disk I/O driver for FatFs.
  *
  *          Implements the SD card SPI protocol (CMD0/CMD8/ACMD41/CMD58 init
  *          sequence, single-block CMD17/CMD24 read/write) on top of a
  *          generic STM32 HAL SPI peripheral (hspi2) with a GPIO chip-select
  *          line (SD_CS_GPIO_Port / SD_CS_Pin).
  *
  *          Auto-detects SDHC/SDXC (block addressing) vs SDSC (byte
  *          addressing) at init time via the OCR CCS bit, so it works with
  *          either card type without recompiling.
  *
  *          Uses SD_CD_GPIO_Port / SD_CD_Pin (pulled up, pulled low when a
  *          card is inserted) to skip the init handshake and report
  *          STA_NODISK when no card is present.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "main.h"   /* for SD_CS_GPIO_Port / SD_CS_Pin and hspi2 extern */

/* USER CODE BEGIN DECL */

extern SPI_HandleTypeDef hspi2;
#define SD_SPI_HANDLE   hspi2

/* Private define -------------------------------------------------------------*/

/* SD SPI commands */
#define CMD0    (0)          /* GO_IDLE_STATE */
#define CMD1    (1)          /* SEND_OP_COND (MMC) */
#define CMD8    (8)          /* SEND_IF_COND */
#define CMD9    (9)          /* SEND_CSD */
#define CMD12   (12)         /* STOP_TRANSMISSION */
#define CMD16   (16)         /* SET_BLOCKLEN */
#define CMD17   (17)         /* READ_SINGLE_BLOCK */
#define CMD18   (18)         /* READ_MULTIPLE_BLOCK */
#define CMD24   (24)         /* WRITE_BLOCK */
#define CMD25   (25)         /* WRITE_MULTIPLE_BLOCK */
#define CMD55   (55)         /* APP_CMD */
#define CMD58   (58)         /* READ_OCR */
#define ACMD41  (41)         /* SD_SEND_OP_COND (SDC) */

/* Card type flags */
#define CT_MMC      0x01
#define CT_SD1      0x02
#define CT_SD2      0x04
#define CT_SDC      (CT_SD1 | CT_SD2)
#define CT_BLOCK    0x08    /* Card uses block (not byte) addressing -- SDHC/SDXC */

#define SD_TIMEOUT_MS   500

/* Private variables ----------------------------------------------------------*/
static volatile DSTATUS Stat = STA_NOINIT;
static BYTE CardType = 0;

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* USER CODE BEGIN PRIVATE_FUNCTIONS */

/*-----------------------------------------------------------------------*/
/* Low-level SPI byte helpers                                            */
/*-----------------------------------------------------------------------*/

static BYTE SPI_RW(BYTE d)
{
  BYTE r = 0xFF;
  HAL_SPI_TransmitReceive(&SD_SPI_HANDLE, &d, &r, 1, 100);
  return r;
}

static void SD_CS_LOW(void)
{
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static void SD_CS_HIGH(void)
{
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

/* Card-detect switch: pulled up, pulled low when a card is inserted (normally-open switch to GND) */
static uint8_t SD_isCardPresent(void)
{
  return (HAL_GPIO_ReadPin(SD_CD_GPIO_Port, SD_CD_Pin) == GPIO_PIN_RESET);
}

/* Deselect card and clock one extra byte (required by SD SPI spec) */
static void SD_Deselect(void)
{
  SD_CS_HIGH();
  SPI_RW(0xFF);
}

/* Wait for card to be ready (0xFF), with timeout. Returns 1 if ready. */
static uint8_t SD_ReadyWait(void)
{
  BYTE res;
  uint32_t start = HAL_GetTick();

  do
  {
    res = SPI_RW(0xFF);
  } while ((res != 0xFF) && ((HAL_GetTick() - start) < SD_TIMEOUT_MS));

  return (res == 0xFF);
}

/* Select card, wait for ready. Returns 1 on success. */
static uint8_t SD_Select(void)
{
  SD_CS_LOW();
  SPI_RW(0xFF); /* extra clock */

  if (SD_ReadyWait())
  {
    return 1;
  }

  SD_Deselect();
  return 0;
}

/*-----------------------------------------------------------------------*/
/* Send a command and return the R1 response                            */
/*-----------------------------------------------------------------------*/
static BYTE SD_SendCmd(BYTE cmd, DWORD arg)
{
  BYTE crc, res;
  uint8_t n;

  /* ACMD<n> is sent as CMD55 followed by CMD<n> */
  if (cmd & 0x80)
  {
    cmd &= 0x7F;
    res = SD_SendCmd(CMD55, 0);
    if (res > 1) return res;
  }

  /* Select card (except for CMD12 STOP_TRANSMISSION and after already selected) */
  if (cmd != CMD12)
  {
    SD_Deselect();
    if (!SD_Select()) return 0xFF;
  }

  /* Command packet: start bit/transmission bit + cmd, 4 arg bytes, CRC */
  BYTE buf[6];
  buf[0] = 0x40 | cmd;
  buf[1] = (BYTE)(arg >> 24);
  buf[2] = (BYTE)(arg >> 16);
  buf[3] = (BYTE)(arg >> 8);
  buf[4] = (BYTE)arg;

  if (cmd == CMD0) crc = 0x95;       /* fixed CRC for CMD0 */
  else if (cmd == CMD8) crc = 0x87;  /* fixed CRC for CMD8 (arg = 0x1AA) */
  else crc = 0x01;                   /* CRC disabled after init; stop bit only */
  buf[5] = crc;

  for (n = 0; n < 6; n++) SPI_RW(buf[n]);

  if (cmd == CMD12) SPI_RW(0xFF); /* skip stuff byte for STOP_TRANSMISSION */

  /* Wait for a valid R1 response (MSB clear), up to 10 tries */
  n = 10;
  do
  {
    res = SPI_RW(0xFF);
  } while ((res & 0x80) && --n);

  return res;
}

/*-----------------------------------------------------------------------*/
/* Receive a data block (512 bytes + discard 2 CRC bytes)                */
/*-----------------------------------------------------------------------*/
static uint8_t SD_RxDataBlock(BYTE *buff, UINT len)
{
  BYTE token;
  uint32_t start = HAL_GetTick();

  /* Wait for the data start token (0xFE) */
  do
  {
    token = SPI_RW(0xFF);
  } while ((token == 0xFF) && ((HAL_GetTick() - start) < SD_TIMEOUT_MS));

  if (token != 0xFE) return 0; /* timeout or error token */

  /* Fill dummy TX with 0xFF and clock in the data payload */
  {
    BYTE txdummy[512];
    memset(txdummy, 0xFF, len);
    HAL_SPI_TransmitReceive(&SD_SPI_HANDLE, txdummy, buff, len, 200);
  }

  /* Discard CRC (2 bytes) */
  SPI_RW(0xFF);
  SPI_RW(0xFF);

  return 1;
}

/*-----------------------------------------------------------------------*/
/* Transmit a data block                                                 */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE == 1
static uint8_t SD_TxDataBlock(const BYTE *buff, BYTE token)
{
  BYTE resp;
  BYTE rxdummy[512];

  if (!SD_ReadyWait()) return 0;

  SPI_RW(token);

  if (token != 0xFD) /* not a stop-transmission token -> send payload */
  {
    HAL_SPI_TransmitReceive(&SD_SPI_HANDLE, (BYTE *)buff, rxdummy, 512, 200);

    SPI_RW(0xFF); /* dummy CRC */
    SPI_RW(0xFF);

    resp = SPI_RW(0xFF);
    if ((resp & 0x1F) != 0x05) return 0; /* data rejected */

    if (!SD_ReadyWait()) return 0; /* wait for card to finish programming */
  }

  return 1;
}
#endif /* _USE_WRITE == 1 */

/* USER CODE END PRIVATE_FUNCTIONS */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
  uint8_t n, ty, ocr[4];
  uint32_t start;

  if (pdrv != 0) return STA_NOINIT;

  if (!SD_isCardPresent())
  {
    Stat = STA_NOINIT | STA_NODISK;
    return Stat;
  }

  /* 74+ dummy clocks with CS high and card deselected, per SD SPI spec */
  SD_CS_HIGH();
  for (n = 0; n < 10; n++) SPI_RW(0xFF);

  ty = 0;

  if (SD_SendCmd(CMD0, 0) == 1) /* card enters idle state */
  {
    start = HAL_GetTick();

    if (SD_SendCmd(CMD8, 0x1AA) == 1) /* SDC v2 -- check voltage range echo */
    {
      for (n = 0; n < 4; n++) ocr[n] = SPI_RW(0xFF);

      if (ocr[2] == 0x01 && ocr[3] == 0xAA)
      {
        /* Card supports 2.7-3.6V, proceed with ACMD41 (HCS bit set) */
        while (((HAL_GetTick() - start) < SD_TIMEOUT_MS) &&
               (SD_SendCmd(ACMD41, 1UL << 30) != 0)) { /* retry */ }

        if (((HAL_GetTick() - start) < SD_TIMEOUT_MS) &&
            (SD_SendCmd(CMD58, 0) == 0)) /* read OCR to check CCS bit */
        {
          for (n = 0; n < 4; n++) ocr[n] = SPI_RW(0xFF);
          ty = (ocr[0] & 0x40) ? (CT_SD2 | CT_BLOCK) : CT_SD2; /* CCS bit -> block addressing */
        }
      }
    }
    else /* SDC v1 or MMC */
    {
      BYTE cmd;
      if (SD_SendCmd(ACMD41, 0) <= 1)
      {
        ty = CT_SD1;
        cmd = ACMD41;
      }
      else
      {
        ty = CT_MMC;
        cmd = CMD1;
      }

      while (((HAL_GetTick() - start) < SD_TIMEOUT_MS) &&
             (SD_SendCmd(cmd, 0) != 0)) { /* retry */ }

      /* Fix block length to 512 bytes for byte-addressed cards */
      if (!(ty && (SD_SendCmd(CMD16, 512) == 0))) ty = 0;
    }
  }

  CardType = ty;
  SD_Deselect();

  if (ty)
  {
    Stat &= ~STA_NOINIT;
  }
  else
  {
    Stat = STA_NOINIT;
  }

  return Stat;
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
  if (pdrv != 0) return STA_NOINIT;

  if (!SD_isCardPresent())
  {
    Stat = STA_NOINIT | STA_NODISK;
  }

  return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
  if (pdrv != 0 || count == 0) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  /* Convert to byte address if card uses byte (not block) addressing */
  if (!(CardType & CT_BLOCK)) sector *= 512;

  if (count == 1)
  {
    if ((SD_SendCmd(CMD17, sector) == 0) && SD_RxDataBlock(buff, 512))
    {
      count = 0;
    }
  }
  else
  {
    /* Single-block loop for simplicity/robustness */
    if (SD_SendCmd(CMD17, sector) == 0)
    {
      do
      {
        if (!SD_RxDataBlock(buff, 512)) break;
        buff += 512;
        sector += (CardType & CT_BLOCK) ? 1 : 512;
      } while (--count);
    }
  }

  SD_Deselect();

  return count ? RES_ERROR : RES_OK;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
  /* USER CODE BEGIN WRITE */
  if (pdrv != 0 || count == 0) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;
  if (Stat & STA_PROTECT) return RES_WRPRT;

  if (!(CardType & CT_BLOCK)) sector *= 512;

  if (count == 1)
  {
    if ((SD_SendCmd(CMD24, sector) == 0) && SD_TxDataBlock(buff, 0xFE))
    {
      count = 0;
    }
  }
  else
  {
    do
    {
      if (SD_SendCmd(CMD24, sector) != 0) break;
      if (!SD_TxDataBlock(buff, 0xFE)) break;
      buff += 512;
      sector += (CardType & CT_BLOCK) ? 1 : 512;
    } while (--count);
  }

  SD_Deselect();

  return count ? RES_ERROR : RES_OK;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
  DRESULT res = RES_ERROR;

  if (pdrv != 0) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
    case CTRL_SYNC:
      if (SD_Select())
      {
        SD_Deselect();
        res = RES_OK;
      }
      break;

    case GET_SECTOR_SIZE:
      *(WORD *)buff = 512;
      res = RES_OK;
      break;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1;
      res = RES_OK;
      break;

    case GET_SECTOR_COUNT:
    {
      BYTE csd[16];
      DWORD cap;

      if ((SD_SendCmd(CMD9, 0) == 0) && SD_RxDataBlock(csd, 16))
      {
        if ((csd[0] >> 6) == 1) /* CSD version 2.0 (SDHC/SDXC) */
        {
          DWORD csize = ((DWORD)(csd[7] & 0x3F) << 16) | ((DWORD)csd[8] << 8) | csd[9];
          cap = (csize + 1) * 1024; /* sectors (512B each) */
        }
        else /* CSD version 1.0 (SDSC) */
        {
          DWORD csize = ((DWORD)(csd[6] & 0x03) << 10) | ((DWORD)csd[7] << 2) | (csd[8] >> 6);
          BYTE n = (csd[9] & 0x03) + ((csd[10] & 0x80) >> 7) + ((csd[5] & 0x0F) << 1) + 2;
          cap = (csize + 1) << (n - 9);
        }
        *(DWORD *)buff = cap;
        res = RES_OK;
      }
      SD_Deselect();
      break;
    }

    default:
      res = RES_PARERR;
      break;
  }

  return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */
