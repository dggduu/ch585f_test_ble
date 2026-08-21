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

#include "lcd_driver_config.h"
#include "flash_driver.h"


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
void lcd_draw_flash_bitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,uint32_t flash_addr)
{
	#define IMG_FLASH_SIZE 128 //设置读取缓冲大小
	uint8_t flash_dat_buff[IMG_FLASH_SIZE];
	uint16_t xmax;
	uint16_t ymax;
	uint16_t i;
	uint8_t * p;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	ymax = y0 + sizey;
	//此处可优化
	//---------页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((y0 >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))||(y0+sizey < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	ymax = (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8;
	if(y0 + sizey < ymax)
	{
		ymax = y0 + sizey;
	}
	while(y0/8+1 < lcd_driver.lcd_refresh_ypage)
	{
		flash_addr += sizex;
		y0 +=8;
	}
	#endif
	if((x0+sizex<0)||(x0>(SCREEN_WIDTH-1)))
	{
		return;
	}
	xmax = x0 + sizex;
	i = 0;
	int16_t y=y0;
	int16_t x=x0;
	while(1)
  {
			if (i <= 0)
			{
				flash_read_addr_ndat(flash_addr, flash_dat_buff, IMG_FLASH_SIZE);
				flash_addr += IMG_FLASH_SIZE;
				p = flash_dat_buff;
				i = IMG_FLASH_SIZE;
			}
			i--;
			if(x >= xmax)
			{
				x=x0;
				y+=8;
				if(y >= ymax)
				{
					return;
				}
			}
			gram_draw_one_byte(x,y,*p++);
			x++;
  }
	#undef IMG_FLASH_SIZE
}

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
void lcd_draw_flash_RLEbitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,uint32_t flash_addr)
{
	#define IMG_FLASH_SIZE 128 //设置读取缓冲大小
	uint8_t flash_dat_buff[IMG_FLASH_SIZE];
	uint8_t rle_num;
	uint8_t rle_dat;
	uint16_t xmax;
	uint16_t i;
	uint8_t * p;
	int16_t y;
	int16_t x;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	uint16_t skip=0;
	//此处可优化
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	uint16_t ymax;
	uint16_t skip=0;
	if((y0 >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))||(y0+sizey < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	ymax = (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8;
	if(y0 + sizey < ymax)
	{
		ymax = y0 + sizey;
	}
	while(y0/8+1 < lcd_driver.lcd_refresh_ypage)
	{
		skip += sizex;
		y0 +=8;
	}
	#endif
	if((x0+sizex<0)||(x0>(SCREEN_WIDTH-1)))
	{
		return;
	}
	xmax = x0 + sizex;
	i=0;
	y=y0;
	x=x0;
	while(1)
  {
			if (i <= 0)
			{
				flash_read_addr_ndat(flash_addr, flash_dat_buff, (IMG_FLASH_SIZE / 2 * 2));
				flash_addr += (IMG_FLASH_SIZE / 2 * 2);
				p = flash_dat_buff;
				i = (IMG_FLASH_SIZE / 2);
			}
			i--;
      rle_num = * p++;
      if(rle_num==0x00)
      {
          break;//结束
      }
			if(skip > 0)
			{
				if(skip > rle_num)
				{
					skip -= rle_num;
					p++;
					continue;
				}
				else
				{
					rle_num -= skip;
					skip = 0;
				}
			}
      rle_dat = * p++;
      while(rle_num--)
      {
					if(x >= xmax)
					{
						x=x0;
						y+=8;
						#if ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
						if(y >= ymax)
						{
							return;
						}
						#endif
					}
					gram_draw_one_byte(x,y,rle_dat);
					x++;
			}
  }
	#undef IMG_FLASH_SIZE
}

#if ((LCD_TYPE == LCD_OLED) || (LCD_TYPE == LCD_GRAY))
__attribute__((weak)) void flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr) 
{
		uint8_t  flash_dat_buff[6];	//头缓冲区
		uint8_t  head_len         ;	//文件头:长度
		uint16_t sizex            ;	//文件头:图片宽
		uint16_t sizey            ;	//文件头:图片高
		uint8_t  img_type         ;	//文件头:图片压缩类型
	
		/* 读取文件头 */
    flash_read_addr_ndat(flash_addr, flash_dat_buff, 6);
    head_len   = flash_dat_buff[0];
    sizex      = ((uint16_t) flash_dat_buff[1] << 8) | flash_dat_buff[2];
    sizey      = ((uint16_t) flash_dat_buff[3] << 8) | flash_dat_buff[4];
    img_type   = flash_dat_buff[5];
    flash_addr = flash_addr + head_len;
    
		/* 解析图片 */
    switch (img_type) 
		{
				/* 无压缩 */
        case IMG_RGB565: ;break;
        case IMG_RGB888: break;
        case IMG_RGB555: break;
        case IMG_RGB444: break;
        case IMG_RGB332: ;break;
			  case IMG_OLED  : lcd_draw_flash_bitmap(x,y,sizex,sizey,flash_addr);break;
				/* 压缩 */
        case IMG_RGB565_ZIP_ORLE2: break;
        case IMG_RGB888_ZIP_ORLE3: break;
        case IMG_RGB555_ZIP_ORLE2: break;
        case IMG_RGB444_ZIP_ORLE2: break;
        case IMG_RGB332_ZIP_ORLE1: break;
				case IMG_OLED_ZIP_ORLE1  : lcd_draw_flash_RLEbitmap(x,y,sizex,sizey,flash_addr);break;
    }
}

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
__attribute__((weak)) void flash_draw_IMG_RGB565(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, uint32_t flash_addr)
{
		#define IMG_FLASH_SIZE 128 //设置读取缓冲大小
		uint8_t flash_dat_buff[IMG_FLASH_SIZE];
		uint32_t total_size   = (uint32_t) sizex * sizey * 2;
		uint32_t aligned_size = total_size / IMG_FLASH_SIZE;//falsh缓冲区对齐的部分
		uint16_t remaining    = total_size % IMG_FLASH_SIZE;//不对齐剩余的部分
	
		lcd_set_addr(x, y, x + sizex - 1, y + sizey - 1);
		while (aligned_size--) 
		{
				flash_read_addr_ndat(flash_addr, flash_dat_buff, IMG_FLASH_SIZE);
				lcd_send_nDat(flash_dat_buff, IMG_FLASH_SIZE);
				flash_addr += IMG_FLASH_SIZE;
		}
		if (remaining > 0) 
		{
				flash_read_addr_ndat(flash_addr, flash_dat_buff, remaining);
				lcd_send_nDat(flash_dat_buff, remaining);
		}
		#undef IMG_FLASH_SIZE
}

/*--------------------------------------------------------------
  * 名称: flash_draw_IMG_RGB332(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr图片数据的flash地址
  * 返回: 无
  * 功能: 从FLASH读取IMG_RGB332格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
__attribute__((weak)) void flash_draw_IMG_RGB332(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, uint32_t flash_addr)
{
		#define IMG_FLASH_SIZE 128 //设置读取缓冲大小
		uint8_t flash_dat_buff[IMG_FLASH_SIZE];
		uint32_t total_size   = (uint32_t) sizex * sizey * 1;
		uint32_t aligned_size = total_size / IMG_FLASH_SIZE;//falsh缓冲区对齐的部分
		uint16_t remaining    = total_size % IMG_FLASH_SIZE;//不对齐剩余的部分
		
		lcd_set_addr(x, y, x + sizex - 1, y + sizey - 1);
		while (aligned_size--) 
		{
				uint16_t i;
				uint8_t * p = flash_dat_buff;
				flash_read_addr_ndat(flash_addr, flash_dat_buff, IMG_FLASH_SIZE);
				flash_addr += IMG_FLASH_SIZE;
				for (i = 0; i < IMG_FLASH_SIZE; i++) 
				{
						uint8_t rgb332 = * p++;
						lcd_send_1Dat(((rgb332 & 0xE0) | ((rgb332 >> 2) & 0x1F))); 
						lcd_send_1Dat(((rgb332 << 3) & 0xF8) | ((rgb332 << 2) & 0x04) | (rgb332 & 0x03)); 
				}
		}
		if (remaining > 0) 
		{
				uint16_t i;
				uint8_t * p = flash_dat_buff;
				flash_read_addr_ndat(flash_addr, flash_dat_buff, remaining);
				for (i = 0; i < remaining; i++) 
				{
						uint8_t rgb332 = * p++;
						lcd_send_1Dat(((rgb332 & 0xE0) | ((rgb332 >> 2) & 0x1F))); 
						lcd_send_1Dat(((rgb332 << 3) & 0xF8) | ((rgb332 << 2) & 0x04) | (rgb332 & 0x03)); 
				}
		}
		#undef IMG_FLASH_SIZE
}

/*--------------------------------------------------------------
  * 名称: flash_draw_IMG_RGB565_ZIP_ORLE2(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr图片数据的flash地址
  * 返回: 无
  * 功能: 从FLASH读取RGB565_ZIP_ORLE2格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
__attribute__((weak)) void flash_draw_IMG_RGB565_ZIP_ORLE2(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, uint32_t flash_addr)
{
		#define IMG_FLASH_SIZE 128 //设置读取缓冲大小
		uint8_t flash_dat_buff[IMG_FLASH_SIZE];
		uint16_t i = 0;
		uint8_t *p;
		uint8_t dat[3];
		lcd_set_addr(x, y, x + sizex - 1, y + sizey - 1);
		while (1) 
		{
				if (i <= 0) 
				{
						flash_read_addr_ndat(flash_addr, flash_dat_buff, (IMG_FLASH_SIZE / 3 * 3));
						flash_addr += (IMG_FLASH_SIZE / 3 * 3);
						p = & flash_dat_buff[0];
						i = (IMG_FLASH_SIZE / 3);
				}
				i--;
				dat[0] = * p++;
				if (dat[0] == 0x00) {break;} 
				dat[1] = * p++;
				dat[2] = * p++;
				while (dat[0]--) 
				{
						lcd_send_1Dat(dat[1]); 
						lcd_send_1Dat(dat[2]); 
				}
		}
		#undef IMG_FLASH_SIZE
}

/*--------------------------------------------------------------
  * 名称: flash_draw_IMG_RGB332_ZIP_ORLE1(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr图片数据的flash地址
  * 返回: 无
  * 功能: 从FLASH读取RGB332_ZIP_ORLE1格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
__attribute__((weak)) void flash_draw_IMG_RGB332_ZIP_ORLE1(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, uint32_t flash_addr)
{
		#define IMG_FLASH_SIZE 128 //读取缓冲大小
		uint8_t flash_dat_buff[IMG_FLASH_SIZE];
		uint16_t i = 0;
    uint8_t * p;
		uint8_t num,rgb332;
		lcd_set_addr(x, y, x + sizex - 1, y + sizey - 1);
    while (1) 
		{
        if (i <= 0) 
				{
            flash_read_addr_ndat(flash_addr, flash_dat_buff, (IMG_FLASH_SIZE / 2 * 2));
            flash_addr += (IMG_FLASH_SIZE / 2 * 2);
            p = & flash_dat_buff[0];
            i = (IMG_FLASH_SIZE / 2);
        }
        i--;
        num = * p++;
        if (num == 0x00) 
				{
            break;
        } 
        rgb332 = * p++;
        while (num--) 
				{
            lcd_send_1Dat(((rgb332 & 0xE0) | ((rgb332 >> 2) & 0x1F))); 
            lcd_send_1Dat(((rgb332 << 3) & 0xF8) | ((rgb332 << 2) & 0x04) | (rgb332 & 0x03)); 
        }
    }
		#undef IMG_FLASH_SIZE
}

/*--------------------------------------------------------------
  * 名称: flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr) 
  * 传入1:x输出坐标
  * 传入2:y输出坐标
  * 传入3:flash_addr带头文件的图片flash地址
  * 返回: 无
  * 功能: 从FLASH读取"带文件头"的任意格式图片投放到屏幕坐标(x,y)上
  * 说明: 该函数使用weak类型 方便修改成更高执行效率的函数 
----------------------------------------------------------------*/
__attribute__((weak)) void flash_draw_img(uint16_t x, uint16_t y, uint32_t flash_addr) 
{
		uint8_t  flash_dat_buff[6];	//头缓冲区
		uint8_t  head_len         ;	//文件头:长度
		uint16_t sizex            ;	//文件头:图片宽
		uint16_t sizey            ;	//文件头:图片高
		uint8_t  img_type         ;	//文件头:图片压缩类型
	
		/* 读取文件头 */
    flash_read_addr_ndat(flash_addr, flash_dat_buff, 6);
    head_len   = flash_dat_buff[0];
    sizex      = ((uint16_t) flash_dat_buff[1] << 8) | flash_dat_buff[2];
    sizey      = ((uint16_t) flash_dat_buff[3] << 8) | flash_dat_buff[4];
    img_type   = flash_dat_buff[5];
    flash_addr = flash_addr + head_len;
    
		/* 解析图片 */
    switch (img_type) 
		{
				/* 无压缩 */
        case IMG_RGB565: flash_draw_IMG_RGB565(x,y,sizex,sizey,flash_addr);break;
        case IMG_RGB888: break;
        case IMG_RGB555: break;
        case IMG_RGB444: break;
        case IMG_RGB332: flash_draw_IMG_RGB332(x,y,sizex,sizey,flash_addr);break;
			  case IMG_OLED  : lcd_draw_flash_bitmap(x,y,sizex,sizey,flash_addr);break;
				/* 压缩 */
        case IMG_RGB565_ZIP_ORLE2: flash_draw_IMG_RGB565_ZIP_ORLE2(x,y,sizex,sizey,flash_addr);break;
        case IMG_RGB888_ZIP_ORLE3: break;
        case IMG_RGB555_ZIP_ORLE2: break;
        case IMG_RGB444_ZIP_ORLE2: break;
        case IMG_RGB332_ZIP_ORLE1: flash_draw_IMG_RGB332_ZIP_ORLE1(x,y,sizex,sizey,flash_addr);break;
				case IMG_OLED_ZIP_ORLE1  : lcd_draw_flash_RLEbitmap(x,y,sizex,sizey,flash_addr);break;
    }
}

#endif
#endif 
