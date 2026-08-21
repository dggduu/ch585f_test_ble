/*
	Copyright 2025 Lu Zhihao

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef _LCD_PORT_CH585_H_
#define _LCD_PORT_CH585_H_

#include "lcd_driver_config.h"

#include "stdint.h"

//此处可包含GPIO CRC SPI DMA等头文件,按需包含
//禁止使用包含main.h或其他容易造成循环包含的文件
//CH585 移植: 硬件SPI0(PA13/PA14/PA15), CS/DC/RST/BLC 走PCA9539扩展IO(见bsp_pin_defs.h)

/*--------------------------------------------------------------
  * 名称: lcd_port_init()
  * 功能: 屏幕接口初始化
  * 说明: SPI 背光IO等
----------------------------------------------------------------*/
void lcd_port_init(void);

/*--------------------------------------------------------------
  * 名称: lcd_delay_ms(uint32_t ms)
  * 传入1: ms时间
  * 功能: 软件延时
  * 说明: 无需精准
----------------------------------------------------------------*/
void lcd_delay_ms(volatile uint32_t ms);

/*--------------------------------------------------------------
  * 名称: lcd_bl_on()
  * 说明: 打开屏幕背光
----------------------------------------------------------------*/
void lcd_bl_on(void);

/*--------------------------------------------------------------
  * 名称: lcd_bl_off()
  * 说明: 关闭屏幕背光
----------------------------------------------------------------*/
void lcd_bl_off(void);

/*--------------------------------------------------------------
  * 名称: lcd_is_busy()
  * 返回: 0屏幕接口空闲 1屏幕接口忙碌
  * 说明: 纯硬件SPI阻塞型驱动无需判断忙碌
----------------------------------------------------------------*/
uint8_t lcd_is_busy(void);

/*--------------------------------------------------------------
  * 名称: lcd_send_1Cmd(uint8_t dat)
  * 传入1: dat待发送的命令
  * 功能: 向屏幕发送1个命令
----------------------------------------------------------------*/
void lcd_send_1Cmd(uint8_t dat);

/*--------------------------------------------------------------
  * 名称: lcd_send_nCmd(uint8_t *p,uint16_t num)
  * 传入1: *p待发送的数组指针
  * 传入2: num发送数量
  * 功能: 向屏幕发送num个命令(TFT: 数组[0]按命令, 其余按数据)
----------------------------------------------------------------*/
void lcd_send_nCmd(uint8_t *p,uint16_t num);

/*--------------------------------------------------------------
  * 名称: lcd_send_1Dat(uint8_t dat)
  * 传入1: *p数组指针
  * 传入2: num发送数量
  * 功能: 向屏幕发送1个数据
----------------------------------------------------------------*/
void lcd_send_1Dat(uint8_t dat);

/*--------------------------------------------------------------
  * 名称: lcd_send_nDat(uint8_t *p,uint16_t num)
  * 传入1: *p数组指针
  * 传入2: num发送数量
  * 功能: 向屏幕发送num个数据
----------------------------------------------------------------*/
void lcd_send_nDat(uint8_t *p,uint16_t num);

/*--------------------------------------------------------------
  * 名称: void lcd_oled_port(uint16_t x0,uint16_t x1,uint16_t page,uint8_t *gram)
  * 传入1:x0刷新起始横坐标
	* 传入2:x1刷新结束横坐标
  * 传入3:page当前刷新的页坐标
  * 传入4:*gram点阵数据指针 往下8点对齐逐行扫描
  * 功能: OLED屏幕从x,page位置开始刷屏
  * 说明: OLED屏幕移植接口 默认weak类型 需要移植改写
----------------------------------------------------------------*/
#if (LCD_TYPE == LCD_OLED)
void lcd_oled_port(uint16_t x0,uint16_t x1,uint16_t page,uint8_t *page_gram);
#endif

/*--------------------------------------------------------------
  * 名称: void lcd_gray_port(uint16_t x0,uint16_t x1,uint16_t page,uint8_t *gram)
  * 传入1:x0刷新起始横坐标
	* 传入2:x1刷新结束横坐标
  * 传入3:page当前刷新的页坐标
  * 传入4:*gram点阵数据指针 往下8点对齐逐行扫描
  * 功能: 灰度OLED屏幕从x,page位置开始刷屏
  * 说明: 灰度OLED屏幕移植接口 默认weak类型 需要移植改写
----------------------------------------------------------------*/
#if (LCD_TYPE == LCD_GRAY)
void lcd_gray_port(uint16_t x0,uint16_t x1,uint16_t page,uint8_t *page_gram);
#endif

/*--------------------------------------------------------------
  * 名称: void rgb565_flush(uint16_t x0,uint16_t x1,uint16_t page,uint8_t *gram)
  * 传入1:x0刷新起始横坐标
	* 传入2:x1刷新结束横坐标
  * 传入3:page当前刷新的页坐标
  * 传入4:*gram点阵数据指针 往下8点对齐逐行扫描
  * 功能: TFT_RGB565屏幕从x,page位置开始刷屏
  * 说明: RGB565屏幕移植接口 默认weak类型 需要移植改写
----------------------------------------------------------------*/
#if (LCD_TYPE == LCD_RGB565)
void lcd_rgb565_port(uint16_t x0,uint16_t x1,uint16_t page,uint8_t *page_gram);
#endif


/*--------------------------------------------------------------
  * 名称: uint16_t lcd_gram_crc_port(uint8_t *gram,uint16_t len)
  * 传入1:*gram待校验数组指针
	* 传入2:len待校验数组长度
	* 返回: crc校验值
  * 说明: weak类型 需要移植,否则无法使用动态刷新
----------------------------------------------------------------*/
#if ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//动态刷新相关
uint16_t lcd_gram_crc_port(uint8_t *gram,uint16_t len);
#endif

#endif
