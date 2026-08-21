#ifndef UART_H
#define UART_H

#include "stm32f10x.h"
#include "stdbool.h"
#include <stdarg.h>
#include <stdio.h>

// ========== 串口配置宏 ==========
#define UARTx                   USART1
#define UART_BAUDRATE           115200
#define UART_GPIO_PORT          GPIOA
#define UART_TX_PIN             GPIO_Pin_9
#define UART_RX_PIN             GPIO_Pin_10
#define UART_GPIO_CLK           RCC_APB2Periph_GPIOA
#define UART_CLK                RCC_APB2Periph_USART1

// ========== DMA配置宏 ==========
#define UART_DMA_CHANNEL        DMA1_Channel5
#define UART_DMA_CLK            RCC_AHBPeriph_DMA1
#define UART_DMA_BUFFER_SIZE    64
#define UART_RX_BUFFER_SIZE     128

// ========== 调试打印开关 ==========
#define UART_DEBUG_ENABLE       1

// ========== 函数声明 ==========
bool uart_init(void);
void uart_log_printf(const char* file, const long line, const char *tag, const char *format, ...);

#if UART_DEBUG_ENABLE
#define uart_debug_printf(format, ...)  uart_log_printf(__FILE__, __LINE__, "DEBUG", format, ##__VA_ARGS__)
#else
#define uart_debug_printf(format, ...)  do {} while(0)
#endif

// ========== DMA接收接口 ==========
uint8_t uart_dma_read_byte(void);
uint16_t uart_dma_read_str(char* buf, uint16_t max_len);
void uart_dma_clear_buf(void);
bool uart_dma_has_data(void);

#endif