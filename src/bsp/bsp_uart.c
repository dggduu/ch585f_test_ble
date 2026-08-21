#include "bsp_uart.h"

/* 当前波特率记录（用于查询） */
static uint32_t g_baudrate = BSP_UART_DEFAULT_BAUDRATE;

/*********************************************************************
 * @brief   初始化UART0，使用默认引脚 PB4 (RX) 和 PB7 (TX)
 * @param   baudrate - 波特率，例如 115200
 * @return  none
 *********************************************************************/
void BSP_UART_Init(uint32_t baudrate)
{
    /* 1. 配置 GPIO 引脚 */
    GPIOB_SetBits(GPIO_Pin_7);               // TXD 先拉高
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);   // RXD 上拉输入
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA); // TXD 推挽输出

    /* 2. 配置串口参数 */
    UART0_DefInit();                         // 默认配置：115200, 8N1, FIFO 使能
    /* 若需要修改波特率，调用 UART0_BaudRateCfg */
    if(baudrate != 115200) {
        UART0_BaudRateCfg(baudrate);
    }
    g_baudrate = baudrate;

    /* 3. 使能发送器（默认已开启） */
    R8_UART0_IER |= RB_IER_TXD_EN;
}

/*********************************************************************
 * @brief   反初始化：关闭UART0，恢复GPIO为默认状态（可选）
 *********************************************************************/
void BSP_UART_DeInit(void)
{
    R8_UART0_IER = RB_IER_RESET;    // 复位中断使能
    GPIOB_ModeCfg(GPIO_Pin_4 | GPIO_Pin_7, GPIO_ModeIN_PU); // 恢复为输入
    g_baudrate = 0;
}

/*********************************************************************
 * @brief   发送单个字节（阻塞）
 *********************************************************************/
void BSP_UART_SendByte(uint8_t data)
{
    while(R8_UART0_TFC == UART_FIFO_SIZE); // 等待FIFO不满
    R8_UART0_THR = data;
}

/*********************************************************************
 * @brief   发送字符串（阻塞）
 *********************************************************************/
void BSP_UART_SendString(const uint8_t *buf, uint16_t len)
{
    while(len--) {
        BSP_UART_SendByte(*buf++);
    }
}

/*********************************************************************
 * @brief   接收单个字节（阻塞等待）
 *********************************************************************/
uint8_t BSP_UART_RecvByte(void)
{
    while(R8_UART0_RFC == 0);   // 等待FIFO有数据
    return R8_UART0_RBR;
}

/*********************************************************************
 * @brief   非阻塞接收单个字节（无数据时返回 0）
 *********************************************************************/
uint8_t BSP_UART_RecvByteNonBlock(void)
{
    if (R8_UART0_RFC > 0) {
        return R8_UART0_RBR;
    }
    return 0;
}

/*********************************************************************
 * @brief   读取FIFO中所有可用数据，存入缓冲区
 * @return  实际读取的字节数
 *********************************************************************/
uint16_t BSP_UART_RecvString(uint8_t *buf)
{
    uint16_t len = 0;
    while(R8_UART0_RFC) {
        *buf++ = R8_UART0_RBR;
        len++;
    }
    return len;
}

/*********************************************************************
 * @brief   动态修改波特率
 *********************************************************************/
void BSP_UART_SetBaudrate(uint32_t baudrate)
{
    UART0_BaudRateCfg(baudrate);
    g_baudrate = baudrate;
}

/*********************************************************************
 * @brief   获取当前波特率（实际可能因时钟偏差略有误差）
 *********************************************************************/
uint32_t BSP_UART_GetBaudrate(void)
{
    return g_baudrate;
}

/* ==================== printf 重定向 ==================== */

/* 
 * 若使用 GCC（MounRiver Studio），重写 _write 函数。
 * 若使用 IAR，则重写 putchar 或 fputc。
 * 这里以 GCC 为例。
 */
int _write(int file, char *ptr, int len)
{
    (void)file;   // 忽略文件句柄
    for(int i = 0; i < len; i++) {
        BSP_UART_SendByte((uint8_t)ptr[i]);
    }
    return len;
}