#define TRUE  1
#define FALSE 0
#define bool BYTE

#include "stm32f1xx_hal.h"
#include "main.h"

#include "diskio.h"
#include "fatfs_sd.h"
#include "fatfs.h"

extern SPI_HandleTypeDef hspi1;
extern volatile uint8_t Timer1, Timer2;                    /* 10ms ë§ˆë‹¤ ê°ì†Œ?˜ëŠ” ?€?´ë¨¸ */

static volatile DSTATUS Stat = STA_NOINIT;              /* ?”ìŠ¤???íƒœ Flag*/
static uint8_t CardType;                                /* SD ?€??0:MMC, 1:SDC, 2:Block addressing */
static uint8_t PowerFlag = 0;                           /* Power ?íƒœ Flag */

/* SPI Chip Select */
static void SELECT(void)
{
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

/* SPI Chip Deselect */
static void DESELECT(void)
{
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

/* SPI ?°ì´???„ì†¡ */
static void SPI_TxByte(BYTE data)
{
  while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
  HAL_SPI_Transmit(&hspi1, &data, 1, SPI_TIMEOUT);
}

/* SPI ?°ì´???¡ìˆ˜??ë¦¬í„´???¨ìˆ˜ */
static uint8_t SPI_RxByte(void)
{
  uint8_t dummy, data;
  dummy = 0xFF;
  data = 0;

  while ((HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY));
  HAL_SPI_TransmitReceive(&hspi1, &dummy, &data, 1, SPI_TIMEOUT);

  return data;
}

/* SPI ?°ì´???¡ìˆ˜???¬ì¸?°í˜• ?¨ìˆ˜ */
static void SPI_RxBytePtr(uint8_t *buff)
{
  *buff = SPI_RxByte();
}

/* SDì¹´ë“œ Ready ?€ê¸?*/
static uint8_t SD_ReadyWait(void)
{
  uint8_t res;

  /* 500ms ì¹´ìš´??ì¤€ë¹?*/
  Timer2 = 50;
  SPI_RxByte();

  do
  {
    /* 0xFF ê°’ì´ ?˜ì‹ ????ê¹Œì? SPI ?µì‹  */
    res = SPI_RxByte();
  } while ((res != 0xFF) && Timer2);

  return res;
}

/* ?„ì› ì¼œê¸° */
static void SD_PowerOn(void)
{
  uint8_t cmd_arg[6];
  uint32_t Count = 0x1FFF;

  /* Deselect ?íƒœ?ì„œ SPI ë©”ì‹œì§€ë¥??„ì†¡?˜ì—¬ ?€ê¸°ìƒ?œë¡œ ë§Œë“ ?? */
  DESELECT();

  for(int i = 0; i < 10; i++)
  {
    SPI_TxByte(0xFF);
  }

  /* SPI Chips Select */
  SELECT();

  /* ì´ˆê¸° GO_IDLE_STATE ?íƒœ ?„í™˜ */
  cmd_arg[0] = (CMD0 | 0x40);
  cmd_arg[1] = 0;
  cmd_arg[2] = 0;
  cmd_arg[3] = 0;
  cmd_arg[4] = 0;
  cmd_arg[5] = 0x95;

  /* ëª…ë ¹ ?„ì†¡ */
  for (int i = 0; i < 6; i++)
  {
    SPI_TxByte(cmd_arg[i]);
  }

  /* ?‘ë‹µ ?€ê¸?*/
  while ((SPI_RxByte() != 0x01) && Count)
  {
    Count--;
  }

  DESELECT();
  SPI_TxByte(0XFF);

  PowerFlag = 1;
}

/* ?„ì› ?„ê¸° */
static void SD_PowerOff(void)
{
  PowerFlag = 0;
}

/* ?„ì› ?íƒœ ?•ì¸ */
static uint8_t SD_CheckPower(void)
{
  /*  0=off, 1=on */
  return PowerFlag;
}

/* ?°ì´???¨í‚· ?˜ì‹  */
static bool SD_RxDataBlock(BYTE *buff, UINT btr)
{
  uint8_t token;

  /* 100ms ?€?´ë¨¸ */
  Timer1 = 10;

  /* ?‘ë‹µ ?€ê¸?*/
  do
  {
    token = SPI_RxByte();
  } while((token == 0xFF) && Timer1);

  /* 0xFE ?´ì™¸ Token ?˜ì‹  ???ëŸ¬ ì²˜ë¦¬ */
  if(token != 0xFE)
    return FALSE;

  /* ë²„í¼???°ì´???˜ì‹  */
  while(btr--)
  {
    SPI_RxBytePtr(buff++);
  }

  SPI_RxByte(); /* CRC ë¬´ì‹œ */
  SPI_RxByte();

  return TRUE;
}

/* ?°ì´???„ì†¡ ?¨í‚· */
#if _READONLY == 0
static bool SD_TxDataBlock(const BYTE *buff, BYTE token)
{
  uint8_t resp = 0xFF, i = 0;  /* resp ì´ˆê¸°??*/
  uint16_t wc;

  /* SDì¹´ë“œ ì¤€ë¹??€ê¸?*/
  if (SD_ReadyWait() != 0xFF)
    return FALSE;

  /* ? í° ?„ì†¡ */
  SPI_TxByte(token);

  /* ?°ì´??? í°??ê²½ìš° */
  if (token != 0xFD)
  {
    wc = 512;

    /* 512 ë°”ì´???°ì´???„ì†¡ */
    while(wc--)
    {
      SPI_TxByte(*buff++);
    }

    SPI_RxByte();       /* CRC ë¬´ì‹œ */
    SPI_RxByte();

    /* ?°ì´???‘ë‹µ ?˜ì‹  */
    while (i <= 64)
    {
      resp = SPI_RxByte();

      /* ?ëŸ¬ ?‘ë‹µ ì²˜ë¦¬ */
      if ((resp & 0x1F) == 0x05)
        break;

      i++;
    }

    /* SPI ?˜ì‹  ë²„í¼ Clear - ?€?„ì•„??ì¶”ê? */
    Timer1 = 20; /* 200ms ?€?„ì•„??*/
    while (SPI_RxByte() == 0 && Timer1);
  }

  if ((resp & 0x1F) == 0x05)
    return TRUE;
  else
    return FALSE;
}
#endif /* _READONLY */

/* CMD ?¨í‚· ?„ì†¡ */
static BYTE SD_SendCmd(BYTE cmd, DWORD arg)
{
  uint8_t crc, res;

  /* SDì¹´ë“œ ?€ê¸?*/
  if (SD_ReadyWait() != 0xFF)
    return 0xFF;

  /* ëª…ë ¹ ?¨í‚· ?„ì†¡ */
  SPI_TxByte(cmd); 			/* Command */
  SPI_TxByte((BYTE) (arg >> 24)); 	/* Argument[31..24] */
  SPI_TxByte((BYTE) (arg >> 16)); 	/* Argument[23..16] */
  SPI_TxByte((BYTE) (arg >> 8)); 	/* Argument[15..8] */
  SPI_TxByte((BYTE) arg); 		/* Argument[7..0] */

  /* ëª…ë ¹ë³?CRC ì¤€ë¹?*/
  crc = 0;
  if (cmd == CMD0)
    crc = 0x95; /* CRC for CMD0(0) */

  if (cmd == CMD8)
    crc = 0x87; /* CRC for CMD8(0x1AA) */

  /* CRC ?„ì†¡ */
  SPI_TxByte(crc);

  /* CMD12 Stop Reading ëª…ë ¹??ê²½ìš°?ëŠ” ?‘ë‹µ ë°”ì´???˜ë‚˜ë¥?ë²„ë¦°??*/
  if (cmd == CMD12)
    SPI_RxByte();

  /* 10???´ì— ?•ìƒ ?°ì´?°ë? ?˜ì‹ ?œë‹¤. */
  uint8_t n = 10;
  do
  {
    res = SPI_RxByte();
  } while ((res & 0x80) && --n);

  return res;
}

/*-----------------------------------------------------------------------
  fatfs?ì„œ ?¬ìš©?˜ëŠ” Global ?¨ìˆ˜??  user_diskio.c ?Œì¼?ì„œ ?¬ìš©?œë‹¤.
-----------------------------------------------------------------------*/

/* SDì¹´ë“œ ì´ˆê¸°??*/
DSTATUS SD_disk_initialize(BYTE drv)
{
  uint8_t n, type, ocr[4];

  /* ?œì¢…ë¥˜ì˜ ?œë¼?´ë¸Œë§?ì§€??*/
  if(drv)
    return STA_NOINIT;

  /* SDì¹´ë“œ ë¯¸ì‚½??*/
  if(Stat & STA_NODISK)
    return Stat;

  /* ¡Ú ¿©±â¿¡ Ãß°¡: ÃÊ±âÈ­ Àü Àú¼ÓÀ¸·Î ÀüÈ¯ */
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  HAL_SPI_Init(&hspi1);


  /* SDì¹´ë“œ Power On */
  SD_PowerOn();

  /* SPI ?µì‹ ???„í•´ Chip Select */
  SELECT();

  /* SDì¹´ë“œ ?€?…ë???ì´ˆê¸°??*/
  type = 0;

  /* Idle ?íƒœ ì§„ì… */
  if (SD_SendCmd(CMD0, 0) == 1)
  {
    /* ?€?´ë¨¸ 1ì´??¤ì • */
    Timer1 = 100;

    /* SD ?¸í„°?˜ì´???™ì‘ ì¡°ê±´ ?•ì¸ */
    if (SD_SendCmd(CMD8, 0x1AA) == 1)
    {
      /* SDC Ver2+ */
      for (n = 0; n < 4; n++)
      {
        ocr[n] = SPI_RxByte();
      }

      if (ocr[2] == 0x01 && ocr[3] == 0xAA)
      {
        /* 2.7-3.6V ?„ì••ë²”ìœ„ ?™ì‘ */
        do {
          if (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 1UL << 30) == 0)
            break; /* ACMD41 with HCS bit */
        } while (Timer1);

        if (Timer1 && SD_SendCmd(CMD58, 0) == 0)
        {
          /* Check CCS bit */
          for (n = 0; n < 4; n++)
          {
            ocr[n] = SPI_RxByte();
          }

          type = (ocr[0] & 0x40) ? 6 : 2;
        }
      }
    }
    else
    {
      /* SDC Ver1 or MMC */
      type = (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 0) <= 1) ? 2 : 1; /* SDC : MMC */

      do {
        if (type == 2)
        {
          if (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 0) == 0)
            break; /* ACMD41 */
        }
        else
        {
          if (SD_SendCmd(CMD1, 0) == 0)
            break; /* CMD1 */
        }
      } while (Timer1);

      if (!Timer1 || SD_SendCmd(CMD16, 512) != 0)
      {
        /* ë¸”ëŸ­ ê¸¸ì´ ? íƒ */
        type = 0;
      }
    }
  }

  CardType = type;

  DESELECT();

  SPI_RxByte(); /* Idle ?íƒœ ?„í™˜ (Release DO) */

  if (type)
  {
    /* Clear STA_NOINIT */
    Stat &= ~STA_NOINIT;

    /* ¡Ú ¿©±â¿¡ Ãß°¡: ÃÊ±âÈ­ ¼º°ø ÈÄ °í¼ÓÀ¸·Î ÀüÈ¯ */
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    HAL_SPI_Init(&hspi1);
  }
  else
  {
    /* Initialization failed */
    SD_PowerOff();
  }

  return Stat;
}

/* ?”ìŠ¤???íƒœ ?•ì¸ */
DSTATUS SD_disk_status(BYTE drv)
{
  if (drv)
    return STA_NOINIT;

  return Stat;
}

/* ?¹í„° ?½ê¸° */
DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
  if (pdrv || !count)
    return RES_PARERR;

  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  if (!(CardType & 4))
    sector *= 512;      /* ì§€??sectorë¥?Byte addressing ?¨ìœ„ë¡?ë³€ê²?*/

  SELECT();

  if (count == 1)
  {
    /* ?±ê? ë¸”ë¡ ?½ê¸° */
    if ((SD_SendCmd(CMD17, sector) == 0) && SD_RxDataBlock(buff, 512))
      count = 0;
  }
  else
  {
    /* ?¤ì¤‘ ë¸”ë¡ ?½ê¸° */
    if (SD_SendCmd(CMD18, sector) == 0)
    {
      do {
        if (!SD_RxDataBlock(buff, 512))
          break;

        buff += 512;
      } while (--count);

      /* STOP_TRANSMISSION, ëª¨ë“  ë¸”ëŸ­?????½ì? ?? ?„ì†¡ ì¤‘ì? ?”ì²­ */
      SD_SendCmd(CMD12, 0);
    }
  }

  DESELECT();
  SPI_RxByte(); /* Idle ?íƒœ(Release DO) */

  return count ? RES_ERROR : RES_OK;
}

/* ?¹í„° ?°ê¸° */
#if _READONLY == 0
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
  if (pdrv || !count)
    return RES_PARERR;

  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  if (Stat & STA_PROTECT)
    return RES_WRPRT;

  if (!(CardType & 4))
    sector *= 512; /* ì§€??sectorë¥?Byte addressing ?¨ìœ„ë¡?ë³€ê²?*/

  SELECT();

  if (count == 1)
  {
    /* ?±ê? ë¸”ë¡ ?°ê¸° */
    if ((SD_SendCmd(CMD24, sector) == 0) && SD_TxDataBlock(buff, 0xFE))
      count = 0;
  }
  else
  {
    /* ?¤ì¤‘ ë¸”ë¡ ?°ê¸° */
    if (CardType & 2)
    {
      SD_SendCmd(CMD55, 0);
      SD_SendCmd(CMD23, count); /* ACMD23 */
    }

    if (SD_SendCmd(CMD25, sector) == 0)
    {
      do {
        if(!SD_TxDataBlock(buff, 0xFC))
          break;

        buff += 512;
      } while (--count);

      if(!SD_TxDataBlock(0, 0xFD))
      {
        count = 1;
      }
    }
  }

  DESELECT();
  SPI_RxByte();

  return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY */

/* ê¸°í? ?¨ìˆ˜ */
DRESULT SD_disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
  DRESULT res;
  BYTE n, csd[16], *ptr = buff;
  WORD csize;

  if (drv)
    return RES_PARERR;

  res = RES_ERROR;

  if (ctrl == CTRL_POWER)
  {
    switch (*ptr)
    {
    case 0:
      if (SD_CheckPower())
        SD_PowerOff();          /* Power Off */
      res = RES_OK;
      break;
    case 1:
      SD_PowerOn();             /* Power On */
      res = RES_OK;
      break;
    case 2:
      *(ptr + 1) = (BYTE) SD_CheckPower();
      res = RES_OK;             /* Power Check */
      break;
    default:
      res = RES_PARERR;
    }
  }
  else
  {
    if (Stat & STA_NOINIT)
      return RES_NOTRDY;

    SELECT();

    switch (ctrl)
    {
    case GET_SECTOR_COUNT:
      /* SDì¹´ë“œ ??Sector??ê°œìˆ˜ (DWORD) */
      if ((SD_SendCmd(CMD9, 0) == 0) && SD_RxDataBlock(csd, 16))
      {
        if ((csd[0] >> 6) == 1)
        {
          /* SDC ver 2.00 - ?•í™•??C_SIZE ?„ë“œ ?Œì‹± */
          csize = ((DWORD)(csd[7] & 0x3F) << 16) | ((DWORD)csd[8] << 8) | csd[9];
          *(DWORD*) buff = (csize + 1) << 10;
        }
        else
        {
          /* MMC or SDC ver 1.XX */
          n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
          csize = (csd[8] >> 6) + ((WORD) csd[7] << 2) + ((WORD) (csd[6] & 3) << 10) + 1;
          *(DWORD*) buff = (DWORD) csize << (n - 9);
        }

        res = RES_OK;
      }
      break;

    case GET_SECTOR_SIZE:
      /* ?¹í„°???¨ìœ„ ?¬ê¸° (WORD) */
      *(WORD*) buff = 512;
      res = RES_OK;
      break;

    case CTRL_SYNC:
      /* ?°ê¸° ?™ê¸°??*/
      if (SD_ReadyWait() == 0xFF)
        res = RES_OK;
      break;

    case MMC_GET_CSD:
      /* CSD ?•ë³´ ?˜ì‹  (16 bytes) */
      if (SD_SendCmd(CMD9, 0) == 0 && SD_RxDataBlock(ptr, 16))
        res = RES_OK;
      break;

    case MMC_GET_CID:
      /* CID ?•ë³´ ?˜ì‹  (16 bytes) */
      if (SD_SendCmd(CMD10, 0) == 0 && SD_RxDataBlock(ptr, 16))
        res = RES_OK;
      break;

    case MMC_GET_OCR:
      /* OCR ?•ë³´ ?˜ì‹  (4 bytes) */
      if (SD_SendCmd(CMD58, 0) == 0)
      {
        for (n = 0; n < 4; n++)
        {
          *ptr++ = SPI_RxByte();
        }

        res = RES_OK;
      }
      break;

    default:
      res = RES_PARERR;
    }

    DESELECT();
    SPI_RxByte();
  }

  return res;
}

/* main()ÀÌ³ª ÃÊ±âÈ­ ½ÃÁ¡¿¡ µü ÇÑ ¹ø */





//================================================
//================================================
//================================================
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t total, free;
char buffer[100];

uint8_t SD_Error_Chk(FRESULT errCode, const char* msg)
{
	if(errCode != FR_OK)
	{
		printf("%s Error = %d\r\n", msg, errCode);   // ¿¡·¯ ÄÚµåµµ °°ÀÌ Ãâ·Â
		return 1;        // ¡Ú ¿¡·¯ÀÏ ¶§ 1(Âü) ¡æ È£ÃâºÎ¿¡¼­ return Ã³¸®
	}
	else
	{
		//printf("%s OK\r\n", msg);
		return 0;        // ¡Ú Á¤»óÀÏ ¶§ 0(°ÅÁş) ¡æ È£ÃâºÎ¿¡¼­ °è¼Ó ÁøÇà
	}
}

//¿©±âºÎÅÍ ³»ÄÚµå

/**
  * @brief  SDÄ«µåÀÇ ÀüÃ¼/¿©À¯ ¿ë·®À» °è»êÇÏ°í Ä«µå »óÅÂ¸¦ Ã¼Å©
  * @retval 1: ¿¡·¯(Ä«µå Åë½Å ½ÇÆĞ µî), 0: Á¤»ó
  */
uint8_t SD_CheckFreeSpace(void)
{
  if(SD_Error_Chk(f_getfree("", &fre_clust, &pfs), "f_getfree"))
  {
    return 1;
  }

  total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
  free  = (uint32_t)(fre_clust * pfs->csize * 0.5);

  printf("Total: %lu KB, Free: %lu KB\r\n", total, free);

  return 0;
}

void SD_Init(void)
{
  if(SD_Error_Chk(f_mount(&fs, "", 0), "f_mount"))return;
  if(SD_CheckFreeSpace())return;

    /* 5°³ ¸í·É¾î µğ·ºÅÍ¸®, ºÎÆÃ ½Ã ÇÑ ¹ø¸¸ »ı¼º (ÀÌ¹Ì ÀÖÀ¸¸é FR_EXIST, ¹«½Ã) */

    f_mkdir("TDAH");
    f_mkdir("TOFH");
    f_mkdir("TDDH");
    f_mkdir("TFDH");
    f_mkdir("TNOH");

  printf("SD_Init Done\r\n");
}

void SD_Read_All(const char* filename)
{

	if(SD_Error_Chk(f_open(&fil, filename, FA_READ), "f_open"))return;

	while(f_gets(buffer, sizeof(buffer), &fil))
	{
	  printf("%s", buffer);
	}

	SD_Error_Chk(f_close(&fil), "f_close");
	printf("SD_Read_All Done\r\n");

}

void SD_Read_Addr(const char* filename, uint16_t add, uint16_t len)
{
  UINT br;
  uint8_t buf[100] = {0,};

  if(len > 100)
  {
    printf("len Over\r\n");
    return;
  }


  if(SD_Error_Chk(f_open(&fil, filename, FA_READ), "f_open"))return;


  if(SD_Error_Chk(f_lseek(&fil, add), "f_lseek"))
  {
  	f_close(&fil);
	return;
  }

  if(SD_Error_Chk(f_read(&fil, buf, len, &br), "f_read"))
  {
	f_close(&fil);
	return;
  }

  if(br == 0) printf("No data (offset %u is beyond EOF)\r\n", add); // ÀĞÀº µ¥ÀÌÅÍ°¡ ÀüÇô ¾øÀ½ (offsetÀÌ EOF ÀÌÈÄ)
  else
  {
    printf("read %u bytes: %.*s\r\n", br, br, buf);
    if(br < len) printf("Partial read: requested %u, got %u bytes\r\n", len, br);   // ¿äÃ»ÇÑ ±æÀÌº¸´Ù Àû°Ô ÀĞÈû (ÆÄÀÏ ³¡¿¡ °É¸²)
  }

  SD_Error_Chk(f_close(&fil), "f_close");
  printf("SD_Read_Addr Done\r\n");
}



void SD_Write_Addr(const char* filename, const uint8_t* data, uint16_t len)
{
  UINT bw;
  uint32_t pos;


  if(SD_Error_Chk(f_open(&fil, filename, FA_OPEN_ALWAYS | FA_WRITE), "f_open"))
    return;

  pos = f_size(&fil);		   // ÇöÀç ÆÄÀÏ Å©±â = ³¡ À§Ä¡
  if(SD_Error_Chk(f_lseek(&fil, pos), "f_lseek"))
  {
    f_close(&fil);
    return;
  }



  if(SD_Error_Chk(f_write(&fil, data, len, &bw), "f_write"))
  {
    f_close(&fil);
    return;
  }

  if(bw < len)
    printf("Partial write: requested %u, written %u bytes\r\n", len, bw);
  else
    printf("write %u bytes at offset %u\r\n", bw, pos);

  SD_Error_Chk(f_close(&fil), "f_close");
  printf("SD_Write_Addr Done\r\n");
}



typedef struct {
  uint16_t hhmm;     // 0~2359 ÇüÅÂ·Î ½Ã:ºĞ (³¯Â¥´Â ÆÄÀÏ¸íÀ¸·Î ºĞ¸®, ¾Æ·¡ ¼³¸í)
  uint16_t len;      // ÀÌ ·¹ÄÚµåÀÇ µ¥ÀÌÅÍ ±æÀÌ
  // ±× ´ÙÀ½¿¡ ½ÇÁ¦ µ¥ÀÌÅÍ(len¹ÙÀÌÆ®)°¡ ÀÌ¾îÁü
} RecordHeader;


void SD_Write_Record(char* dirName, uint32_t yymmdd, uint16_t hhmm, const uint8_t* data, uint16_t len)
{
  char filename[24];   // "TNOH/261231.txt" µî ¿©À¯ÀÖ°Ô


  sprintf(filename, "%s/%lu.txt", dirName, yymmdd);   // "TNOH/260621.txt"

  uint32_t pos;
  UINT bw;
  RecordHeader hdr = { hhmm, len };

  if(SD_Error_Chk(f_open(&fil, filename, FA_OPEN_ALWAYS | FA_WRITE), "f_open"))
    return;

  pos = f_size(&fil);
  if(SD_Error_Chk(f_lseek(&fil, pos), "f_lseek"))
  {
    f_close(&fil);
    return;
  }

  if(SD_Error_Chk(f_write(&fil, &hdr, sizeof(hdr), &bw), "f_write(hdr)")) // ¨ç Çì´õ ¸ÕÀú ¾¸
  {
	  f_close(&fil);
	  return;
  }

  if(bw < sizeof(hdr))	 // Çì´õ ºÎºĞ¾²±â = Ä¡¸íÀû ¡æ Áï½Ã Á¾·á
  {
	printf("Header write incomplete! requested %u, written %u\r\n", (unsigned)sizeof(hdr), bw);
	f_close(&fil);
	return;
  }

  if(SD_Error_Chk(f_write(&fil, data, len, &bw), "f_write(data)"))// ¨è µ¥ÀÌÅÍ ÀÌ¾î¼­ ¾¸
  {
	 f_close(&fil);
	 return;
  }
  if(bw < len)	 // µ¥ÀÌÅÍ ºÎºĞ¾²±â = °æ°í¸¸ ³²±â°í °è¼Ó ÁøÇà (Çì´õ´Â ÀÌ¹Ì Á¤»óÀûÀ¸·Î ½áÁ³À¸´Ï ÆÄÀÏ ÀÚÃ¼´Â ¾È ±úÁü)
  {
	printf("Data write incomplete! requested %u, written %u\r\n", len, bw);
	/* return ¾øÀ½ ? ÀÇµµÀûÀ¸·Î °è¼Ó ÁøÇàÇØ¼­ f_close()±îÁö µµ´Ş */
  }

  SD_Error_Chk(f_close(&fil), "f_close");
}

void SD_Read_Range(const char* dirName, uint32_t yymmdd,  uint16_t startHHMM, uint16_t endHHMM)
{
  RecordHeader hdr;
  UINT br;
  uint8_t buf[300];   // ÃÖ´ë ±æÀÌ¸¸Å­ ¹öÆÛ (¿î¿µ ½Ã ÀûÀıÈ÷ Á¶Á¤)
  char filename[24];
  sprintf(filename, "%s/%lu.txt", dirName, yymmdd);   // "TNOH/260621.txt"

  if(SD_Error_Chk(f_open(&fil, filename, FA_READ), "f_open"))
    return;

  while(1)
  {
    FRESULT res = f_read(&fil, &hdr, sizeof(hdr), &br);
    if(res != FR_OK || br < sizeof(hdr))
      break;   // ´õ ÀĞÀ» Çì´õ ¾øÀ½ (ÆÄÀÏ ³¡)

    if(hdr.hhmm >= startHHMM && hdr.hhmm <= endHHMM)
    {
      // ¡Ú ¹üÀ§ ¾È ? µ¥ÀÌÅÍ ½ÇÁ¦·Î ÀĞÀ½
      if(SD_Error_Chk(f_read(&fil, buf, hdr.len, &br), "f_read"))
	  {
		f_close(&fil);
		return;
	  }
	  if(br < hdr.len) printf("Warning: header says %u bytes but only %u read (corrupted?)\r\n", hdr.len, br);

	  printf("%s [%04u] %u bytes: %.*s\r\n",dirName, hdr.hhmm, br, br, buf);
      // ¿©±â¼­ ¼­¹ö ¼Û½Å¿ë ¹öÆÛ¿¡ ´©ÀûÇÏ°Å³ª Áï½Ã Àü¼Û
    }
    else
    {
      // ¡Ú ¹üÀ§ ¹Û ? µ¥ÀÌÅÍ´Â ÀĞÁö ¾Ê°í ±×³É °Ç³Ê¶Ù±â (Æ÷ÀÎÅÍ¸¸ ÀÌµ¿)
      if(SD_Error_Chk(f_lseek(&fil, f_tell(&fil) + hdr.len), "f_lseek"))
      {
	    f_close(&fil);
	    return;
	  }
    }
  }

  SD_Error_Chk(f_close(&fil), "f_close");
}

/**
 * @brief  ÁöÁ¤ÇÑ µğ·ºÅÍ¸® ¾È¿¡¼­ ÆÄÀÏ °³¼ö°¡ maxFiles¸¦ ÃÊ°úÇÏ¸é,
 *         °¡Àå ¿À·¡µÈ(=³¯Â¥°¡ °¡Àå ÀÌ¸¥) ÆÄÀÏ 1°³¸¦ »èÁ¦ÇÑ´Ù.
 *
 * @param  dirPath  °Ë»çÇÒ µğ·ºÅÍ¸® °æ·Î (¿¹: "TDAH")
 * @param  maxFiles Çã¿ëÇÒ ÃÖ´ë ÆÄÀÏ °³¼ö (¿¹: 62 ¡æ µÎ ´ŞÄ¡ º¸°ü)
 *
 * @note   ÆÄÀÏ¸íÀÌ "YYMMDD.txt" Çü½ÄÀ¸·Î Ç×»ó °°Àº ÀÚ¸´¼ö¶ó¼­,
 *         ¹®ÀÚ¿­ ºñ±³(strcmp)¸¸À¸·Î ³¯Â¥ ¼ø¼­ ºñ±³°¡ Á¤È®È÷ ¼º¸³ÇÑ´Ù.
 */
void SD_CleanupOldFiles_Simple(const char* dirPath, uint16_t maxFiles)
{
  DIR dir;
  FILINFO fno;
  FRESULT res;                 // ¡Ú Ãß°¡
  char oldestName[32] = "";
  uint16_t count = 0;

  if(SD_Error_Chk(f_opendir(&dir, dirPath), "f_opendir")) return;

  while((res = f_readdir(&dir, &fno)) == FR_OK && fno.fname[0] != 0)   // ¡Ú ¼öÁ¤
  {
    if(fno.fattrib & AM_DIR) continue;

    count++;

    if(oldestName[0] == 0 || strcmp(fno.fname, oldestName) < 0)
      strcpy(oldestName, fno.fname);
  }
  if(res != FR_OK)
  {
    printf("f_readdir Error = %d (dir scan incomplete)\r\n", res);
  }

  SD_Error_Chk(f_closedir(&dir), "f_closedir");

  if(count > maxFiles && oldestName[0] != 0)
  {
    char fullPath[40];
    sprintf(fullPath, "%s/%s", dirPath, oldestName);

    if(SD_Error_Chk(f_unlink(fullPath), "f_unlink"))
    {
      return;
    }
    printf("Deleted: %s (count was %u)\r\n", fullPath, count);
  }
}

void SD_CleanupAll(void)
{
  SD_CleanupOldFiles_Simple("TDAH", 31);
  SD_CleanupOldFiles_Simple("TOFH", 31);
  SD_CleanupOldFiles_Simple("TDDH", 31);
  SD_CleanupOldFiles_Simple("TFDH", 31);
  SD_CleanupOldFiles_Simple("TNOH", 31);
}
uint32_t tt1,tt2;
uint8_t clrFlag;
void SD_Test()
{
	uint16_t hhmm = 900;
	uint8_t timeStr[11] ={0,};
	static uint32_t timeStamp;

//	if(clrFlag)
//	{
//		clrFlag = 0;
//		SD_CleanupAll();
//	}
	if(HAL_GetTick()-timeStamp >= 5000)
	{

		timeStamp = HAL_GetTick();
		tt1 = timeStamp;
		sprintf(timeStr,"%08lu\r\n",HAL_GetTick());

		SD_Write_Record("TDAH",260623, hhmm, timeStr, 10);
		SD_Write_Record("TOFH",260624, hhmm, timeStr, 10);
		SD_Write_Record("TDDH",260625, hhmm, timeStr, 10);
		SD_Write_Record("TFDH",260626, hhmm, timeStr, 10);
		SD_Write_Record("TNOH",260627, hhmm, timeStr, 10);

		printf("%08lu\r\n",timeStamp);
		tt2= HAL_GetTick() -tt1;
	}

//	SD_Read_Range("TDAH",260623, 0, 2355);
//	SD_Read_Range("TOFH",260624, 0, 2355);
//	SD_Read_Range("TDDH",260625, 0, 2355);
//	SD_Read_Range("TFDH",260626, 0, 2355);
//	SD_Read_Range("TNOH",260627, 0, 2355);

}

