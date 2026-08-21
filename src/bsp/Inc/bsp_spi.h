#ifndef __BSP_SPI_H__
#define __BSP_SPI_H__

#include <stdint.h>

void bsp_spi_init(void);
void bsp_spi_set_speed(uint32_t hz);   // 设置时钟频率（约）
uint8_t bsp_spi_transfer_byte(uint8_t data);
void bsp_spi_transfer(uint8_t *tx, uint8_t *rx, uint16_t len);

void bsp_spi_send_bulk(uint8_t *data, uint16_t len);   // 批量发送（只发不收，LCD 专用）

#endif