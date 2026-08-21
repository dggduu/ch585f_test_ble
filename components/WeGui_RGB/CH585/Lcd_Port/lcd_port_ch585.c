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

/*
 * CH585 WeGui RGB 硬件移植层
 * 屏幕: ST7789V2(BOE154IPS) 240x240 RGB565, 3位色, 页缓存动态刷新
 * 接口: 硬件SPI0 (PA13=SCK PA14=MOSI), CS/DC/RST/BLC 走 PCA9539 扩展IO (bsp_pin_defs.h)
 * 依赖: bsp_spi_init/bsp_spi_send_bulk, SCREEN_CS/DC/RST/BLC 宏
 */

#include "lcd_driver_config.h"

#if(LCD_PORT == _LCD_PORT_DEMO)
#include "lcd_port_ch585.h"
#include "lcd_driver.h"

#include "CH58x_common.h"
#include "bsp_spi.h"
#include "bsp_pin_defs.h"

/*--------------------------------------------------------------
  * 名称: lcd_port_init()
  * 功能: 屏幕接口初始化
  * 说明: SPI 背光IO等 必须调用lcd_ic_init()
  * 注意: 调用前需先完成 BSP_IO_EXT_Init (CS/DC/RST/BLC在扩展IO上)
----------------------------------------------------------------*/
void lcd_port_init(void)
{
	//--1.硬件SPI0初始化(与bsp_lcd_hw共用, 勿重复配置不同模式)--
	bsp_spi_init();
	//----------------------------------------------------------

	//--2.扩展IO电平初始化(空闲状态)--
	SCREEN_CS_SET();//片选空闲
	SCREEN_DC_SET();//数据模式
	lcd_bl_on();    //背光亮
	//--------------------------------

	//--3.硬件复位--
	SCREEN_RST_CLR();
	lcd_delay_ms(100);
	SCREEN_RST_SET();
	lcd_delay_ms(100);
	//--------------

	//--4.屏幕驱动IC初始化--
	lcd_ic_init();
	//----------------------
}

/*--------------------------------------------------------------
  * 名称: lcd_delay_ms(uint32_t ms)
  * 传入1: ms时间
  * 功能: 软件延时
  * 说明: 无需精准
----------------------------------------------------------------*/
void lcd_delay_ms(volatile uint32_t ms)
{
	while (ms--)
	{
		mDelaymS(1);
	}
}

/*--------------------------------------------------------------
  * 名称: lcd_bl_on()
  * 说明: 打开屏幕背光
----------------------------------------------------------------*/
void lcd_bl_on(void)
{
	SCREEN_BLC_SET();
}

/*--------------------------------------------------------------
  * 名称: lcd_bl_off()
  * 说明: 关闭屏幕背光
----------------------------------------------------------------*/
void lcd_bl_off(void)
{
	SCREEN_BLC_CLR();
}

/*--------------------------------------------------------------
  * 名称: lcd_is_busy()
  * 返回: 0屏幕接口空闲 1屏幕接口忙碌
  * 说明: 硬件SPI0阻塞型驱动(SPI0_MasterTrans等待发送完成)
----------------------------------------------------------------*/
uint8_t lcd_is_busy(void)
{
	return 0;
}

/*--------------------------------------------------------------
  * 名称: lcd_send_1Cmd(uint8_t dat)
  * 传入1: dat待发送的命令
  * 功能: 向屏幕发送1个命令
----------------------------------------------------------------*/
void lcd_send_1Cmd(uint8_t dat)
{
	SCREEN_DC_CLR();
	SCREEN_CS_CLR();
	bsp_spi_send_bulk(&dat, 1);
	SCREEN_CS_SET();
}

/*--------------------------------------------------------------
  * 名称: lcd_send_nCmd(uint8_t *p,uint16_t num)
  * 传入1: *p待发送的数组指针
  * 传入2: num发送数量
  * 功能: 向屏幕发送num个命令
  * 说明: TFT类型 数组[0]按照命令发送, 余下按照数据发送
----------------------------------------------------------------*/
void lcd_send_nCmd(uint8_t *p,uint16_t num)
{
	//--1.先发命令--
	SCREEN_DC_CLR();
	SCREEN_CS_CLR();
	bsp_spi_send_bulk(p, 1);
	//---------------

	//--2.剩余按数据发送--
	if (num > 1)
	{
		SCREEN_DC_SET();
		bsp_spi_send_bulk(&p[1], num - 1);
	}
	//----------------------

	//--3.结束发送--
	SCREEN_CS_SET();
	//---------------
}

/*--------------------------------------------------------------
  * 名称: lcd_send_1Dat(uint8_t dat)
  * 传入1: dat待发送的数据
  * 功能: 向屏幕发送1个数据
----------------------------------------------------------------*/
void lcd_send_1Dat(uint8_t dat)
{
	SCREEN_DC_SET();
	SCREEN_CS_CLR();
	bsp_spi_send_bulk(&dat, 1);
	SCREEN_CS_SET();
}

/*--------------------------------------------------------------
  * 名称: lcd_send_nDat(uint8_t *p,uint16_t num)
  * 传入1: *p待发送的数组指针
  * 传入2: num发送数量
  * 功能: 向屏幕发送num个数据
----------------------------------------------------------------*/
void lcd_send_nDat(uint8_t *p,uint16_t num)
{
	SCREEN_DC_SET();
	SCREEN_CS_CLR();
	bsp_spi_send_bulk(p, num);
	SCREEN_CS_SET();
}

//----------------------------RGB565屏幕刷屏接口-------------------------------------
/*--------------------------------------------------------------
  * 名称: void lcd_rgb565_port(uint16_t x0,uint16_t x1,uint16_t page,uint8_t *gram)
  * 传入1:x0刷新起始横坐标x
	* 传入2:x1刷新起始坐标x
  * 传入3:page刷新页
  * 传入4:*gram点阵数据指针 往下8点对齐逐行扫描
  * 功能: 从x,page位置开始刷屏 点阵数据转换成tft数据发送
  * 说明: 逐行转换进缓冲后批量SPI发送, 避免逐字节发送
----------------------------------------------------------------*/
#if (LCD_TYPE == LCD_RGB565)
static uint8_t lcd_rgb565_buf[SCREEN_WIDTH * 2];//一行像素的RGB565数据(大端 hi,lo)

void lcd_rgb565_port(uint16_t x0,uint16_t x1,uint16_t page,uint8_t *page_gram)
{
	uint8_t page_bit,*p,mask,c;
	uint16_t x,y0,y1,x_len;
	y0 = page * 8;
	y1 = y0 + 8;//y1 = SCREEN_HIGH-1;
	x_len = x1-x0;

	//--1.配置刷新窗口位置--
	lcd_set_addr(x0,y0,x1,y1);
	//----------------------

	//--2.准备发送屏幕数据--
	SCREEN_DC_SET();
	SCREEN_CS_CLR();
	//----------------------

	for(page_bit=0;page_bit<8;)
	{
		mask = 0x01<<page_bit++;
		p = page_gram;
		if(++y0 > SCREEN_HIGH){break;}
		{
			uint8_t *buf = lcd_rgb565_buf;
			for(x=0;x<=x_len;x++)
			{
				#if (LCD_COLOUR_BIT==1)//---1位色---
					c=0;if(*p++&mask){c=1;}
				#elif (LCD_COLOUR_BIT==2)//---2位色---
					c=0;if(*p++&mask){c=1;}if(*p++&mask){c+=2;}
				#elif (LCD_COLOUR_BIT==3)//---3位色---
					c=0;if(*p++&mask){c=1;}if(*p++&mask){c+=2;}if(*p++&mask){c+=4;}
				#endif

				//--2.打包一个颜色(大端)--
				*buf++ = lcd_driver.colour[c]>>8;
				*buf++ = lcd_driver.colour[c];
				//-------------------------
			}
			//--3.批量发送一行--
			bsp_spi_send_bulk(lcd_rgb565_buf,(x_len+1)*2);
			//------------------
		}
	}
	//------4.结束发送------
	SCREEN_CS_SET();
	//----------------------
}
#endif

/*--------------------------------------------------------------
  * 名称: uint16_t lcd_gram_crc_port(uint8_t *gram,uint16_t len)
  * 传入1:*gram待校验数组指针
	* 传入2:len待校验数组长度
	* 返回: crc校验值
  * 说明: 软件CRC16(多项式0x1021), 仅用于显存区域变化检测, 两次计算结果一致即可
----------------------------------------------------------------*/
#if ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//动态刷新相关
uint16_t lcd_gram_crc_port(uint8_t *gram,uint16_t len)
{
	uint16_t crc = 0xFFFF;
	uint8_t i;
	while (len--)
	{
		crc ^= (uint16_t)(*gram++) << 8;
		for (i = 0; i < 8; i++)
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021;
			}
			else
			{
				crc <<= 1;
			}
		}
	}
	return crc;
}
#endif

#endif //#if(LCD_PORT == _LCD_PORT_DEMO)
