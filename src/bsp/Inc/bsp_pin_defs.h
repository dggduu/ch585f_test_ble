#ifndef __BSP_PIN_DEFS_H__
#define __BSP_PIN_DEFS_H__

#include "bsp_io_ext.h"

// 设备地址（由外部定义）
extern uint8_t g_ioext_addr;

// 引脚编号
#define PIN_SCREEN_CS   0
#define PIN_SCREEN_DC   1
#define PIN_SCREEN_RST  2
#define PIN_SCREEN_BLC  3
#define PIN_SD_CS       4

// 屏幕控制宏
#define SCREEN_CS_SET()   BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SCREEN_CS, 1)
#define SCREEN_CS_CLR()   BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SCREEN_CS, 0)

#define SCREEN_DC_SET()   BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SCREEN_DC, 1)
#define SCREEN_DC_CLR()   BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SCREEN_DC, 0)

#define SCREEN_RST_SET()  BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SCREEN_RST, 1)
#define SCREEN_RST_CLR()  BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SCREEN_RST, 0)


//控制LEDK的NMOS电平
#define SCREEN_BLC_SET()  BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SCREEN_BLC, 1)
#define SCREEN_BLC_CLR()  BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SCREEN_BLC, 0)

// SD卡CS控制
#define SD_CS_SET()       BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SD_CS, 1)
#define SD_CS_CLR()       BSP_IO_EXT_SetPinLevel(g_ioext_addr, PIN_SD_CS, 0)

#endif