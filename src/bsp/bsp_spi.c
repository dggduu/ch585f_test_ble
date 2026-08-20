#include "bsp_spi.h"
#include "CH58x_common.h"

/* 引脚定义 */
#define SPI_SCK_PIN   GPIO_Pin_13
#define SPI_MOSI_PIN  GPIO_Pin_14
#define SPI_MISO_PIN  GPIO_Pin_15

/* GPIO 操作宏 */
#define SPI_SCK_HIGH()   GPIOA_SetBits(SPI_SCK_PIN)
#define SPI_SCK_LOW()    GPIOA_ResetBits(SPI_SCK_PIN)
#define SPI_MOSI_HIGH()  GPIOA_SetBits(SPI_MOSI_PIN)
#define SPI_MOSI_LOW()   GPIOA_ResetBits(SPI_MOSI_PIN)
#define SPI_MISO_READ()  (GPIOA_ReadPortPin(SPI_MISO_PIN) ? 1 : 0)

/* 静态变量：半周期延时（微秒） */
static uint32_t s_half_delay_us = 1;   // 默认最小值

/* 初始化：配置 GPIO，并使 SCK 初始为低 */
void bsp_spi_init(void) {
    // 配置 SCK 和 MOSI 为推挽输出（5mA 驱动）
    GPIOA_ModeCfg(SPI_SCK_PIN | SPI_MOSI_PIN, GPIO_ModeOut_PP_5mA);
    // 配置 MISO 为输入上拉
    GPIOA_ModeCfg(SPI_MISO_PIN, GPIO_ModeIN_PU);

    // 空闲时 SCK 为低（模式 0）
    SPI_SCK_LOW();
    // 设置一个默认速度（例如 400kHz）
    bsp_spi_set_speed(400000);
}

/* 设置 SPI 时钟频率（软件模拟通过延时实现） */
void bsp_spi_set_speed(uint32_t hz) {
    if (hz == 0) hz = 1;   // 避免除零
    // 计算半周期微秒数 = 1/(2*hz) 秒 = 500000/hz 微秒
    uint32_t half_us = 500000 / hz;
    if (half_us < 1) half_us = 1;      // 最小 1us（对应最高约 500kHz）
    if (half_us > 1000) half_us = 1000; // 最大 1ms（超低速，可调整）
    s_half_delay_us = half_us;
}

/* 单字节全双工收发 */
uint8_t bsp_spi_transfer_byte(uint8_t tx_data) {
    uint8_t rx_data = 0;

    // 从最高位（bit7）开始发送
    for (int i = 7; i >= 0; i--) {
        // 1. 设置 MOSI（在 SCK 下降沿之后改变数据，模式0）
        if (tx_data & (1 << i))
            SPI_MOSI_HIGH();
        else
            SPI_MOSI_LOW();

        // 2. SCK 上升沿（主设备采样 MISO）
        SPI_SCK_HIGH();
        DelayUs(s_half_delay_us);   // 保持高电平

        // 3. 读取 MISO 并存入接收字节
        if (SPI_MISO_READ())
            rx_data |= (1 << i);

        // 4. SCK 下降沿（从设备采样 MOSI）
        SPI_SCK_LOW();
        DelayUs(s_half_delay_us);   // 保持低电平
    }

    return rx_data;
}

/* 批量传输（支持只发、只收、同时收发） */
void bsp_spi_transfer(uint8_t *tx, uint8_t *rx, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t d = tx ? tx[i] : 0xFF;
        uint8_t r = bsp_spi_transfer_byte(d);
        if (rx) rx[i] = r;
    }
}

/* 纯发送（适用于 LCD 等，复用 transfer_byte） */
void bsp_spi_send_bulk(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        bsp_spi_transfer_byte(data[i]);
    }
}