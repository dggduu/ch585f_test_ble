#ifndef __BSP_IO_EXT_H__
#define __BSP_IO_EXT_H__

#include "CH58x_common.h"
#include <stdbool.h>
#include <stdint.h>

/* PCA9539PW 寄存器地址 */
#define PCA9539_INPUT_PORT_0      0x00
#define PCA9539_INPUT_PORT_1      0x01
#define PCA9539_OUTPUT_PORT_0     0x02
#define PCA9539_OUTPUT_PORT_1     0x03
#define PCA9539_POLARITY_INV_0    0x04
#define PCA9539_POLARITY_INV_1    0x05
#define PCA9539_CONFIG_PORT_0     0x06
#define PCA9539_CONFIG_PORT_1     0x07

/* API 函数 */
void BSP_IO_EXT_Init(uint8_t devAddr);
uint8_t BSP_IO_EXT_WriteReg(uint8_t devAddr, uint8_t reg, uint8_t value);
uint8_t BSP_IO_EXT_ReadReg(uint8_t devAddr, uint8_t reg, uint8_t *value);
uint8_t BSP_IO_EXT_ReadInput(uint8_t devAddr, uint16_t *value);
uint8_t BSP_IO_EXT_WriteOutput(uint8_t devAddr, uint16_t value);
uint8_t BSP_IO_EXT_ConfigPort(uint8_t devAddr, uint8_t port, uint8_t config);
uint8_t BSP_IO_EXT_SetPinDirection(uint8_t devAddr, uint8_t port, uint8_t pin, uint8_t isOutput);
uint8_t BSP_IO_EXT_SetPinLevel(uint8_t devAddr, uint8_t pin, uint8_t level);
void BSP_IO_EXT_DeInit(void);
void BSP_IO_EXT_Scan(void);   // 直接调用 bsp_i2c_scan

#endif