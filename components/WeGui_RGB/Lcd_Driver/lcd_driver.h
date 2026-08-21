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

#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include "stdint.h"
#include "string.h"
#include "lcd_driver_config.h"
#include "lcd_res.h"

#ifdef __cplusplus
extern "C"{
#endif

//--RAM加速宏--
#ifndef RAM_SPEEDUP_FUNC_0
#	define RAM_SPEEDUP_FUNC_0
#endif
#ifndef RAM_SPEEDUP_FUNC_1
#	define RAM_SPEEDUP_FUNC_1
#endif
#ifndef RAM_SPEEDUP_FUNC_2
#	define RAM_SPEEDUP_FUNC_2
#endif
#ifndef RAM_SPEEDUP_FUNC_3
#	define RAM_SPEEDUP_FUNC_3
#endif

typedef union unicode_par
{
	uint16_t u16;
	uint8_t u8[2];
}unicode_t;

typedef enum circle_part
{
	C_RU = 0x01,//右上八分之一圆
	C_UR = 0x02,//上右八分之一圆
	C_UL = 0x04,//上左八分之一圆
	C_LU = 0x08,//左上八分之一圆
	C_LD = 0x10,//左下八分之一圆
	C_DL = 0x20,//下左八分之一圆
	C_DR = 0x40,//下右八分之一圆
	C_RD = 0x80,//右下八分之一圆

	C_HL = C_UL | C_LU | C_LD | C_DL ,//左半圆
	C_HR = C_UR | C_RU | C_RD | C_DR ,//右半圆
	C_HU = C_RU | C_UR | C_UL | C_LU ,//上半圆
	C_HD = C_LD | C_DL | C_DR | C_RD ,//下半圆

	C_QLU = C_UL | C_LU,//左上角四分圆
	C_QLD = C_LD | C_DL,//左下角四分圆
	C_QRU = C_UR | C_RU,//右上角四分圆
	C_QRD = C_DR | C_RD,//右下角四分圆

	C_ALL = 0xFF,//完整的园

}circle_part_t;

typedef enum lcd_driver_mode//驱动模式
{
# if (LCD_COLOUR_BIT==1)
		write_0=0x00,				      //全屏写0
		write_1=0x01,				      //全屏写1
		write_inverse=0x0F,	      //全屏反转写入
		write_0_inBox=0x10,			  //限制区域内写0(高级)
		write_1_inBox=0x11,			  //限制区域内写1(高级)
		write_inverse_inBox=0x1F, //限制区域内反转(高级)
# elif (LCD_COLOUR_BIT==2)
		write_0=0x00,				      //全屏写00B
		write_1=0x01,				      //全屏写01B
		write_2=0x02,				      //全屏写10B
		write_3=0x03,				      //全屏写11B
		write_inverse=0x0F,	      //全屏反转写入
		write_0_inBox=0x10 ,			//限制区域内写00(高级)
		write_1_inBox=0x11 ,			//限制区域内写01(高级)
		write_2_inBox=0x12 ,			//限制区域内写10(高级)
		write_3_inBox=0x13 ,			//限制区域内写11(高级)
		write_inverse_inBox=0x1F, //限制区域内反转(高级)
# elif (LCD_COLOUR_BIT==3)
		write_0=0x00,				      //全屏写000B
		write_1=0x01,				      //全屏写001B
		write_2=0x02,				      //全屏写010B
		write_3=0x03,				      //全屏写011B
		write_4=0x04,				      //全屏写100B
		write_5=0x05,				      //全屏写101B
		write_6=0x06,				      //全屏写110B
		write_7=0x07,				      //全屏写111B
		write_inverse=0x0F,	      //全屏反转写入
		write_0_inBox=0x10,			  //限制区域内写000(高级)
		write_1_inBox=0x11,			  //限制区域内写001(高级)
		write_2_inBox=0x12,			  //限制区域内写010(高级)
		write_3_inBox=0x13,			  //限制区域内写011(高级)
		write_4_inBox=0x14,			  //限制区域内写100(高级)
		write_5_inBox=0x15,			  //限制区域内写101(高级)
		write_6_inBox=0x16,			  //限制区域内写110(高级)
		write_7_inBox=0x17,			  //限制区域内写111(高级)
		write_inverse_inBox=0x1F, //限制区域内反转(高级)
# endif
}lcd_driver_mode_t;

typedef struct//高级驱动参数
{
	uint16_t x_min;   //限制矩形x最小值
	uint16_t x_max;   //限制矩形x最大值
	uint16_t ypage_min;//限制矩形ypage最小值
	uint16_t ypage_max;//限制矩形ypage最大值
	uint8_t ypage_min_temp;//最小ypage允许写入的像素点映射(储存计算结果,用于快速运行)
	uint8_t ypage_max_temp;//最大ypage允许写入的像素点映射(储存计算结果,用于快速运行)
}LcdBoxDriver_t;

typedef struct lcd_driver
{
	//----------显存----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	uint8_t LCD_GRAM[GRAM_YPAGE_NUM][SCREEN_WIDTH][LCD_COLOUR_BIT];
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	uint8_t LCD_GRAM[GRAM_YPAGE_NUM][SCREEN_WIDTH][LCD_COLOUR_BIT];
	uint8_t lcd_refresh_ypage;//记录当前刷屏的是哪一页
	#endif
	//--------动态刷新--------
	#if ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//动态刷新模式
	uint16_t crc[((SCREEN_HIGH+7)/8)][PAGE_CRC_NUM];//储存crc校验值
	#endif
	//--------绘画驱动--------
	void (*Write_GRAM)(uint16_t x,uint16_t ypage,uint8_t u8_value);//普通写,显示驱动函数
	void (*Clear_GRAM)(uint16_t x,uint16_t ypage,uint8_t u8_value);//普通清,显示驱动函数
	LcdBoxDriver_t Box;//高级驱动辅助,限制写入区域
	#if (LCD_TYPE == LCD_RGB565)
		#if (LCD_COLOUR_BIT == 1)
		uint16_t colour[2];//绘图颜色(RGB565多色阶专用)
		#elif (LCD_COLOUR_BIT == 2)
		uint16_t colour[4];//绘图颜色(RGB565多色阶专用)
		#elif (LCD_COLOUR_BIT == 3)
		uint16_t colour[8];//绘图颜色(RGB565多色阶专用)
		#else
		#error("not support LCD_COLOUR_BIT!")
		#endif
	#endif
	//----------字体----------
	const fonts_ascii_t *fonts_ASCII;   //ASCII字体
	const fonts_utf8_t *fonts_UTF8;//UTF8裁剪字体
	uint8_t newline_high;//字符串\n换行距离
}lcd_driver_t;



typedef enum //flash图片格式代码
{
  IMG_RGB565=0x00,//无压缩 RGB565
  IMG_RGB888=0x01,//无压缩 RGB888
  IMG_RGB555=0x02,//无压缩 RGB555
  IMG_RGB444=0x03,//无压缩 RGB444
  IMG_RGB332=0x04,//无压缩 RGB332
  IMG_OLED=0x0F,//无压缩 点阵
  IMG_RGB565_ZIP_ORLE2=0x20,//原始RLE_2字节对齐压缩 RGB565
  IMG_RGB888_ZIP_ORLE3=0x31,//原始RLE_3字节对齐压缩 RGB888
  IMG_RGB555_ZIP_ORLE2=0x22,//原始RLE_2字节对齐压缩 RGB555
  IMG_RGB444_ZIP_ORLE2=0x23,//原始RLE_2字节对齐压缩 RGB555
  IMG_RGB332_ZIP_ORLE1=0x14,//原始RLE_1字节对齐压缩 RGB332
  IMG_OLED_ZIP_ORLE1=0x1F,//原始RLE_1字节对齐压缩 点阵
}imgarry_type_t;

#if ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//动态刷新
/*--------------------------------------------------------------
  * 名称: lcd_reset_crc()
  * 功能: 刷新一次crc值(动态刷新专用)
  * 说明: 用于强制刷新屏幕,防止动态刷新出现区域不刷新
----------------------------------------------------------------*/
void lcd_reset_crc(void);
#endif

/*--------------------------------------------------------------
  * 名称: LCD_Refresh(void)
  * 功能: 驱动接口,将显存LCD_GRAM全部内容发送至屏幕
  * 说明: weak类型 允许重写
----------------------------------------------------------------*/
uint8_t LCD_Refresh(void);

#if (LCD_TYPE == LCD_RGB565)
/*--------------------------------------------------------------
  * 名称: rgb_set_driver_colour(uint8_t num,uint16_t colour)
  * 传入: num 颜色序号 对应设置writer_num的笔刷颜色
  * 传入: colour 颜色
  * 功能: 设置驱动笔刷颜色(RGB屏幕专用)
  * 说明:
----------------------------------------------------------------*/
void rgb_set_driver_colour(uint8_t num,uint16_t colour);
#endif

/*--------------------------------------------------------------
  * 名称: lcd_set_driver_mode(lcd_driver_mode_t mode)
  * 传入: mode 驱动模式
  * 功能: 设置驱动方式
----------------------------------------------------------------*/
void lcd_set_driver_mode(lcd_driver_mode_t mode);

/*--------------------------------------------------------------
  * 名称: lcd_set_driver_box(uint16_t x_min ,uint16_t y_min ,uint16_t x_max,uint16_t y_max)
  * 传入: (x_min,y_min)起始点 (x_max,y_max)终点
  * 功能: 设置高级驱动的限制区域(Box)
----------------------------------------------------------------*/
void lcd_set_driver_box(uint16_t x_min ,uint16_t y_min ,uint16_t x_max,uint16_t y_max);

/*--------------------------------------------------------------
  * 名称: gram_draw_one_byte(uint16_t x,int16_t y,uint8_t u8_value)
  * 传入1: (x,y)坐标
  * 传入2: u8_page 一字节点阵数据
  * 功能: 将u8_page值以对其坐标的方式写到显存
  * 说明: 坐标点x,y支持负数
----------------------------------------------------------------*/
void gram_draw_one_byte(int16_t x,int16_t y,uint8_t u8_page);

/*--------------------------------------------------------------
  * 名称: lcd_draw_pixl(int16_t x,uint16_t y)
  * 传入: x,y 坐标点
  * 功能: 绘制一个像素点
  * 说明: 坐标点支持负数
----------------------------------------------------------------*/
void lcd_draw_pixl(int16_t x,int16_t y);

/*--------------------------------------------------------------
  * 名称: lcd_draw_line(int16_t x1,int16_t y1,int16_t x2,int16_t y2)
  * 传入: (x1,y1)起点 (x0,y0)终点
  * 功能: 两点绘制一条直线
  * 说明: 布雷森汉姆直线算法
----------------------------------------------------------------*/
void lcd_draw_line(int16_t x1,int16_t y1,int16_t x2,int16_t y2);

/*--------------------------------------------------------------
  * lcd_draw_circel_part(int16_t x0,int16_t y0,uint16_t r,circle_part_t cPart)
  * 传入: (x0,y0):起点  r:半径 cPart:圆的部分
  * 功能: 绘制圆形部分
----------------------------------------------------------------*/
void lcd_draw_circel_part(int16_t x0,int16_t y0,uint16_t r,circle_part_t cPart);

/*--------------------------------------------------------------
  * lcd_fill_circel_part(int16_t x0,int16_t y0,uint16_t r,circle_part_t cPart)
  * 传入: (x0,y0):起点  r:半径 cPart:圆的部分
  * 功能: 填充圆形部分
----------------------------------------------------------------*/
void lcd_fill_circel_part(int16_t x0,int16_t y0,uint16_t r,circle_part_t cPart);

/*--------------------------------------------------------------
  * 名称: lcd_draw_box(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max)
  * 传入: (x_min,y_min)起点 (x_max,y_max)终点
  * 功能: 绘制矩形
----------------------------------------------------------------*/
void lcd_draw_box(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max);

/*--------------------------------------------------------------
  * 名称: lcd_fill_box(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max)
  * 传入: 4点坐标
  * 功能: 填充矩形
  * 说明: 坐标点支持负数
----------------------------------------------------------------*/
void lcd_fill_box(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max);

/*--------------------------------------------------------------
  * 名称: lcd_fill_rbox(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max, int8_t r)
  * 传入: (x_min,y_min)起点 (x_max,y_max)终点 r:半径
  * 功能: 填充倒圆角矩形
----------------------------------------------------------------*/
void lcd_fill_rbox(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max, int8_t r);

/*--------------------------------------------------------------
  * 名称: lcd_draw_rbox(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max, uint16_t r);
  * 传入: (x_min,y_min)起点 (x_max,y_max)终点
  * 功能: 绘制镂空倒圆角矩形
----------------------------------------------------------------*/
void lcd_draw_rbox(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max, uint16_t r);

/*--------------------------------------------------------------
  * 名称: lcd_draw_bitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,uint8_t BMP[])
  * 传入1: x0 坐标左上角横坐标点
  * 传入2: y0 坐标左上角纵坐标点
  * 传入3: sizex 点阵图形x宽度
  * 传入4: sizey 点阵图形y高度
  * 传入5: BMP[] 点阵图形数组地址
  * 功能: 将点阵图形摆放到任意坐标点上
  * 说明: 坐标点支持负数
----------------------------------------------------------------*/
void lcd_draw_bitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,const uint8_t BMP[]);

/*--------------------------------------------------------------
  * 名称: lcd_draw_RLEbitmap(int16_t x0,int16_t y0,uint16_t sizex,uint8_t sizey,uint8_t BMP[])
  * 传入1: x0 坐标左上角横坐标点
	* 传入2: y0 坐标左上角纵坐标点
  * 传入3: sizex 点阵图形x宽度
  * 传入4: sizey 点阵图形y高度
  * 传入5: BMP[] 点阵图形数组地址
  * 功能: 将压缩点阵图形摆放到任意坐标点上
  * 说明: 坐标点支持负数
----------------------------------------------------------------*/
void lcd_draw_RLEbitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,const uint8_t RLEBMP[]);
	
/*--------------------------------------------------------------
  * 名称: lcd_draw_ascii(int16_t x,int16_t y,char chr)
  * 传入: (x,y)左上角坐标 char:字符
  * 功能: 绘制一个Ascii字符
----------------------------------------------------------------*/
void lcd_draw_ascii(int16_t x,int16_t y,char chr);

/*--------------------------------------------------------------
  * 名称: void lcd_draw_unicode(int16_t x,int16_t y,unicode_t unicode_id)
  * 传入: (x,y)左上角坐标 unicode_id
  * 功能: 根据输入的unicode_id寻找对应的"裁剪字库"绘制到屏幕坐标上
----------------------------------------------------------------*/
void lcd_draw_unicode(int16_t x,int16_t y,unicode_t unicode_id);

/*--------------------------------------------------------------
  * 名称: void lcd_draw_int32(int16_t x,int16_t y,int32_t num)//写数字,自动长度,32位带符号
  * 传入: (x,y)左上角坐标 num带符号16位数字
  * 功能: 根据输入的num数字用对应的"ASCII字库"绘制到屏幕对应坐标上
----------------------------------------------------------------*/
void lcd_draw_int32(int16_t x,int16_t y,int32_t num);

/*--------------------------------------------------------------
  * 名称: lcd_draw_utf8_string(int16_t x,int16_t y,char *p)
  * 传入: (x,y)左上角坐标 *p字符串指针
  * 功能: 在指定坐标上按照系统设定字体格式绘制字符串
----------------------------------------------------------------*/
void lcd_draw_utf8_string(int16_t x,int16_t y,char *p);

/*--------------------------------------------------------------
  * 名称: uint16_t lcd_get_utf8_string_xlen(char *p)
  * 传入: *p字符串指针
  * 功能: 返回字符串在系统字体上x方向的总像素长度
----------------------------------------------------------------*/
uint16_t lcd_get_utf8_string_xlen(char *p);

/*--------------------------------------------------------------
  * 名称: lcd_get_utf8_yline(int16_t x,int16_t y,char *p)
  * 传入: *p字符串指针
  * 功能: 返回UTF8字符串换行的次数
----------------------------------------------------------------*/
uint16_t lcd_get_utf8_yline(char *p);

/*--------------------------------------------------------------
  * 名称: lcd_clear_gram(void)
  * 功能: 清显存,全部清为0x00
----------------------------------------------------------------*/
void lcd_clear_gram(void);

/*--------------------------------------------------------------
  * 名称: lcd_fill_gram(uint8_t n)
  * 功能: 显存全部刷成"笔刷n"(lcd_driver.colour[n])的颜色
----------------------------------------------------------------*/
void lcd_fill_gram(uint8_t n);

/*--------------------------------------------------------------
  * 名称: lcd_driver_init()
  * 功能: 驱动初始化
----------------------------------------------------------------*/
void lcd_driver_init(void);

#if (FLASH_PORT != _F_NO_PORT)
/*--------------------------------------------------------------
  * 名称: flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1: x输出坐标
  * 传入2: y输出坐标
  * 传入3: flash_addr带头文件的图片flash地址
  * 功能: 从FLASH读取"带文件头"的任意格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数默认使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
void flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr);
#endif

//---------驱动结构体---------
extern lcd_driver_t lcd_driver;

#ifdef __cplusplus
}
#endif

#endif

