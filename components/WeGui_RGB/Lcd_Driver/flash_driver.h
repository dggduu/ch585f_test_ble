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

#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

#include "lcd_driver.h"

#if (FLASH_PORT != _F_NO_PORT)

/*--------------------------------------------------------------
  * 名称: lcd_draw_flash_bitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,uint32_t flash_addr)
  * 传入1: x0 坐标左上角横坐标点
	* 传入2: y0 坐标左上角纵坐标点
  * 传入3: sizex 点阵图形x宽度
  * 传入4: sizey 点阵图形y高度
  * 传入5: BMP[] 点阵图形数组地址
  * 功能: 从外挂flash读取无损点阵图形摆放到任意坐标点上
  * 说明: 坐标点支持负数
----------------------------------------------------------------*/
void lcd_draw_flash_bitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,uint32_t flash_addr);

/*--------------------------------------------------------------
  * 名称: lcd_draw_flash_RLEbitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,uint32_t flash_addr)
  * 传入1: x0 坐标左上角横坐标点
	* 传入2: y0 坐标左上角纵坐标点
  * 传入3: sizex 点阵图形x宽度
  * 传入4: sizey 点阵图形y高度
  * 传入5: BMP[] 点阵图形数组地址
  * 功能: 从外挂flash读取压缩点阵图形摆放到任意坐标点上
  * 说明: 坐标点支持负数
----------------------------------------------------------------*/
void lcd_draw_flash_RLEbitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,uint32_t flash_addr);
	
//---------------------------------------单色屏--------------------------------------------
#if ((LCD_TYPE == LCD_OLED) || (LCD_TYPE == LCD_GRAY))
/*--------------------------------------------------------------
  * 名称: flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr带头文件的图片flash地址
  * 返回: 无
  * 功能: 从FLASH读取"带文件头"的任意格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
void flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr);

//--------------------------------------RGB565屏-------------------------------------------
#elif (LCD_TYPE == LCD_RGB565)
/*--------------------------------------------------------------
  * 名称: flash_draw_IMG_RGB565(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr图片数据的flash地址
  * 返回: 无
  * 功能: 从FLASH读取IMG_RGB565格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
void flash_draw_IMG_RGB565(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, uint32_t flash_addr);

/*--------------------------------------------------------------
  * 名称: flash_draw_IMG_RGB332(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr图片数据的flash地址
  * 返回: 无
  * 功能: 从FLASH读取IMG_RGB332格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
void flash_draw_IMG_RGB332(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, uint32_t flash_addr);

/*--------------------------------------------------------------
  * 名称: flash_draw_IMG_RGB565_ZIP_ORLE2(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr图片数据的flash地址
  * 返回: 无
  * 功能: 从FLASH读取RGB565_ZIP_ORLE2格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
void flash_draw_IMG_RGB565_ZIP_ORLE2(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, uint32_t flash_addr);

/*--------------------------------------------------------------
  * 名称: flash_draw_IMG_RGB332_ZIP_ORLE1(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr图片数据的flash地址
  * 返回: 无
  * 功能: 从FLASH读取RGB332_ZIP_ORLE1格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
void flash_draw_IMG_RGB332_ZIP_ORLE1(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, uint32_t flash_addr);

/*--------------------------------------------------------------
  * 名称: flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr带头文件的图片flash地址
  * 返回: 无
  * 功能: 从FLASH读取"带文件头"的任意格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
void flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr);

#endif //

#endif //#if (FLASH_PORT != _F_NO_PORT)

#endif //#ifndef FLASH_DRIVER_H

