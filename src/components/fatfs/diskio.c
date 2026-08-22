/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for FatFs / CH585 SD Card Integration       */
/*-----------------------------------------------------------------------*/

#include "ff.h"         /* FatFs 基础定义 */
#include "diskio.h"     /* FatFs 磁盘 I/O 接口定义 */
#include "bsp_sd.h"     /* CH585 SD 卡底层驱动 */

/* 物理盘符映射 (默认仅使用 SD 卡作为盘符 0) */
#define DEV_MMC     0   /* Map MMC/SD card to physical drive 0 */


/*-----------------------------------------------------------------------*/
/* 获取设备状态 (Get Drive Status)                                        */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv       /* 物理盘符 */
)
{
    if (pdrv != DEV_MMC) {
        return STA_NOINIT;
    }

    // 检查 SD 卡类型标记，若为 ERR 则表明尚未初始化或异常
    if (SD_TYPE == ERR) {
        return STA_NOINIT;
    }

    return 0; // 返回 0 表示设备就绪
}


/*-----------------------------------------------------------------------*/
/* 初始化设备 (Initialize a Drive)                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv       /* 物理盘符 */
)
{
    if (pdrv != DEV_MMC) {
        return STA_NOINIT;
    }

    // 调用 bsp_sd_init 进行 SD 卡初始化及底层 SPI 速率配置
    uint8_t res = bsp_sd_init();

    if (res == 0) {
        return 0; // 初始化成功
    }

    return STA_NOINIT; // 初始化失败
}


/*-----------------------------------------------------------------------*/
/* 读取扇区 (Read Sector(s))                                              */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,      /* 物理盘符 */
    BYTE *buff,     /* 数据接收缓冲区 */
    LBA_t sector,   /* 起始扇区地址 (LBA) */
    UINT count      /* 读取的扇区数量 */
)
{
    if (pdrv != DEV_MMC || !count) {
        return RES_PARERR;
    }

    // 调用底层多扇区/单扇区读取接口
    uint8_t res = bsp_sd_read_disk(buff, (uint32_t)sector, (uint8_t)count);

    if (res == 0) {
        return RES_OK;
    }

    return RES_ERROR;
}


/*-----------------------------------------------------------------------*/
/* 写入扇区 (Write Sector(s))                                             */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,          /* 物理盘符 */
    const BYTE *buff,   /* 数据发送缓冲区 */
    LBA_t sector,       /* 起始扇区地址 (LBA) */
    UINT count          /* 写入的扇区数量 */
)
{
    if (pdrv != DEV_MMC || !count) {
        return RES_PARERR;
    }

    // 调用底层写磁盘接口
    uint8_t res = bsp_sd_write_disk((uint8_t *)buff, (uint32_t)sector, (uint8_t)count);

    if (res == 0) {
        return RES_OK;
    }

    return RES_ERROR;
}

#endif


/*-----------------------------------------------------------------------*/
/* 杂项控制功能 (Miscellaneous Functions)                                */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,      /* 物理盘符 */
    BYTE cmd,       /* 控制指令 */
    void *buff      /* 控制参数/数据输出指针 */
)
{
    if (pdrv != DEV_MMC) {
        return RES_PARERR;
    }

    DRESULT res = RES_ERROR;

    switch (cmd) {
    case CTRL_SYNC: 
        // 强制写入同步 (SPI SD 卡写入内部会在 block 操作中阻塞等待完成，直接返回 OK)
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT: 
        // 获取总扇区数 (f_mkfs 格式化与 f_getfree 获取剩余容量时必需)
        *(DWORD *)buff = bsp_sd_get_sector_count();
        res = RES_OK;
        break;

    case GET_SECTOR_SIZE: 
        // 获取扇区大小
        *(WORD *)buff = MSD_BLOCKSIZE; // 512 Bytes
        res = RES_OK;
        break;

    case GET_BLOCK_SIZE: 
        // 获取擦除块大小（按 1 个扇区对齐处理）
        *(DWORD *)buff = 1;
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
        break;
    }

    return res;
}


/*-----------------------------------------------------------------------*/
/* 获取时间戳 (Get Time Function for FatFs File Timestamps)               */
/*-----------------------------------------------------------------------*/

#if !FF_FS_READONLY && !FF_FS_NORTC
DWORD get_fattime (void)
{
    /* 如果项目中有 RTC 硬件，可替换为真实的年月日时分秒转换 */
    /* 格式：
       Bit 31:25 - 年份 offset from 1980 (0..127)
       Bit 24:21 - 月份 (1..12)
       Bit 20:16 - 日 (1..31)
       Bit 15:11 - 时 (0..23)
       Bit 10:5  - 分 (0..59)
       Bit 4:0   - 秒 / 2 (0..29)
    */
    return ((DWORD)(2026 - 1980) << 25)
         | ((DWORD)1 << 21)
         | ((DWORD)1 << 16)
         | ((DWORD)0 << 11)
         | ((DWORD)0 << 5)
         | ((DWORD)0 >> 1);
}
#endif