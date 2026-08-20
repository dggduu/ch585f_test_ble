#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#include "CH58x_common.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* I2C 传输方向 */
#define BSP_I2C_WRITE  0
#define BSP_I2C_READ   1

/* 初始化 I2C 主机（SCL=PB13, SDA=PB12，100kHz） */
void bsp_i2c_init(void);

/* 写一个字节到从机寄存器 */
uint8_t bsp_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data);

/* 从从机寄存器读一个字节 */
uint8_t bsp_i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data);

/* 发送起始+地址，检测是否有 ACK（用于扫描） */
uint8_t bsp_i2c_probe(uint8_t dev_addr);

/* 扫描总线，打印所有响应的设备地址（用于调试） */
void bsp_i2c_scan(void);

/* 关闭 I2C 外设 */
void bsp_i2c_deinit(void);

#ifdef __cplusplus
}
#endif

#endif