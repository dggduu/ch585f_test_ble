#include "bsp_spi.h"
#include "CH58x_common.h"
#include "CH58x_spi.h"

void bsp_spi_init(void) {
    GPIOA_ModeCfg(GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PU);
    SPI0_MasterDefInit();
    SPI0_DataMode(Mode0_HighBitINFront);
    
}

void bsp_spi_set_speed(uint32_t hz) {
    if (hz == 0) hz = 1;
    uint32_t div = FREQ_SYS / (2 * hz);   // 使用系统时钟
    if (div < 1) div = 1;
    if (div > 255) div = 255;
    SPI0_CLKCfg((uint8_t)div);
}

// 全双工单字节收发（发送 data，返回接收值）
uint8_t bsp_spi_transfer_byte(uint8_t data) {
    SPI0_MasterSendByte(data);   // 发送并等待完成
    return R8_SPI0_FIFO;         // 直接读取 FIFO 获取接收到的数据
}

// 批量传输（支持只发、只收、同时收发）
void bsp_spi_transfer(uint8_t *tx, uint8_t *rx, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t d = tx ? tx[i] : 0xFF;
        uint8_t r = bsp_spi_transfer_byte(d);
        if (rx) rx[i] = r;
    }
}

// 仅供发送（LCD 专用，更高效）
void bsp_spi_send_bulk(uint8_t *data, uint16_t len) {
    SPI0_MasterTrans(data, len);
}