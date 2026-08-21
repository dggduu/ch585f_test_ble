#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "CH58x_common.h"

// 引脚定义：与硬件连接保持一致（PB12-SDA, PB13-SCL）
#define I2C_SCL_PIN   GPIO_Pin_13
#define I2C_SDA_PIN   GPIO_Pin_12

// API 声明
void    bsp_i2c_init(void);
void    bsp_i2c_deinit(void);
uint8_t bsp_i2c_probe(uint8_t dev_addr);
void    bsp_i2c_scan(void);

uint8_t bsp_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data);
uint8_t bsp_i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data);
uint8_t bsp_i2c_write_bytes(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint16_t len);
uint8_t bsp_i2c_read_bytes(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);

#endif /* __BSP_I2C_H */