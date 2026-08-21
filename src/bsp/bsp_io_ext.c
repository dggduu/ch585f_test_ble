#include "bsp_io_ext.h"
#include "bsp_i2c.h"
#include <string.h>   // for memset

/* 静态缓存：保存输出端口值（PORT0 / PORT1）和配置端口值 */
static uint8_t s_output_cache[2];
static uint8_t s_config_cache[2];

/* 内部辅助函数：写入单个寄存器（不更新缓存） */
static uint8_t write_reg(uint8_t devAddr, uint8_t reg, uint8_t value) {
    return bsp_i2c_write_reg(devAddr, reg, value);
}

/* 内部辅助函数：读取单个寄存器 */
static uint8_t read_reg(uint8_t devAddr, uint8_t reg, uint8_t *value) {
    return bsp_i2c_read_reg(devAddr, reg, value);
}

/* 初始化：读取当前硬件状态以填充缓存（失败则使用安全默认值） */
void BSP_IO_EXT_Init(uint8_t devAddr) {
    bsp_i2c_init();

    // 尝试读取当前输出值，若失败则默认 0x00（输出低）
    if (read_reg(devAddr, PCA9539_OUTPUT_PORT_0, &s_output_cache[0]) != 0) {
        s_output_cache[0] = 0x00;
    }
    if (read_reg(devAddr, PCA9539_OUTPUT_PORT_1, &s_output_cache[1]) != 0) {
        s_output_cache[1] = 0x00;
    }

    // 尝试读取当前方向配置，若失败则默认 0xFF（全部输入）
    if (read_reg(devAddr, PCA9539_CONFIG_PORT_0, &s_config_cache[0]) != 0) {
        s_config_cache[0] = 0xFF;
    }
    if (read_reg(devAddr, PCA9539_CONFIG_PORT_1, &s_config_cache[1]) != 0) {
        s_config_cache[1] = 0xFF;
    }
}

/* 写寄存器（直接写入，不读） */
uint8_t BSP_IO_EXT_WriteReg(uint8_t devAddr, uint8_t reg, uint8_t value) {
    return write_reg(devAddr, reg, value);
}

/* 读寄存器 */
uint8_t BSP_IO_EXT_ReadReg(uint8_t devAddr, uint8_t reg, uint8_t *value) {
    return read_reg(devAddr, reg, value);
}

/* 读取两个输入端口（无缓存，直接读取） */
uint8_t BSP_IO_EXT_ReadInput(uint8_t devAddr, uint16_t *value) {
    uint8_t port0, port1;
    if (read_reg(devAddr, PCA9539_INPUT_PORT_0, &port0)) return 1;
    if (read_reg(devAddr, PCA9539_INPUT_PORT_1, &port1)) return 1;
    *value = ((uint16_t)port1 << 8) | port0;
    return 0;
}

/* 设置整个输出端口（更新缓存 + 写入硬件） */
uint8_t BSP_IO_EXT_WriteOutput(uint8_t devAddr, uint16_t value) {
    s_output_cache[0] = (uint8_t)value;
    s_output_cache[1] = (uint8_t)(value >> 8);
    if (write_reg(devAddr, PCA9539_OUTPUT_PORT_0, s_output_cache[0])) return 1;
    if (write_reg(devAddr, PCA9539_OUTPUT_PORT_1, s_output_cache[1])) return 1;
    return 0;
}

/* 配置整个端口方向（更新缓存 + 写入硬件） */
uint8_t BSP_IO_EXT_ConfigPort(uint8_t devAddr, uint8_t port, uint8_t config) {
    if (port == 0) {
        s_config_cache[0] = config;
        return write_reg(devAddr, PCA9539_CONFIG_PORT_0, config);
    } else {
        s_config_cache[1] = config;
        return write_reg(devAddr, PCA9539_CONFIG_PORT_1, config);
    }
}

/* 设置单个引脚方向（使用缓存，无需读取） */
uint8_t BSP_IO_EXT_SetPinDirection(uint8_t devAddr, uint8_t port, uint8_t pin, uint8_t isOutput) {
    if (pin > 7) return 1; // 无效引脚

    uint8_t *cache = (port == 0) ? &s_config_cache[0] : &s_config_cache[1];
    if (isOutput)
        *cache &= ~(1 << pin);
    else
        *cache |= (1 << pin);

    uint8_t reg = (port == 0) ? PCA9539_CONFIG_PORT_0 : PCA9539_CONFIG_PORT_1;
    return write_reg(devAddr, reg, *cache);
}

/* 设置单个引脚电平（使用输出缓存，无需读取） */
uint8_t BSP_IO_EXT_SetPinLevel(uint8_t devAddr, uint8_t pin, uint8_t level)
{
    uint8_t port = pin >> 3;          // 0 或 1
    uint8_t bit  = pin & 0x07;        // 0~7
    uint8_t *cache = (port == 0) ? &s_output_cache[0] : &s_output_cache[1];

    uint8_t old_val = *cache;         // 当前缓存值
    uint8_t new_val = old_val;

    if (level)
        new_val |=  (1 << bit);
    else
        new_val &= ~(1 << bit);

    // 如果整个端口的输出值没有变化，直接返回成功，不写硬件
    if (old_val == new_val)
        return 0;

    // 更新缓存并写入 PCA9539
    *cache = new_val;
    uint8_t reg = (port == 0) ? PCA9539_OUTPUT_PORT_0 : PCA9539_OUTPUT_PORT_1;
    return write_reg(devAddr, reg, new_val);
}

/* 反初始化 */
void BSP_IO_EXT_DeInit(void) {
    bsp_i2c_deinit();
}

/* I2C 总线扫描 */
void BSP_IO_EXT_Scan(void) {
    bsp_i2c_scan();
}