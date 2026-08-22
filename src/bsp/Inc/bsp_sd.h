#ifndef __BSP_SD_H
#define __BSP_SD_H

#include <stdint.h>
#include "ff.h"

extern uint8_t SD_TYPE;

// SD卡类型定义
#define ERR         0x00
#define MMC         0x01
#define V1          0x02
#define V2          0x04
#define V2HC        0x06

#define DUMMY_BYTE           0xFF 
#define MSD_BLOCKSIZE        512

// SD CMD定义
#define CMD0    0       // 卡复位
#define CMD1    1
#define CMD8    8       // SEND_IF_COND
#define CMD9    9       // 读CSD数据
#define CMD10   10      // 读CID数据
#define CMD12   12      // 停止数据传输
#define CMD16   16      // 设置SectorSize
#define CMD17   17      // 读单扇区
#define CMD18   18      // 读多扇区
#define CMD23   23      // 设置多扇区写入前预擦除
#define CMD24   24      // 写单扇区
#define CMD25   25      // 写多扇区
#define CMD41   41      // SD_SEND_OP_COND
#define CMD55   55      // APP_CMD
#define CMD58   58      // 读OCR信息
#define CMD59   59      // 使能/禁止CRC

// 数据写入回应字
#define MSD_DATA_OK                0x05
#define MSD_DATA_CRC_ERROR         0x0B
#define MSD_DATA_WRITE_ERROR       0x0D
#define MSD_DATA_OTHER_ERROR       0xFF

// SD卡回应标记字
#define MSD_RESPONSE_NO_ERROR      0x00
#define MSD_IN_IDLE_STATE          0x01
#define MSD_ERASE_RESET            0x02
#define MSD_ILLEGAL_COMMAND        0x04
#define MSD_COM_CRC_ERROR          0x08
#define MSD_ERASE_SEQUENCE_ERROR   0x10
#define MSD_ADDRESS_ERROR          0x20
#define MSD_PARAMETER_ERROR        0x40
#define MSD_RESPONSE_FAILURE       0xFF

typedef struct {
    uint8_t  CSDStruct;
    uint8_t  SysSpecVersion;
    uint8_t  Reserved1;
    uint8_t  TAAC;
    uint8_t  NSAC;
    uint8_t  MaxBusClkFrec;
    uint16_t CardComdClasses;
    uint8_t  RdBlockLen;
    uint8_t  PartBlockRead;
    uint8_t  WrBlockMisalign;
    uint8_t  RdBlockMisalign;
    uint8_t  DSRImpl;
    uint8_t  Reserved2;
    uint32_t DeviceSize;
    uint8_t  MaxRdCurrentVDDMin;
    uint8_t  MaxRdCurrentVDDMax;
    uint8_t  MaxWrCurrentVDDMin;
    uint8_t  MaxWrCurrentVDDMax;
    uint8_t  DeviceSizeMul;
    uint8_t  EraseGrSize;
    uint8_t  EraseGrMul;
    uint8_t  WrProtectGrSize;
    uint8_t  WrProtectGrEnable;
    uint8_t  ManDeflECC;
    uint8_t  WrSpeedFact;
    uint8_t  MaxWrBlockLen;
    uint8_t  WriteBlockPaPartial;
    uint8_t  Reserved3;
    uint8_t  ContentProtectAppli;
    uint8_t  FileFormatGrouop;
    uint8_t  CopyFlag;
    uint8_t  PermWrProtect;
    uint8_t  TempWrProtect;
    uint8_t  FileFormat;
    uint8_t  ECC;
    uint8_t  CSD_CRC;
    uint8_t  Reserved4;
} MSD_CSD;

typedef struct {
    uint8_t  ManufacturerID;
    uint16_t OEM_AppliID;
    uint32_t ProdName1;
    uint8_t  ProdName2;
    uint8_t  ProdRev;
    uint32_t ProdSN;
    uint8_t  Reserved1;
    uint16_t ManufactDate;
    uint8_t  CID_CRC;
    uint8_t  Reserved2;
} MSD_CID;

typedef struct {
    MSD_CSD  CSD;
    MSD_CID  CID;
    uint32_t Capacity;
    uint32_t BlockSize;
    uint16_t RCA;
    uint8_t  CardType;
    uint32_t SpaceTotal;
    uint32_t SpaceFree;
} MSD_CARDINFO, *PMSD_CARDINFO;

extern MSD_CARDINFO SD0_CardInfo;

/* 重构后 API 列表 */
uint8_t  bsp_sd_init(void);
void     bsp_sd_cs(uint8_t select);
uint32_t bsp_sd_get_sector_count(void);
uint8_t  bsp_sd_get_cid(uint8_t *cid_data);
uint8_t  bsp_sd_get_csd(uint8_t *csd_data);
int      bsp_sd_get_card_info(PMSD_CARDINFO SD0_CardInfo);
uint8_t  bsp_sd_receive_data(uint8_t *data, uint16_t len);
uint8_t  bsp_sd_send_block(uint8_t *buf, uint8_t cmd);
uint8_t  bsp_sd_read_disk(uint8_t *buf, uint32_t sector, uint8_t cnt);
uint8_t  bsp_sd_write_disk(uint8_t *buf, uint32_t sector, uint8_t cnt);

void     bsp_sd_get_capacity(void);
void     bsp_sd_write_file(char filename[], BYTE write_buff[], uint8_t bufSize);

#endif