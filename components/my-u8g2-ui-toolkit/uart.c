// #include "uart.h"
// #include <string.h>
// #include <stdint.h>

// static uint8_t uart_dma_buf[UART_DMA_BUFFER_SIZE];
// static uint8_t uart_rx_buf[UART_RX_BUFFER_SIZE];
// static uint16_t uart_rx_head = 0;
// static uint16_t uart_rx_tail = 0;
// static bool uart_dma_init_flag = false;

// int fputc(int ch, FILE *f)
// {
//     (void)f;
//     while ((UARTx->SR & 0X40) == 0);
//     UARTx->DR = (uint8_t)ch;
//     return ch;
// }

// static void uart_vprintf(const char *format, va_list args)
// {
//     vprintf(format, args);
// }

// static void uart_dma_process_data(void)
// {
//     if (!uart_dma_init_flag) return;
    
//     uint16_t recv_len = UART_DMA_BUFFER_SIZE - DMA_GetCurrDataCounter(UART_DMA_CHANNEL);
//     if (recv_len == 0) return;
    
//     DMA_Cmd(UART_DMA_CHANNEL, DISABLE);
    
//     // 拷贝到环形缓冲区
//     for (uint16_t i = 0; i < recv_len; i++) {
//         if (((uart_rx_tail + 1) % UART_RX_BUFFER_SIZE) == uart_rx_head) break;
//         uart_rx_buf[uart_rx_tail] = uart_dma_buf[i];
//         uart_rx_tail = (uart_rx_tail + 1) % UART_RX_BUFFER_SIZE;
//     }
    
//     DMA_SetCurrDataCounter(UART_DMA_CHANNEL, UART_DMA_BUFFER_SIZE);
//     DMA_Cmd(UART_DMA_CHANNEL, ENABLE);
// }

// bool uart_init(void)
// {
//     GPIO_InitTypeDef GPIO_InitStruct;
//     USART_InitTypeDef USART_InitStruct;
//     DMA_InitTypeDef DMA_InitStruct;
    
//     RCC_APB2PeriphClockCmd(UART_GPIO_CLK | UART_CLK | RCC_APB2Periph_AFIO, ENABLE);
//     RCC_AHBPeriphClockCmd(UART_DMA_CLK, ENABLE);
    
//     GPIO_InitStruct.GPIO_Pin = UART_TX_PIN;
//     GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
//     GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(UART_GPIO_PORT, &GPIO_InitStruct);
    
//     GPIO_InitStruct.GPIO_Pin = UART_RX_PIN;
//     GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//     GPIO_Init(UART_GPIO_PORT, &GPIO_InitStruct);
    
//     USART_InitStruct.USART_BaudRate = UART_BAUDRATE;
//     USART_InitStruct.USART_WordLength = USART_WordLength_8b;
//     USART_InitStruct.USART_StopBits = USART_StopBits_1;
//     USART_InitStruct.USART_Parity = USART_Parity_No;
//     USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//     USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//     USART_Init(UARTx, &USART_InitStruct);
    
//     // 配置RX DMA
//     DMA_DeInit(UART_DMA_CHANNEL);
//     DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(UARTx->DR);
//     DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)uart_dma_buf;
//     DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
//     DMA_InitStruct.DMA_BufferSize = UART_DMA_BUFFER_SIZE;
//     DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//     DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
//     DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//     DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//     DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
//     DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
//     DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
//     DMA_Init(UART_DMA_CHANNEL, &DMA_InitStruct);
    
//     // 使能外设
//     USART_DMACmd(UARTx, USART_DMAReq_Rx, ENABLE);
//     DMA_Cmd(UART_DMA_CHANNEL, ENABLE);
//     USART_Cmd(UARTx, ENABLE);
    
//     // 初始化标记
//     uart_dma_init_flag = true;
//     uart_dma_clear_buf();
    
//     return true;
// }

// void uart_log_printf(const char* file, const long line, const char *tag, const char *format, ...)
// {
//     va_list args;
//     va_start(args, format);
    
//     printf("[%s](%s:%ld): ", tag, file, line);
//     uart_vprintf(format, args);
//     printf("\n");
    
//     va_end(args);
// }

// uint8_t uart_dma_read_byte(void)
// {
//     uart_dma_process_data();
//     if (uart_rx_head == uart_rx_tail) return 0x00;
    
//     uint8_t byte = uart_rx_buf[uart_rx_head];
//     uart_rx_head = (uart_rx_head + 1) % UART_RX_BUFFER_SIZE;
//     return byte;
// }

// uint16_t uart_dma_read_str(char* buf, uint16_t max_len)
// {
//     if (buf == NULL || max_len == 0) return 0;
    
//     uart_dma_process_data();
//     uint16_t read_len = 0;
//     memset(buf, 0, max_len);
    
//     while (uart_rx_head != uart_rx_tail && read_len < (max_len - 1)) {
//         buf[read_len] = uart_rx_buf[uart_rx_head];
//         read_len++;
//         if (buf[read_len-1] == '\n') {
//             uart_rx_head = (uart_rx_head + 1) % UART_RX_BUFFER_SIZE;
//             break;
//         }
//         uart_rx_head = (uart_rx_head + 1) % UART_RX_BUFFER_SIZE;
//     }
    
//     return read_len;
// }

// void uart_dma_clear_buf(void)
// {
//     DMA_Cmd(UART_DMA_CHANNEL, DISABLE);
//     memset(uart_dma_buf, 0, UART_DMA_BUFFER_SIZE);
//     memset(uart_rx_buf, 0, UART_RX_BUFFER_SIZE);
//     uart_rx_head = 0;
//     uart_rx_tail = 0;
//     DMA_SetCurrDataCounter(UART_DMA_CHANNEL, UART_DMA_BUFFER_SIZE);
//     DMA_Cmd(UART_DMA_CHANNEL, ENABLE);
// }

// bool uart_dma_has_data(void)
// {
//     uart_dma_process_data();
//     return (uart_rx_head != uart_rx_tail);
// }