#include "bsp_sd.h"
#include "bsp_spi.h"
#include "bsp_pin_defs.h"
#include "CH58x_common.h"
#include <stdio.h>

uint8_t SD_TYPE = 0x00;
MSD_CARDINFO SD0_CardInfo;

/**
 * @brief SD卡片选控制（带 SPI 总线防冲突逻辑）
 * @param select 1: 选中SD卡(CS拉低)  0: 取消选中(CS拉高)
 */
void bsp_sd_cs(uint8_t select) {
    if (select) {
        // 确保屏幕片选禁用，释放 SPI 总线
        SCREEN_CS_SET();
        SD_CS_CLR();
    } else {
        SD_CS_SET();
        // 释放片选后提供 8 个 Dummy 时钟，使 SD 卡内部逻辑释放 MISO 总线
        bsp_spi_transfer_byte(DUMMY_BYTE);
    }
}

/**
 * @brief 向 SD 卡发送命令
 */
static uint8_t bsp_sd_send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc) {
    uint8_t r1;
    uint8_t retry = 200;

    bsp_sd_cs(0);
    mDelaymS(1);
    bsp_sd_cs(1);

    // 等待 SD 卡准备就绪 (MISO拉高)
    do {
        r1 = bsp_spi_transfer_byte(DUMMY_BYTE);
    } while (r1 != 0xFF && --retry);

    if (retry == 0) {
        bsp_sd_cs(0);
        return 0xFF;
    }

    // 发送命令包
    bsp_spi_transfer_byte(cmd | 0x40);
    bsp_spi_transfer_byte((uint8_t)(arg >> 24));
    bsp_spi_transfer_byte((uint8_t)(arg >> 16));
    bsp_spi_transfer_byte((uint8_t)(arg >> 8));
    bsp_spi_transfer_byte((uint8_t)arg);
    bsp_spi_transfer_byte(crc);

    if (cmd == CMD12) {
        bsp_spi_transfer_byte(DUMMY_BYTE);
    }

    // 等待响应 (最高位为 0 表示有效响应)
    retry = 200;
    do {
        r1 = bsp_spi_transfer_byte(DUMMY_BYTE);
    } while ((r1 & 0x80) && --retry);

    return r1;
}

/**
 * @brief SD 卡初始化
 */
uint8_t bsp_sd_init(void) {
    uint8_t r1;
    uint8_t buff[4] = {0};
    uint16_t retry;
    uint8_t i;

    // 适配低速传输以完成卡初始化 (约 200-400kHz)
    bsp_spi_set_speed(300000);

    bsp_sd_cs(0);
    // 发送至少 74 个 Dummy 脉冲使卡进入 SPI 模式
    for (retry = 0; retry < 10; retry++) {
        bsp_spi_transfer_byte(DUMMY_BYTE);
    }

    // CMD0 复位进入 IDLE 状态
    retry = 100;
    do {
        r1 = bsp_sd_send_cmd(CMD0, 0, 0x95);
    } while (r1 != MSD_IN_IDLE_STATE && --retry);

    if (retry == 0) {
        bsp_sd_cs(0);
        return 1; // 复位超时
    }

    SD_TYPE = 0;
    // CMD8 验证接口条件 (检查是否支持 V2.0)
    r1 = bsp_sd_send_cmd(CMD8, 0x1AA, 0x87);
    if (r1 == MSD_IN_IDLE_STATE) {
        // 读取 R7 响应后 4 字节
        for (i = 0; i < 4; i++) buff[i] = bsp_spi_transfer_byte(DUMMY_BYTE);
        
        if (buff[2] == 0x01 && buff[3] == 0xAA) { // 确认支持 2.7V - 3.6V
            retry = 0xFFFE;
            do {
                bsp_sd_send_cmd(CMD55, 0, 0x01);
                r1 = bsp_sd_send_cmd(CMD41, 0x40000000, 0x01);
            } while (r1 && --retry);

            if (retry && bsp_sd_send_cmd(CMD58, 0, 0x01) == 0) {
                for (i = 0; i < 4; i++) buff[i] = bsp_spi_transfer_byte(DUMMY_BYTE);
                if (buff[0] & 0x40) {
                    SD_TYPE = V2HC; // 高容量卡 (SDHC/SDXC)
                } else {
                    SD_TYPE = V2;   // 标准容量卡 (SDSC)
                }
            }
        }
    } else { // SD V1.x 或 MMC 卡
        bsp_sd_send_cmd(CMD55, 0, 0x01);
        r1 = bsp_sd_send_cmd(CMD41, 0, 0x01);
        if (r1 <= 1) {
            SD_TYPE = V1;
            retry = 0xFFFE;
            do {
                bsp_sd_send_cmd(CMD55, 0, 0x01);
                r1 = bsp_sd_send_cmd(CMD41, 0, 0x01);
            } while (r1 && --retry);
        } else {
            SD_TYPE = MMC;
            retry = 0xFFFE;
            do {
                r1 = bsp_sd_send_cmd(CMD1, 0, 0x01);
            } while (r1 && --retry);
        }

        if (retry == 0 || bsp_sd_send_cmd(CMD16, MSD_BLOCKSIZE, 0x01) != 0) {
            SD_TYPE = ERR;
        }
    }

    bsp_sd_cs(0);

    // 初始化完成后，将 SPI 切换为高速模式 (如 20MHz/24MHz)
    bsp_spi_set_speed(20000000);

    return (SD_TYPE != ERR) ? 0 : 1;
}

/**
 * @brief 从 SD 卡读取特定长度数据包
 */
uint8_t bsp_sd_receive_data(uint8_t *data, uint16_t len) {
    uint8_t r1;
    uint16_t retry = 2000;

    bsp_sd_cs(1);
    // 等待起始 Token 0xFE
    do {
        r1 = bsp_spi_transfer_byte(DUMMY_BYTE);
    } while (r1 != 0xFE && --retry);

    if (r1 != 0xFE) {
        bsp_sd_cs(0);
        return 1;
    }

    // 接收数据主体
    bsp_spi_transfer(NULL, data, len);

    // 忽略 2 字节 CRC
    bsp_spi_transfer_byte(DUMMY_BYTE);
    bsp_spi_transfer_byte(DUMMY_BYTE);

    return 0;
}

/**
 * @brief 写入一个 512 字节的数据块
 */
uint8_t bsp_sd_send_block(uint8_t *buf, uint8_t cmd) {
    uint8_t r1;
    uint16_t retry = 2000;

    do {
        r1 = bsp_spi_transfer_byte(DUMMY_BYTE);
    } while (r1 != 0xFF && --retry);

    bsp_spi_transfer_byte(cmd);

    if (cmd != 0xFD) { // 不是 Stop Token
        // 优先尝试使用 CH585 的 SPI DMA 发送（需确保地址 4 字节对齐）
        if (((uint32_t)buf & 0x03) == 0) {
            bsp_spi_send_bulk(buf, MSD_BLOCKSIZE);
        } else {
            bsp_spi_transfer(buf, NULL, MSD_BLOCKSIZE);
        }

        // 假 CRC
        bsp_spi_transfer_byte(DUMMY_BYTE);
        bsp_spi_transfer_byte(DUMMY_BYTE);

        // 接收数据响应令牌 (Data Response)
        r1 = bsp_spi_transfer_byte(DUMMY_BYTE);
        if ((r1 & 0x1F) != 0x05) { // 检查响应状态是否为 ACCEPTED (010b)
            return 2;
        }

        // 等待卡内部写入完成 (Busy Wait)
        retry = 0xFFFF;
        while (bsp_spi_transfer_byte(DUMMY_BYTE) == 0x00 && --retry);
        if (retry == 0) return 3;
    }

    return 0;
}

uint8_t bsp_sd_get_cid(uint8_t *cid_data) {
    uint8_t r1 = bsp_sd_send_cmd(CMD10, 0, 0x01);
    if (r1 == 0x00) {
        r1 = bsp_sd_receive_data(cid_data, 16);
    }
    bsp_sd_cs(0);
    return r1 ? 1 : 0;
}

uint8_t bsp_sd_get_csd(uint8_t *csd_data) {
    uint8_t r1 = bsp_sd_send_cmd(CMD9, 0, 0x01);
    if (r1 == 0x00) {
        r1 = bsp_sd_receive_data(csd_data, 16);
    }
    bsp_sd_cs(0);
    return r1 ? 1 : 0;
}

uint32_t bsp_sd_get_sector_count(void) {
    uint8_t csd[16];
    uint32_t capacity;
    uint8_t n;
    uint16_t csize;

    if (bsp_sd_get_csd(csd) != 0) return 0;

    if ((csd[0] & 0xC0) == 0x40) { // CSD V2.0 (SDHC/SDXC)
        csize = csd[9] + ((uint16_t)csd[8] << 8) + 1;
        capacity = (uint32_t)csize << 10;
    } else { // CSD V1.0 (SDSC)
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
        capacity = (uint32_t)csize << (n - 9);
    }
    return capacity;
}

int bsp_sd_get_card_info(PMSD_CARDINFO SD0_CardInfo) {
    uint8_t r1;
    uint8_t CSD_Tab[16];
    uint8_t CID_Tab[16];

    r1 = bsp_sd_send_cmd(CMD9, 0, 0xFF);
    if (r1 != 0x00) return r1;
    if (bsp_sd_receive_data(CSD_Tab, 16)) return 1;

    r1 = bsp_sd_send_cmd(CMD10, 0, 0xFF);
    if (r1 != 0x00) return r1;
    if (bsp_sd_receive_data(CID_Tab, 16)) return 2;

    bsp_sd_cs(0);

    SD0_CardInfo->CSD.CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
    SD0_CardInfo->CardType = SD_TYPE;
    
    if (SD0_CardInfo->CardType == V2HC) {
        SD0_CardInfo->CSD.DeviceSize = (uint16_t)(CSD_Tab[8]) * 256 + CSD_Tab[9];
        SD0_CardInfo->Capacity = (uint32_t)(SD0_CardInfo->CSD.DeviceSize + 1) * 512 * 1024;
    } else {
        SD0_CardInfo->Capacity = bsp_sd_get_sector_count() * MSD_BLOCKSIZE;
    }

    SD0_CardInfo->BlockSize = MSD_BLOCKSIZE;
    return 0;
}

uint8_t bsp_sd_write_disk(uint8_t *buf, uint32_t sector, uint8_t cnt) {
    uint8_t r1;
    if (SD_TYPE != V2HC) sector *= MSD_BLOCKSIZE;

    if (cnt == 1) {
        r1 = bsp_sd_send_cmd(CMD24, sector, 0x01);
        if (r1 == 0) {
            r1 = bsp_sd_send_block(buf, 0xFE);
        }
    } else {
        if (SD_TYPE != MMC) {
            bsp_sd_send_cmd(CMD55, 0, 0x01);
            bsp_sd_send_cmd(CMD23, cnt, 0x01);
        }
        r1 = bsp_sd_send_cmd(CMD25, sector, 0x01);
        if (r1 == 0) {
            do {
                r1 = bsp_sd_send_block(buf, 0xFC);
                buf += MSD_BLOCKSIZE;
            } while (--cnt && r1 == 0);
            r1 = bsp_sd_send_block(NULL, 0xFD); // Stop token
        }
    }
    bsp_sd_cs(0);
    return r1;
}

uint8_t bsp_sd_read_disk(uint8_t *buf, uint32_t sector, uint8_t cnt) {
    uint8_t r1;
    if (SD_TYPE != V2HC) sector <<= 9;

    if (cnt == 1) {
        r1 = bsp_sd_send_cmd(CMD17, sector, 0x01);
        if (r1 == 0) {
            r1 = bsp_sd_receive_data(buf, MSD_BLOCKSIZE);
        }
    } else {
        r1 = bsp_sd_send_cmd(CMD18, sector, 0x01);
        if (r1 == 0) {
            do {
                r1 = bsp_sd_receive_data(buf, MSD_BLOCKSIZE);
                buf += MSD_BLOCKSIZE;
            } while (--cnt && r1 == 0);
            bsp_sd_send_cmd(CMD12, 0, 0x01);
        }
    }
    bsp_sd_cs(0);
    return r1;
}

void bsp_sd_get_capacity(void) {
    FATFS FS;
    FATFS *fs;
    DWORD fre_clust, AvailableSize, UsedSize, TotalSpace;
    uint8_t res;

    res = bsp_sd_init();
    if (res != 0) {
        printf("SD卡初始化失败!\r\n");
        return;
    }

    res = f_mount(&FS, "0:", 1);
    if (res != FR_OK) {
        printf("文件系统挂载失败 (%d)\r\n", res);
        return;
    }

    res = f_getfree("0:", &fre_clust, &fs);
    if (res == FR_OK) {
        TotalSpace = (uint32_t)(((fs->n_fatent - 2) * fs->csize) / 2 / 1024);
        AvailableSize = (uint32_t)((fre_clust * fs->csize) / 2 / 1024);
        UsedSize = TotalSpace - AvailableSize;
        printf("\r\n%lu MB 总空间.\r\n%lu MB 可用.\r\n%lu MB 已用.\r\n", TotalSpace, AvailableSize, UsedSize);
    } else {
        printf("获取容量失败 (%d)\r\n", res);
    }
    f_mount(NULL, "0:", 1);
}

void bsp_sd_write_file(char filename[], BYTE write_buff[], uint8_t bufSize) {
    FATFS fs;
    FIL file;
    uint8_t res = 0;
    UINT bw;

    res = bsp_sd_init();
    if (res != 0) {
        printf("SD卡初始化失败!\r\n");
        return;
    }

    res = f_mount(&fs, "0:", 1);
    if (res == FR_NO_FILESYSTEM) {
        printf("无文件系统，开始格式化...\r\n");
        
        // 适配新版 FatFs (R0.13+) f_mkfs 参数
        MKFS_PARM opt = {0};
        BYTE work_buf[FF_MAX_SS];
        res = f_mkfs("0:", &opt, work_buf, sizeof(work_buf));
        
        if (res == FR_OK) {
            f_mount(NULL, "0:", 1);
            f_mount(&fs, "0:", 1);
        } else {
            printf("格式化失败 (%d)!\r\n", res);
            return;
        }
    } else if (res != FR_OK) {
        printf("挂载失败!\r\n");
        return;
    }

    res = f_open(&file, filename, FA_OPEN_ALWAYS | FA_WRITE);
    if (res == FR_OK) {
        f_lseek(&file, f_size(&file)); // 追加写入
        res = f_write(&file, write_buff, bufSize, &bw);
        if (res == FR_OK) {
            printf("写入成功，实际写入 %u 字节\r\n", bw);
        } else {
            printf("写入错误 (%d)\r\n", res);
        }
        f_close(&file);
    } else {
        printf("打开/创建文件失败 (%d)\r\n", res);
    }

    f_mount(NULL, "0:", 1);
}