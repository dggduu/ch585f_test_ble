#include "bsp_i2c.h"

#define I2C_TIMEOUT_COUNT  10000

/* 私有辅助函数：清除 ADDR 标志位 */
static inline void i2c_clear_addr_flag(void)
{
    (void)R16_I2C_STAR1;
    (void)R16_I2C_STAR2;
}

/* 初始化 I2C 外设 */
void bsp_i2c_init(void)
{
    // 如果硬件配置涉及引脚重映射，在此开启（如不需要可注释掉）
    // GPIOPinRemap(ENABLE, RB_PIN_I2C);

    // 配置引脚为带上拉输入模式（由硬件 I2C 控制引脚）
    GPIOB_ModeCfg(I2C_SCL_PIN | I2C_SDA_PIN, GPIO_ModeIN_PU);

    // 禁用 I2C 中断，完全采用阻塞轮询机制
    PFIC_DisableIRQ(I2C_IRQn);
    I2C_ITConfig(I2C_IT_BUF | I2C_IT_EVT | I2C_IT_ERR, DISABLE);

    // 初始化硬件 I2C 主机模式（100kHz 标准模式，可根据需要提升至 400000）
    I2C_Init(I2C_Mode_I2C, 400000, I2C_DutyCycle_16_9, I2C_Ack_Enable,
             I2C_AckAddr_7bit, 0x00);
}

/* 探测设备地址 */
uint8_t bsp_i2c_probe(uint8_t dev_addr)
{
    uint32_t timeout = I2C_TIMEOUT_COUNT;
    uint8_t ack_received = 0;

    while(I2C_GetFlagStatus(I2C_FLAG_BUSY) && --timeout);
    if(timeout == 0) return 1;

    I2C_GenerateSTART(ENABLE);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if(timeout == 0) {
        I2C_GenerateSTOP(ENABLE);
        return 1;
    }

    I2C_Send7bitAddress(dev_addr << 1, I2C_Direction_Transmitter);

    timeout = I2C_TIMEOUT_COUNT;
    while(--timeout)
    {
        if(I2C_GetFlagStatus(I2C_FLAG_ADDR) == SET)
        {
            ack_received = 1;
            break;
        }
        if(I2C_GetFlagStatus(I2C_FLAG_AF) == SET)
        {
            I2C_ClearFlag(I2C_FLAG_AF);
            ack_received = 0;
            break;
        }
    }

    I2C_GenerateSTOP(ENABLE);
    i2c_clear_addr_flag();

    return ack_received ? 0 : 1; // 0 表示设备在线 (ACK)
}

/* 扫描总线设备 */
void bsp_i2c_scan(void)
{
    PRINT("I2C Hardware Scanning...\r\n");
    uint8_t found = 0;
    for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
        if (bsp_i2c_probe(addr) == 0) {
            PRINT("Device found at 0x%02X\r\n", addr);
            found = 1;
        }
        mDelaymS(1);
    }
    if (!found) PRINT("No I2C device found!\r\n");
}

/* 向指定设备的寄存器写入多字节数据 */
uint8_t bsp_i2c_write_bytes(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint16_t len)
{
    uint32_t timeout = I2C_TIMEOUT_COUNT;

    // 1. 等待总线空闲
    while(I2C_GetFlagStatus(I2C_FLAG_BUSY) && --timeout);
    if(timeout == 0) return 1;

    // 2. 发送起始条件
    I2C_GenerateSTART(ENABLE);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if(timeout == 0) goto error;

    // 3. 发送从机写地址
    I2C_Send7bitAddress(dev_addr << 1, I2C_Direction_Transmitter);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --timeout) {
        if(I2C_GetFlagStatus(I2C_FLAG_AF) == SET) {
            I2C_ClearFlag(I2C_FLAG_AF);
            goto error;
        }
    }
    if(timeout == 0) goto error;

    // 4. 发送寄存器地址
    I2C_SendData(reg);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --timeout);
    if(timeout == 0) goto error;

    // 5. 循环发送数据
    for(uint16_t i = 0; i < len; i++)
    {
        I2C_SendData(data[i]);
        timeout = I2C_TIMEOUT_COUNT;
        while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --timeout);
        if(timeout == 0) goto error;
    }

    // 6. 发送停止条件
    I2C_GenerateSTOP(ENABLE);
    return 0;

error:
    I2C_GenerateSTOP(ENABLE);
    return 1;
}

/* 向指定寄存器写入单字节 */
uint8_t bsp_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data)
{
    return bsp_i2c_write_bytes(dev_addr, reg, &data, 1);
}

/* 从指定设备的寄存器读取多字节数据 */
uint8_t bsp_i2c_read_bytes(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    if(len == 0) return 1;
    uint32_t timeout = I2C_TIMEOUT_COUNT;

    // --- 第一阶段：写目标寄存器地址 ---
    while(I2C_GetFlagStatus(I2C_FLAG_BUSY) && --timeout);
    if(timeout == 0) return 1;

    I2C_GenerateSTART(ENABLE);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if(timeout == 0) goto error;

    I2C_Send7bitAddress(dev_addr << 1, I2C_Direction_Transmitter);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --timeout) {
        if(I2C_GetFlagStatus(I2C_FLAG_AF) == SET) {
            I2C_ClearFlag(I2C_FLAG_AF);
            goto error;
        }
    }
    if(timeout == 0) goto error;

    I2C_SendData(reg);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --timeout);
    if(timeout == 0) goto error;

    // --- 第二阶段：Repeated START 读取数据 ---
    I2C_GenerateSTART(ENABLE);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if(timeout == 0) goto error;

    I2C_Send7bitAddress(dev_addr << 1, I2C_Direction_Receiver);
    timeout = I2C_TIMEOUT_COUNT;
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && --timeout);
    if(timeout == 0) goto error;

    // 读取数据流控制
    for(uint16_t i = 0; i < len; i++)
    {
        if(i == len - 1)
        {
            // 最后一个字节：关闭 ACK，产生 STOP
            I2C_AcknowledgeConfig(DISABLE);
            I2C_GenerateSTOP(ENABLE);
        }
        else
        {
            I2C_AcknowledgeConfig(ENABLE);
        }

        timeout = I2C_TIMEOUT_COUNT;
        while(I2C_GetFlagStatus(I2C_FLAG_RXNE) == RESET && --timeout);
        if(timeout == 0) goto error;

        data[i] = I2C_ReceiveData();
    }

    // 恢复默认 ACK 使能，供后续传输使用
    I2C_AcknowledgeConfig(ENABLE);
    return 0;

error:
    I2C_AcknowledgeConfig(ENABLE);
    I2C_GenerateSTOP(ENABLE);
    return 1;
}

/* 从指定寄存器读取单字节 */
uint8_t bsp_i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data)
{
    return bsp_i2c_read_bytes(dev_addr, reg, data, 1);
}

/* 关闭 I2C 外设 */
void bsp_i2c_deinit(void)
{
    I2C_Cmd(DISABLE);
    GPIOB_ModeCfg(I2C_SCL_PIN | I2C_SDA_PIN, GPIO_ModeIN_PU);
}