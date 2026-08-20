#include "bsp_io_ext.h"
#include "bsp_i2c.h"

void BSP_IO_EXT_Init(uint8_t devAddr) {
    bsp_i2c_init();
    // 可做一次通信测试，但 bsp_i2c_init 已包含
}

uint8_t BSP_IO_EXT_WriteReg(uint8_t devAddr, uint8_t reg, uint8_t value) {
    return bsp_i2c_write_reg(devAddr, reg, value);
}

uint8_t BSP_IO_EXT_ReadReg(uint8_t devAddr, uint8_t reg, uint8_t *value) {
    return bsp_i2c_read_reg(devAddr, reg, value);
}

uint8_t BSP_IO_EXT_ReadInput(uint8_t devAddr, uint16_t *value) {
    uint8_t port0, port1;
    if(bsp_i2c_read_reg(devAddr, PCA9539_INPUT_PORT_0, &port0)) return 1;
    if(bsp_i2c_read_reg(devAddr, PCA9539_INPUT_PORT_1, &port1)) return 1;
    *value = ((uint16_t)port1 << 8) | port0;
    return 0;
}

uint8_t BSP_IO_EXT_WriteOutput(uint8_t devAddr, uint16_t value) {
    if(bsp_i2c_write_reg(devAddr, PCA9539_OUTPUT_PORT_0, (uint8_t)value)) return 1;
    if(bsp_i2c_write_reg(devAddr, PCA9539_OUTPUT_PORT_1, (uint8_t)(value >> 8))) return 1;
    return 0;
}

uint8_t BSP_IO_EXT_ConfigPort(uint8_t devAddr, uint8_t port, uint8_t config) {
    uint8_t reg = (port == 0) ? PCA9539_CONFIG_PORT_0 : PCA9539_CONFIG_PORT_1;
    return bsp_i2c_write_reg(devAddr, reg, config);
}

uint8_t BSP_IO_EXT_SetPinDirection(uint8_t devAddr, uint8_t port, uint8_t pin, uint8_t isOutput) {
    uint8_t reg = (port == 0) ? PCA9539_CONFIG_PORT_0 : PCA9539_CONFIG_PORT_1;
    uint8_t config;
    if(bsp_i2c_read_reg(devAddr, reg, &config)) return 1;
    if(isOutput)
        config &= ~(1 << pin);
    else
        config |= (1 << pin);
    return bsp_i2c_write_reg(devAddr, reg, config);
}

uint8_t BSP_IO_EXT_SetPinLevel(uint8_t devAddr, uint8_t pin, uint8_t level) {
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 0x07;
    uint8_t reg  = (port == 0) ? PCA9539_OUTPUT_PORT_0 : PCA9539_OUTPUT_PORT_1;
    uint8_t current;
    if (BSP_IO_EXT_ReadReg(devAddr, reg, &current)) return 1;   // 读取当前输出值
    if (level) current |= (1 << bit);
    else       current &= ~(1 << bit);
    return BSP_IO_EXT_WriteReg(devAddr, reg, current);
}

void BSP_IO_EXT_DeInit(void) {
    bsp_i2c_deinit();
}

void BSP_IO_EXT_Scan(void) {
    bsp_i2c_scan();
}