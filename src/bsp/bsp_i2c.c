#include "bsp_i2c.h"
#include "CH58x_common.h"

/* 引脚定义：SCL=PB13, SDA=PB12 */
#define I2C_SCL_PIN   GPIO_Pin_13
#define I2C_SDA_PIN   GPIO_Pin_12

/* 方向宏：切换为输入（释放总线）或输出（拉低） */
#define SCL_OUT()     GPIOB_ModeCfg(I2C_SCL_PIN, GPIO_ModeOut_PP_5mA)
#define SDA_OUT()     GPIOB_ModeCfg(I2C_SDA_PIN, GPIO_ModeOut_PP_5mA)
#define SDA_IN()      GPIOB_ModeCfg(I2C_SDA_PIN, GPIO_ModeIN_PU)

/* 电平操作 */
#define SCL_HIGH()    GPIOB_SetBits(I2C_SCL_PIN)
#define SCL_LOW()     GPIOB_ResetBits(I2C_SCL_PIN)
#define SDA_HIGH()    GPIOB_SetBits(I2C_SDA_PIN)
#define SDA_LOW()     GPIOB_ResetBits(I2C_SDA_PIN)
#define SDA_READ()    (GPIOB_ReadPortPin(I2C_SDA_PIN) != 0)

/* 延时函数（约 5?s @ 32MHz，可调整） */
static void I2C_Delay(void) {
    for(volatile int i = 0; i < 30; i++) __nop();
}

/* ---------- 起始/停止 ---------- */
static void I2C_Start(void) {
    SDA_OUT();
    SCL_OUT();
    SDA_HIGH();
    SCL_HIGH();
    I2C_Delay();
    SDA_LOW();
    I2C_Delay();
    SCL_LOW();
    I2C_Delay();
}

static void I2C_Stop(void) {
    SDA_OUT();
    SCL_LOW();
    SDA_LOW();
    I2C_Delay();
    SCL_HIGH();
    I2C_Delay();
    SDA_HIGH();
    I2C_Delay();
}

/* ---------- 写一个字节，返回 ACK 位（0=ACK, 1=NACK） ---------- */
static uint8_t I2C_WriteByte(uint8_t data) {
    SDA_OUT();
    for(uint8_t i = 0; i < 8; i++) {
        if(data & 0x80) SDA_HIGH();
        else SDA_LOW();
        data <<= 1;
        I2C_Delay();
        SCL_HIGH();
        I2C_Delay();
        SCL_LOW();
        I2C_Delay();
    }
    // 释放 SDA，读取 ACK
    SDA_IN();   // 切换为输入
    SCL_HIGH();
    I2C_Delay();
    uint8_t ack = SDA_READ();   // 0=ACK, 1=NACK
    SCL_LOW();
    I2C_Delay();
    SDA_OUT();  // 恢复输出
    return ack;
}

/* ---------- 读一个字节，并发送 ACK(0) 或 NACK(1) ---------- */
static uint8_t I2C_ReadByte(uint8_t ack) {
    uint8_t data = 0;
    SDA_IN();   // 释放总线
    for(uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        SCL_HIGH();
        I2C_Delay();
        if(SDA_READ()) data |= 0x01;
        SCL_LOW();
        I2C_Delay();
    }
    // 发送 ACK 或 NACK
    SDA_OUT();
    if(ack) SDA_HIGH();   // NACK
    else SDA_LOW();       // ACK
    SCL_HIGH();
    I2C_Delay();
    SCL_LOW();
    I2C_Delay();
    SDA_IN();   // 释放
    return data;
}

/* ---------- 对外 API ---------- */

/* 初始化：配置 GPIO 为推挽输出，初始高电平 */
void bsp_i2c_init(void) {
    GPIOB_ModeCfg(I2C_SCL_PIN | I2C_SDA_PIN, GPIO_ModeOut_PP_5mA);
    SCL_HIGH();
    SDA_HIGH();
    I2C_Delay();
}

/* 写寄存器（软件模拟） */
uint8_t bsp_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data) {
    I2C_Start();
    if(I2C_WriteByte((dev_addr << 1) | 0)) { I2C_Stop(); return 1; } // 写地址
    if(I2C_WriteByte(reg)) { I2C_Stop(); return 1; }
    if(I2C_WriteByte(data)) { I2C_Stop(); return 1; }
    I2C_Stop();
    return 0;
}

/* 读寄存器（软件模拟） */
uint8_t bsp_i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data) {
    I2C_Start();
    if(I2C_WriteByte((dev_addr << 1) | 0)) { I2C_Stop(); return 1; } // 写地址
    if(I2C_WriteByte(reg)) { I2C_Stop(); return 1; }
    I2C_Start();  // 重新起始
    if(I2C_WriteByte((dev_addr << 1) | 1)) { I2C_Stop(); return 1; } // 读地址
    *data = I2C_ReadByte(1);   // 读一个字节，发 NACK
    I2C_Stop();
    return 0;
}

/* 探测设备：发送地址+写位，检测 ACK */
uint8_t bsp_i2c_probe(uint8_t dev_addr) {
    I2C_Start();
    uint8_t ack = I2C_WriteByte((dev_addr << 1) | 0);
    I2C_Stop();
    return ack;   // 0 表示 ACK（有设备），1 表示无响应
}

/* 扫描总线（与硬件版本完全兼容） */
void bsp_i2c_scan(void) {
    uint8_t addr;
    uint8_t found = 0;
    PRINT("I2C Scanning...\r\n");
    for(addr = 0x01; addr < 0x80; addr++) {
        if(bsp_i2c_probe(addr) == 0) {
            PRINT("Device found at 0x%02X\r\n", addr);
            found = 1;
        }
        I2C_Delay(); // 避免总线冲突
    }
    if(!found) PRINT("No I2C device found!\r\n");
}

/* 关闭 I2C（释放引脚为输入） */
void bsp_i2c_deinit(void) {
    GPIOB_ModeCfg(I2C_SCL_PIN | I2C_SDA_PIN, GPIO_ModeIN_PU);
}