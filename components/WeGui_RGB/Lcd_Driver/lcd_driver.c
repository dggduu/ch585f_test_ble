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
#include "lcd_driver.h"

//---------驱动结构体---------
lcd_driver_t lcd_driver;

//---------快速计算用---------
const static uint8_t cal_1[] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF,0x57,0x65,0x47,0x75,0x69,0x20,0x52,0x47,0x42};


//------------------------------------------------------------内部驱动接口------------------------------------------------------------

/*--------------------------------------------------------------
  * 名称: LCD_Refresh(void)
  * 功能: 驱动接口,将显存LCD_GRAM全部内容发送至屏幕
  * 说明: weak类型 允许重写
----------------------------------------------------------------*/
RAM_SPEEDUP_FUNC_0
__attribute__((weak)) uint8_t LCD_Refresh(void)
{
	//-------------方式1:全缓存全屏刷--------------
	#if (LCD_MODE == _FULL_BUFF_FULL_UPDATE)
	uint16_t ypage;
	for(ypage=0;ypage<GRAM_YPAGE_NUM;ypage++)
	{
		//--------屏幕发送--------
		#if (LCD_TYPE == LCD_OLED)     //OLED屏幕
			lcd_oled_port(0,SCREEN_WIDTH-1,ypage,&lcd_driver.LCD_GRAM[ypage][0][0]);
		#elif (LCD_TYPE == LCD_GRAY)   //灰度屏幕
			lcd_gray_port(0,SCREEN_WIDTH-1,ypage,&lcd_driver.LCD_GRAM[ypage][0][0]);
		#elif (LCD_TYPE == LCD_RGB565) //彩色屏幕RGB565
			lcd_rgb565_port(0,SCREEN_WIDTH-1,ypage,&lcd_driver.LCD_GRAM[ypage][0][0]);
		#endif
	}
	return 0;

	//-------------方式2:全缓存动态刷--------------
	#elif (LCD_MODE == _FULL_BUFF_DYNA_UPDATE)
	uint8_t ypage;
	for(ypage=0;ypage<GRAM_YPAGE_NUM;ypage++)
	{
		uint16_t i_crc;
		uint8_t i_dyna_count;//每页检测DYNA_CRC_NUM次
		for(i_dyna_count=0;i_dyna_count<PAGE_CRC_NUM;i_dyna_count++)
		{
			uint16_t x_start = i_dyna_count * DYNA_CRC_ONCE_XNUM;
			uint16_t x_end   = x_start + DYNA_CRC_ONCE_XNUM ;
			if(x_end > (SCREEN_WIDTH-1)){x_end = SCREEN_WIDTH-1;}
			//--------CRC动态刷新校验--------
			i_crc = lcd_gram_crc_port(&lcd_driver.LCD_GRAM[ypage][x_start][0],(x_end-x_start)*LCD_COLOUR_BIT);
			//---------------------------
			if(lcd_driver.crc[ypage][i_dyna_count] != i_crc)
			{
				lcd_driver.crc[ypage][i_dyna_count] = i_crc;
				//--------屏幕发送--------
				#if (LCD_TYPE == LCD_OLED)     //OLED屏幕
					lcd_oled_port(x_start,x_end,ypage,&lcd_driver.LCD_GRAM[ypage][x_start][0]);
				#elif (LCD_TYPE == LCD_GRAY)   //灰度屏幕
					lcd_gray_port(x_start,x_end,ypage,&lcd_driver.LCD_GRAM[ypage][x_start][0]);
				#elif (LCD_TYPE == LCD_RGB565) //彩色屏幕RGB565
					lcd_rgb565_port(x_start,x_end,ypage,&lcd_driver.LCD_GRAM[ypage][x_start][0]);
				#endif
				//------------------------
			}
		}
	}
	return 0;

	//-------------方式3:页缓存全屏刷新--------------
	#elif (LCD_MODE == _PAGE_BUFF_FULL_UPDATE)
	uint16_t ypage;
	for(ypage=0;((ypage+lcd_driver.lcd_refresh_ypage)*8) < SCREEN_HIGH;)
	{
		//--------屏幕发送--------
		#if (LCD_TYPE == LCD_OLED)     //OLED屏幕
			lcd_oled_port(0,SCREEN_WIDTH-1,(lcd_driver.lcd_refresh_ypage+ypage),&lcd_driver.LCD_GRAM[ypage][0][0]);
		#elif (LCD_TYPE == LCD_GRAY)   //灰度屏幕
			lcd_gray_port(0,SCREEN_WIDTH-1,ypage,&lcd_driver.LCD_GRAM[ypage][0][0]);
		#elif (LCD_TYPE == LCD_RGB565) //彩色屏幕RGB565
			lcd_rgb565_port(0,SCREEN_WIDTH-1,(lcd_driver.lcd_refresh_ypage+ypage),&lcd_driver.LCD_GRAM[ypage][0][0]);
		#endif
		if(++ypage>=GRAM_YPAGE_NUM){break;}
	}
	lcd_driver.lcd_refresh_ypage += GRAM_YPAGE_NUM;
	if(lcd_driver.lcd_refresh_ypage >= ((SCREEN_HIGH+7)/8))
	{
		lcd_driver.lcd_refresh_ypage = 0;
	}
	return lcd_driver.lcd_refresh_ypage;

	//-------------方式4:页缓存动态刷新--------------
	#elif (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE)
	uint8_t ypage;
	for(ypage=0;ypage<GRAM_YPAGE_NUM;ypage++)
	{
		uint16_t i_crc;
		uint8_t i_dyna_count;//每页检测DYNA_CRC_NUM次
		if((lcd_driver.lcd_refresh_ypage + ypage)>=((SCREEN_HIGH+7)/8)){break;}
		for(i_dyna_count=0;i_dyna_count<PAGE_CRC_NUM;i_dyna_count++)
		{
			uint16_t x_start = i_dyna_count * DYNA_CRC_ONCE_XNUM;
			uint16_t x_end   = x_start + DYNA_CRC_ONCE_XNUM ;
			if(x_end > (SCREEN_WIDTH-1)){x_end = SCREEN_WIDTH-1;}
			//--------CRC动态刷新校验--------
			i_crc = lcd_gram_crc_port(&lcd_driver.LCD_GRAM[ypage][x_start][0],(x_end-x_start)*LCD_COLOUR_BIT);
			//---------------------------
			if(lcd_driver.crc[lcd_driver.lcd_refresh_ypage + ypage][i_dyna_count] != i_crc)
			{
				lcd_driver.crc[lcd_driver.lcd_refresh_ypage + ypage][i_dyna_count] = i_crc;
				//--------屏幕发送--------
				#if (LCD_TYPE == LCD_OLED)     //OLED屏幕
					lcd_oled_port(x_start,x_end,(lcd_driver.lcd_refresh_ypage+ypage),&lcd_driver.LCD_GRAM[ypage][x_start][0]);
				#elif (LCD_TYPE == LCD_GRAY)   //灰度屏幕
					lcd_gray_port(x_start,x_end,(lcd_driver.lcd_refresh_ypage+ypage),&lcd_driver.LCD_GRAM[ypage][x_start][0]);
				#elif (LCD_TYPE == LCD_RGB565) //彩色屏幕RGB565
					lcd_rgb565_port(x_start,x_end,(lcd_driver.lcd_refresh_ypage+ypage),&lcd_driver.LCD_GRAM[ypage][x_start][0]);
				#endif
				//------------------------
			}
		}
	}
	lcd_driver.lcd_refresh_ypage += GRAM_YPAGE_NUM;
	if(lcd_driver.lcd_refresh_ypage >= ((SCREEN_HIGH+7)/8))
	{
		lcd_driver.lcd_refresh_ypage = 0;
	}
	return lcd_driver.lcd_refresh_ypage;
	#endif
}

#if ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//动态刷新

/*--------------------------------------------------------------
  * 名称: lcd_reset_crc()
  * 功能: 刷新一次crc值(动态刷新专用)
  * 说明: 用于强制刷新屏幕,防止动态刷新出现区域不刷新
----------------------------------------------------------------*/
void lcd_reset_crc()
{
	uint16_t i=0;
	uint16_t *p=&lcd_driver.crc[0][0];
	while(i < ((SCREEN_HIGH+7)/8)*PAGE_CRC_NUM)
	{
		p[i]=0xff;//随机值,不是0x0就ok
		i++;
	}
}
#endif

#if (LCD_TYPE == LCD_RGB565)
/*--------------------------------------------------------------
  * 名称: rgb_set_driver_colour(uint8_t num,uint16_t colour)
	* 传入: num 颜色序号 对应设置writer_num的笔刷颜色
	* 传入: colour 颜色
	* 功能: 设置驱动笔刷颜色
  * 说明:
----------------------------------------------------------------*/
void rgb_set_driver_colour(uint8_t num,uint16_t colour)
{
	#if (LCD_COLOUR_BIT == 1)
	static uint16_t colour_lastime[2]={0x0000,0x0000};
	#elif (LCD_COLOUR_BIT == 2)
	static uint16_t colour_lastime[4]={0x0000,0x0000,0x0000,0x0000};
	#elif (LCD_COLOUR_BIT == 3)
	static uint16_t colour_lastime[8]={0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};
	#else
	#error("not support LCD_COLOUR_BIT!")
	#endif

	if(colour_lastime[num] != colour)
	{
		lcd_driver.colour[num] = colour_lastime[num] = colour;
		#if((LCD_MODE == _FULL_BUFF_DYNA_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))
		//动态刷新才有crc
		lcd_reset_crc();//色彩已改变,强制刷新一下crc动态刷新值, 保证颜色正确
		#endif
	}
}
#endif


/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_0(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以0的颜色写入显存
----------------------------------------------------------------*/
RAM_SPEEDUP_FUNC_1 //代码加速宏定义
static void lcd_driver_Write_0(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存

#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}


/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_1(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以1的颜色写入显存
----------------------------------------------------------------*/
RAM_SPEEDUP_FUNC_1 //代码加速宏定义
static void lcd_driver_Write_1(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_2(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以1的颜色写入显存
----------------------------------------------------------------*/
#if(LCD_COLOUR_BIT>=2)
RAM_SPEEDUP_FUNC_2 //代码加速宏定义
static void lcd_driver_Write_2(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] &=(~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &=(~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] &=(~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &=(~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif


/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_3(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以3的颜色写入显存
----------------------------------------------------------------*/
#if(LCD_COLOUR_BIT>=2)
RAM_SPEEDUP_FUNC_2 //代码加速宏定义
static void lcd_driver_Write_3(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_4(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以4的颜色写入显存
----------------------------------------------------------------*/
#if(LCD_COLOUR_BIT>=3)
RAM_SPEEDUP_FUNC_3 //代码加速宏定义
static void lcd_driver_Write_4(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_5(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以5的颜色写入显存
----------------------------------------------------------------*/
#if(LCD_COLOUR_BIT>=3)
RAM_SPEEDUP_FUNC_3 //代码加速宏定义
static void lcd_driver_Write_5(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0]|= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_6(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以6的颜色写入显存
----------------------------------------------------------------*/
#if(LCD_COLOUR_BIT>=3)
RAM_SPEEDUP_FUNC_3 //代码加速宏定义
static void lcd_driver_Write_6(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_7(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以7的颜色写入显存
----------------------------------------------------------------*/
#if(LCD_COLOUR_BIT>=3)
RAM_SPEEDUP_FUNC_3 //代码加速宏定义
static void lcd_driver_Write_7(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_inv(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 普通快速驱动函数,将值以"反色"的方式写入显存
----------------------------------------------------------------*/
static void lcd_driver_Write_inv(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] ^= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] ^= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] ^= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] ^= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_0_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以0的颜色写入到显存
----------------------------------------------------------------*/
static void lcd_driver_Write_0_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;


	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_1_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以1的颜色写入到显存
----------------------------------------------------------------*/
static void lcd_driver_Write_1_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;

	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_2_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以2的颜色写入到显存
----------------------------------------------------------------*/
#if (LCD_COLOUR_BIT>=2)
static void lcd_driver_Write_2_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;;

	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif


/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_3_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以3的颜色写入到显存
----------------------------------------------------------------*/
#if (LCD_COLOUR_BIT>=2)
static void lcd_driver_Write_3_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;

	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------单页缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] &= (~u8_value);
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif


/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_4_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以4的颜色写入到显存
----------------------------------------------------------------*/
#if (LCD_COLOUR_BIT>=3)
static void lcd_driver_Write_4_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;

	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_5_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以5的颜色写入到显存
----------------------------------------------------------------*/
#if (LCD_COLOUR_BIT>=3)
static void lcd_driver_Write_5_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;

	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_6_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以6的颜色写入到显存
----------------------------------------------------------------*/
#if (LCD_COLOUR_BIT>=3)
static void lcd_driver_Write_6_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;

	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] &= (~u8_value);
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_7_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以7的颜色写入到显存
----------------------------------------------------------------*/
#if (LCD_COLOUR_BIT>=3)
static void lcd_driver_Write_7_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;

	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] |= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] |= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}
#endif

/*--------------------------------------------------------------
  * 名称: lcd_driver_Write_inv_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
  * 功能: 高级驱动函数,在限制区域(Box)内,将值以反写的方式写入到显存
----------------------------------------------------------------*/
static void lcd_driver_Write_inv_inBox(uint16_t x,uint16_t ypage,uint8_t u8_value)
{
	if((x<lcd_driver.Box.x_min)||(x>lcd_driver.Box.x_max))
		return;
	if((ypage<lcd_driver.Box.ypage_min)||(ypage>lcd_driver.Box.ypage_max))
		return;
	if(ypage == lcd_driver.Box.ypage_min)
		u8_value = u8_value & lcd_driver.Box.ypage_min_temp;
	if(ypage == lcd_driver.Box.ypage_max)
		u8_value = u8_value & lcd_driver.Box.ypage_max_temp;
	if(u8_value == 0x00)
		return;


	//---------全屏缓存-----------
#	if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
#		if (LCD_COLOUR_BIT>=1)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] ^= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] ^= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	//---------自定缓存-----------
#	elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((ypage >= lcd_driver.lcd_refresh_ypage) && (ypage <= lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM-1))
	{
		ypage%=GRAM_YPAGE_NUM;
#		if (LCD_COLOUR_BIT==1)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
#		elif (LCD_COLOUR_BIT==2)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] ^= u8_value;
#		elif (LCD_COLOUR_BIT==3)
	  lcd_driver.LCD_GRAM[ypage][x][0] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][1] ^= u8_value;
		lcd_driver.LCD_GRAM[ypage][x][2] ^= u8_value;
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif
	}
#	endif
}



/*--------------------------------------------------------------
  * 名称: lcd_set_driver_mode(lcd_driver_mode_t mode)
  * 传入: mode 驱动模式 设置笔刷(颜色)
  * 功能: 设置驱动方式
----------------------------------------------------------------*/
void lcd_set_driver_mode(lcd_driver_mode_t mode)
{
#		if (LCD_COLOUR_BIT==1)
	  switch(mode)
		{
			case write_0 :						//(普通)全屏写0显示
				lcd_driver.Write_GRAM = lcd_driver_Write_0;
				lcd_driver.Clear_GRAM = lcd_driver_Write_1;
				break;
			case write_1 :						//(普通)全屏写1显示
				lcd_driver.Write_GRAM = lcd_driver_Write_1;
				lcd_driver.Clear_GRAM = lcd_driver_Write_0;
				break;
			case write_inverse :			//(普通)全屏反转写入
				lcd_driver.Write_GRAM = lcd_driver_Write_inv;
				lcd_driver.Clear_GRAM = lcd_driver_Write_inv;
				break;
			case write_0_inBox:				//(高级)限制区域内写0
				lcd_driver.Write_GRAM = lcd_driver_Write_0_inBox;
				lcd_driver.Clear_GRAM = lcd_driver_Write_1_inBox;
				break;
			case write_1_inBox:				//(高级)限制区域内写1
				lcd_driver.Write_GRAM = lcd_driver_Write_1_inBox;
				lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;
				break;
			case write_inverse_inBox:	//(高级)限制区域内反转
				lcd_driver.Write_GRAM = lcd_driver_Write_inv_inBox;
				lcd_driver.Clear_GRAM = lcd_driver_Write_inv_inBox;
				break;
		}
#		elif (LCD_COLOUR_BIT==2)
	  switch(mode)
		{
			case write_0 :						//(普通)全屏写0显示
				lcd_driver.Write_GRAM = lcd_driver_Write_0;
				lcd_driver.Clear_GRAM = lcd_driver_Write_3;
				break;
			case write_1 :						//(普通)全屏写1显示
				lcd_driver.Write_GRAM = lcd_driver_Write_1;
				lcd_driver.Clear_GRAM = lcd_driver_Write_0;
				break;
			case write_2 :						//(普通)全屏写2显示
				lcd_driver.Write_GRAM = lcd_driver_Write_2;
				lcd_driver.Clear_GRAM = lcd_driver_Write_0;
				break;
			case write_3 :						//(普通)全屏写3显示
				lcd_driver.Write_GRAM = lcd_driver_Write_3;
				lcd_driver.Clear_GRAM = lcd_driver_Write_0;
				break;
			case write_inverse :			//(普通)全屏反转写入
				lcd_driver.Write_GRAM = lcd_driver_Write_inv;
				lcd_driver.Clear_GRAM = lcd_driver_Write_inv;
				break;
			case write_0_inBox:				//(高级)限制区域内写0
				lcd_driver.Write_GRAM = lcd_driver_Write_0_inBox;
				lcd_driver.Clear_GRAM = lcd_driver_Write_1_inBox;
				break;
			case write_1_inBox:				//(高级)限制区域内写1
				lcd_driver.Write_GRAM = lcd_driver_Write_1_inBox;
				lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;
				break;
			case write_2_inBox:				//(高级)限制区域内写1
				lcd_driver.Write_GRAM = lcd_driver_Write_2_inBox;
				lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;
				break;
			case write_3_inBox:				//(高级)限制区域内写1
				lcd_driver.Write_GRAM = lcd_driver_Write_3_inBox;
				lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;
				break;
			case write_inverse_inBox:	//(高级)限制区域内反转
				lcd_driver.Write_GRAM = lcd_driver_Write_inv_inBox;
				lcd_driver.Clear_GRAM = lcd_driver_Write_inv_inBox;
				break;
		}
#		elif (LCD_COLOUR_BIT==3)
		lcd_driver.Clear_GRAM = lcd_driver_Write_0;
	  switch(mode)
		{
			case write_0 :lcd_driver.Write_GRAM = lcd_driver_Write_0;lcd_driver.Clear_GRAM = lcd_driver_Write_0;break;
			case write_1 :lcd_driver.Write_GRAM = lcd_driver_Write_1;lcd_driver.Clear_GRAM = lcd_driver_Write_0;break;
			case write_2 :lcd_driver.Write_GRAM = lcd_driver_Write_2;lcd_driver.Clear_GRAM = lcd_driver_Write_0;break;
			case write_3 :lcd_driver.Write_GRAM = lcd_driver_Write_3;lcd_driver.Clear_GRAM = lcd_driver_Write_0;break;
			case write_4 :lcd_driver.Write_GRAM = lcd_driver_Write_4;lcd_driver.Clear_GRAM = lcd_driver_Write_0;break;
			case write_5 :lcd_driver.Write_GRAM = lcd_driver_Write_5;lcd_driver.Clear_GRAM = lcd_driver_Write_0;break;
			case write_6 :lcd_driver.Write_GRAM = lcd_driver_Write_6;lcd_driver.Clear_GRAM = lcd_driver_Write_0;break;
			case write_7 :lcd_driver.Write_GRAM = lcd_driver_Write_7;lcd_driver.Clear_GRAM = lcd_driver_Write_0;break;
			case write_inverse:lcd_driver.Write_GRAM = lcd_driver_Write_inv;lcd_driver.Clear_GRAM = lcd_driver_Write_inv;break;
			case write_0_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_0_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;break;
			case write_1_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_1_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;break;
			case write_2_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_2_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;break;
			case write_3_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_3_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;break;
			case write_4_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_4_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;break;
			case write_5_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_5_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;break;
			case write_6_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_6_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;break;
			case write_7_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_7_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_0_inBox;break;
			case write_inverse_inBox:lcd_driver.Write_GRAM = lcd_driver_Write_inv_inBox;lcd_driver.Clear_GRAM = lcd_driver_Write_inv_inBox;break;
		}
#		else
		#error("not support this LCD_COLOUR_BIT!")
#		endif

}

//--------------------------------------------------------------驱动接口--------------------------------------------------------------


/*--------------------------------------------------------------
  * 名称: lcd_set_driver_box(uint8_t x_min ,uint8_t y_min ,uint8_t x_max,uint8_t y_max)
  * 传入: (x_min,y_min)起始点 (x_max,y_max)终点
  * 功能: 设置高级驱动的限制区域(Box)
----------------------------------------------------------------*/
void lcd_set_driver_box(uint16_t x_min ,uint16_t y_min ,uint16_t x_max,uint16_t y_max)
{
	lcd_driver.Box.x_min = x_min;
	lcd_driver.Box.x_max = x_max;
	lcd_driver.Box.ypage_min = y_min/8;
	lcd_driver.Box.ypage_max = y_max/8;

	y_min =  8 - y_min%8;
	lcd_driver.Box.ypage_min_temp = 0;
	while (y_min--)
	{
		lcd_driver.Box.ypage_min_temp = lcd_driver.Box.ypage_min_temp >>1;
		lcd_driver.Box.ypage_min_temp |= 0x80;
	}
	y_max = y_max%8;
	lcd_driver.Box.ypage_max_temp = 0;

	while (y_max--)
	{
		lcd_driver.Box.ypage_max_temp = lcd_driver.Box.ypage_max_temp <<1;
		lcd_driver.Box.ypage_max_temp |= 0x01;
	}
}

/*--------------------------------------------------------------
  * 名称: gram_draw_one_byte(uint16_t x,int16_t y,uint8_t u8_value)
  * 传入1: (x,y)坐标
  * 传入2: u8_page 一字节点阵数据
  * 功能: 将u8_page值以对其坐标的方式写到显存
  * 说明: 坐标点x,y支持负数
----------------------------------------------------------------*/
RAM_SPEEDUP_FUNC_1 //代码加速宏定义
void gram_draw_one_byte(int16_t x,int16_t y,uint8_t u8_page)
{
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	if((x<0)||(x>(SCREEN_WIDTH-1))||(y>=SCREEN_HIGH)||(y+8<0))
	{
		return;
	}
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((y >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))||(y+8 < (lcd_driver.lcd_refresh_ypage*8))||((uint16_t)x>(SCREEN_WIDTH-1))/*||(x<0)||(x>(SCREEN_WIDTH-1))*/)
	{
		return;
	}
	#endif

	if (y>=0)
	{
		uint8_t start_page  = y>>3;//y/8;
		uint8_t page_offset = y&0x07;//y%8;
		lcd_driver.Write_GRAM(x,start_page,(u8_page<<page_offset));
		if(page_offset!=0)
		{
			if(start_page < (((SCREEN_HIGH+7)/8)-1))
			{
				lcd_driver.Write_GRAM(x,start_page+1,(u8_page>>(8-page_offset)));
			}
		}
	}
	else if(y> -8)
	{
		lcd_driver.Write_GRAM(x,0,(u8_page>>(-y)));
	}
}

/*--------------------------------------------------------------
  * 名称: lcd_draw_pixl(int16_t x,int16_t y)
  * 传入: x,y 坐标点
  * 功能: 绘制一个像素点
  * 说明: 支持负数不报错
----------------------------------------------------------------*/
inline void lcd_draw_pixl(int16_t x,int16_t y)
{
	gram_draw_one_byte(x,y,0x01);
}

//高效率画垂直线, 输入x点 Y起点,Y终点
static void Lcd_Draw_VLine(int16_t x,int16_t y_min, int16_t y_max)
{
	uint8_t offset;
	uint8_t ypage;
	uint8_t i;
	if((x<0)||(x>=SCREEN_WIDTH))
	{
		return;
	}
	if(y_max < y_min)
	{
		int16_t temp = y_min;
		y_min = y_max;
		y_max = temp;
	}
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	if(y_min < 0)
	{
		y_min = 0;
	}
	if(y_max > (SCREEN_HIGH-1))
	{
		y_max = SCREEN_HIGH-1;
	}
	if((y_max < 0) ||(y_min > SCREEN_HIGH))
	{
		return;
	}
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if(y_min < (lcd_driver.lcd_refresh_ypage*8))
	{
		y_min = (lcd_driver.lcd_refresh_ypage*8);
	}
	if(y_min > ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))
	{
		return;
	}
	if(y_max > ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))
	{
		y_max = ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8);
	}
	#endif
	ypage   = y_min>>3;//y_min / 8;
	offset = y_min&0x07;//y_min % 8;
	//处理第一ypage
	if(offset!=0)
	{
		i=0x00;
		while (offset<8)
		{
			if(y_min <= y_max)
			{
				i |= (1<<offset);
				y_min ++;
				offset ++;
			}
			else
			{
				lcd_driver.Write_GRAM(x,ypage,i);
				return;
			}
		}
		lcd_driver.Write_GRAM(x,ypage,i);
		ypage++;
	}
	y_min --;
	while(y_min <= y_max)
	{
		offset = y_max - y_min;
		//处理中间ypage
		if(offset>=8)
		{
			lcd_driver.Write_GRAM(x,ypage,0xff);
			ypage++;
			y_min+=8;
		}
		//处理结束ypage
		else
		{
			lcd_driver.Write_GRAM(x,ypage,cal_1[offset]);
			break;
		}
	}
}

//高效率画水平线, 输入x点 Y起点,Y终点
static void Lcd_Draw_HLine(int16_t x_min ,int16_t y, int16_t x_max)
{
	uint8_t offset;
	uint8_t ypage;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	if((y<0)||(y>=SCREEN_HIGH))
	{
		return;
	}
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存 优化
	if((y/8 > (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)) || (y/8 < (lcd_driver.lcd_refresh_ypage)))
	{
		return;
	}
	#endif
	if(x_max < x_min)
	{
		int16_t temp = x_min;
		x_min = x_max;
		x_max = temp;
	}
	if((x_max < 0) ||(x_min > SCREEN_WIDTH))
	{
		return;
	}
	if(x_min < 0)
	{
		x_min = 0;
	}
	if(x_max > (SCREEN_WIDTH-1))
	{
		x_max = SCREEN_WIDTH-1;
	}
	ypage   = y / 8;
	offset = 0x01 << (y % 8);
	while(x_min<=x_max)
	{
		lcd_driver.Write_GRAM(x_min,ypage,offset);
		x_min++;
	}
}


/*--------------------------------------------------------------
  * 名称: lcd_draw_line(int16_t x1,int16_t y1,int16_t x2,int16_t y2)
  * 传入: (x1,y1)起点 (x0,y0)终点
  * 功能: 两点绘制一条直线
  * 说明: 布雷森汉姆直线算法
----------------------------------------------------------------*/
void lcd_draw_line(int16_t x1,int16_t y1,int16_t x2,int16_t y2)
{
	uint16_t t;
	int16_t xerr,yerr,delta_x,delta_y,distance,incx,incy,uRow,uCol;

	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存 优化
	if(
		((y1 >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8)) && (y2 >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8)))
		||((y1 < (lcd_driver.lcd_refresh_ypage*8))&&(y2 < (lcd_driver.lcd_refresh_ypage*8))))
	{
		return;
	}
	#endif

	xerr=0;
	yerr=0;
	delta_x=x2-x1; //计算坐标增量
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0){incx=1;} //设置单步方向
	else if (delta_x==0)//垂直线
	{
		Lcd_Draw_VLine(x1,y1,y2);//高效率绘制垂直线
		return;
	}
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)//水平线
	{
		Lcd_Draw_HLine(x1,y1,x2);//高效率绘制水平线
		return;
	}
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		lcd_draw_pixl(uRow,uCol);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
	lcd_draw_pixl(x2,y2);//画点
}


/*--------------------------------------------------------------
  * lcd_draw_circel_part(int16_t x0,int16_t y0,uint16_t r,circle_part_t cPart)
  * 传入: (x0,y0):起点  r:半径 cPart:圆的部分
  * 功能: 绘制圆形部分
----------------------------------------------------------------*/
void lcd_draw_circel_part(int16_t x0,int16_t y0,uint16_t r,circle_part_t cPart)
{
	int16_t x;
	int16_t y;
	int16_t d;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存 优化
	if((y0-r >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))||(y0+r < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	#endif
	x = 0;
	y = r;
	d = 3 - r*2;
	while(x <= y)
	{
		if(cPart&C_RU)lcd_draw_pixl(x0 + y, y0 - x);
		if(cPart&C_UR)lcd_draw_pixl(x0 + x, y0 - y);
		if(cPart&C_UL)lcd_draw_pixl(x0 - x, y0 - y);
		if(cPart&C_LU)lcd_draw_pixl(x0 - y, y0 - x);
		if(cPart&C_LD)lcd_draw_pixl(x0 - y, y0 + x);
		if(cPart&C_DL)lcd_draw_pixl(x0 - x, y0 + y);
		if(cPart&C_DR)lcd_draw_pixl(x0 + x, y0 + y);
		if(cPart&C_RD)lcd_draw_pixl(x0 + y, y0 + x);
		if(d < 0)
		{
			d += x*4 + 6;
		}
		else
		{
			d += 10 + (x-y)*4;
			y--;
		}
		x++;
	}
	//---专门处理反色驱动---
	x--;
	if((lcd_driver.Write_GRAM == lcd_driver_Write_inv)||
		(lcd_driver.Write_GRAM == lcd_driver_Write_inv_inBox))
	{
		if((cPart&(C_RD|C_DR))==(C_RD|C_DR))lcd_draw_pixl(x0+x,y0+x);//补右下角边边界重叠点
		if((cPart&(C_LU|C_UL))==(C_LU|C_UL))lcd_draw_pixl(x0-x,y0-x);//补左上角边边界重叠点
		if((cPart&(C_LD|C_DL))==(C_LD|C_DL))lcd_draw_pixl(x0-x,y0+x);//补左下角边边界重叠点
		if((cPart&(C_RU|C_UR))==(C_RU|C_UR))lcd_draw_pixl(x0+x,y0-x);//补右上角边边界重叠点
		if((cPart&(C_RU|C_RD))==(C_RU|C_RD))lcd_draw_pixl(x0+r,y0);//补右边边界重叠点
		if((cPart&(C_LU|C_LD))==(C_LU|C_LD))lcd_draw_pixl(x0-r,y0);//补左边边界重叠点
		if((cPart&(C_UL|C_UR))==(C_UL|C_UR))lcd_draw_pixl(x0,y0-r);//补上边边界重叠点
		if((cPart&(C_DL|C_DR))==(C_DL|C_DR))lcd_draw_pixl(x0,y0+r);//补下边边界重叠点
	}
}

/*--------------------------------------------------------------
  * lcd_fill_circel_part(int16_t x0,int16_t y0,uint16_t r,circle_part_t cPart)
  * 传入: (x0,y0):起点  r:半径 cPart:圆的部分
  * 功能: 填充圆形部分
----------------------------------------------------------------*/
void lcd_fill_circel_part(int16_t x0,int16_t y0,uint16_t r,circle_part_t cPart)
{
	int16_t x;
	int16_t y;
	int16_t d;
	uint16_t j;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存 优化
	if((y0-r >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))||(y0+r < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	#endif
	x = 0;
	y = r;
	d = 3 - r*2;
	while(x <= y)
	{
			//填充圆角
			if(cPart&C_RU)Lcd_Draw_HLine(x0 + y, y0 - x, x0+x);//lcd_draw_line(x0 + y, y0 - x, x0 + y, y0);
			if(cPart&C_UR)Lcd_Draw_VLine(x0 + x, y0 - y, y0-x);//lcd_draw_line(x0 + x, y0 - y, x0, y0 - y);
			if(cPart&C_UL)Lcd_Draw_VLine(x0 - x, y0 - y, y0-x);//lcd_draw_line(x0 - x, y0 - y, x0, y0 - y);
			if(cPart&C_LU)Lcd_Draw_HLine(x0 - y, y0 - x, x0-x);//lcd_draw_line(x0 - y, y0 - x, x0 - y, y0);
			if(cPart&C_LD)Lcd_Draw_HLine(x0 - y, y0 + x, x0-x);//lcd_draw_line(x0 - y, y0 + x, x0 - y, y0);
			if(cPart&C_DL)Lcd_Draw_VLine(x0 - x, y0 + y, y0+x);//lcd_draw_line(x0 - x, y0 + y, x0, y0 + y);
			if(cPart&C_DR)Lcd_Draw_VLine(x0 + x, y0 + y, y0+x);//lcd_draw_line(x0 + x, y0 + y, x0, y0 + y);
			if(cPart&C_RD)Lcd_Draw_HLine(x0 + y, y0 + x, x0+x);//lcd_draw_line(x0 + y, y0 + x, x0 + y, y0);
		if(d < 0)
		{
			d += x*4 + 6;
		}
		else
		{
			d += 10 + (x-y)*4;
			y--;
		}
		x++;
	}
	//---专门处理反色驱动---
	if((lcd_driver.Write_GRAM == lcd_driver_Write_inv)||
		(lcd_driver.Write_GRAM == lcd_driver_Write_inv_inBox))
	{
		j=0;
		while(j<x)
		{
			if((cPart&(C_RD|C_DR))==(C_RD|C_DR))lcd_draw_pixl(x0+j,y0+j);//画点
			if((cPart&(C_LU|C_UL))==(C_LU|C_UL))lcd_draw_pixl(x0-j,y0-j);//画点
			if((cPart&(C_LD|C_DL))==(C_LD|C_DL))lcd_draw_pixl(x0-j,y0+j);//画点
			if((cPart&(C_RU|C_UR))==(C_RU|C_UR))lcd_draw_pixl(x0+j,y0-j);//画点
			j++;
		}
		if((cPart&(C_RU|C_RD))==(C_RU|C_RD))Lcd_Draw_HLine(x0, y0, x0+r);
		if((cPart&(C_LU|C_LD))==(C_LU|C_LD))Lcd_Draw_HLine(x0-r, y0, x0);
		if((cPart&(C_UL|C_UR))==(C_UL|C_UR))Lcd_Draw_VLine(x0, y0, y0+r);
		if((cPart&(C_DL|C_DR))==(C_DL|C_DR))Lcd_Draw_VLine(x0, y0-r, y0);
	}
}

/*--------------------------------------------------------------
  * 名称: lcd_fill_box(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max)
  * 传入: 4点坐标
  * 功能: 填充矩形
  * 说明: 坐标点支持负数
----------------------------------------------------------------*/
void lcd_fill_box(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max)
{
	uint8_t ypage;
	uint8_t offset;
	uint16_t x;

	if(y_max < y_min)
	{
		int16_t temp = y_min;
		y_min = y_max;
		y_max = temp;
	}
	if(y_min < 0){y_min=0;}
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	if(y_max > (SCREEN_HIGH-1)){y_max = (SCREEN_HIGH-1);}
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	uint16_t max = (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8;
	if(y_max > max){y_max = max;}
	if((y_min >= max)||(y_max < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	#endif

	if(x_max < x_min)
	{
		int16_t temp = x_min;
		x_min = x_max;
		x_max = temp;
	}
	if(x_min < 0){x_min=0;}
	if(x_max > (SCREEN_WIDTH-1)){x_max = (SCREEN_WIDTH-1);}
	if((x_min>x_max)||(y_min>y_max)){return;}//错误值,不绘画

	ypage   = y_min / 8;
	offset = y_min % 8;
	x = x_min;
	//处理第一ypage
	if(offset!=0)
	{
		uint8_t i=0x00;
		while (offset<8)
		{
			if(y_min <= y_max)
			{
				i |= (1<<offset);
				y_min ++;
				offset ++;
			}
			else
			{
				while(x <= x_max)
				{
					lcd_driver.Write_GRAM(x,ypage,i);
					x++;
				}
				return;
			}
		}
		while(x <= x_max)
		{
			lcd_driver.Write_GRAM(x,ypage,i);
			x++;
		}
		x = x_min;
		ypage++;
	}
	y_min --;
	while(y_min <= y_max)
	{
		offset = y_max - y_min;
		//处理中间ypage
		if(offset>=8)
		{
			while(x <= x_max)
			{
				lcd_driver.Write_GRAM(x,ypage,0xff);
				x++;
			}
			x = x_min;
			ypage++;
			y_min+=8;
		}
		//处理结束ypage
		else
		{
			while(x <= x_max)
			{
				lcd_driver.Write_GRAM(x,ypage,cal_1[offset]);
				x++;
			}
			break;
		}
	}
}

/*--------------------------------------------------------------
  * 名称: lcd_draw_box(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max)
  * 传入: (x_min,y_min)起点 (x_max,y_max)终点
  * 功能: 绘制矩形
----------------------------------------------------------------*/
void lcd_draw_box(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max)
{
	Lcd_Draw_VLine(x_min,y_min,y_max);
	Lcd_Draw_VLine(x_max,y_min,y_max);
	Lcd_Draw_HLine(x_min,y_min,x_max);
	Lcd_Draw_HLine(x_min,y_max,x_max);
}

/*--------------------------------------------------------------
  * 名称: lcd_draw_bitmap(int16_t x0,int16_t y0,uint16_t sizex,uint8_t sizey,uint8_t BMP[])
  * 传入1: x0 坐标左上角横坐标点
	* 传入2: y0 坐标左上角纵坐标点
  * 传入3: sizex 点阵图形x宽度
  * 传入4: sizey 点阵图形y高度
  * 传入5: BMP[] 点阵图形数组地址
  * 功能: 将点阵图形摆放到任意坐标点上
  * 说明: 坐标点支持负数
----------------------------------------------------------------*/
//__attribute__ ((section ("RAMCODE"))) //将该代码搬运到RAM运行, 能实现零等待高速运行
void lcd_draw_bitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,const uint8_t BMP[])
{
	uint16_t xi;
	int16_t ymax;

	if((x0+sizex<0)||(x0>(SCREEN_WIDTH-1)))
	{
		return;
	}

	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	ymax = y0 + sizey;
	//此处可优化
	//---------单页缓存-----------
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
		BMP += sizex;
		y0 +=8;
	}
	#endif

	while(y0<ymax)
	{
		for(xi=0;xi<sizex;xi++)
		{
			int16_t x=x0+xi;
			if(x<SCREEN_WIDTH)
			{
				gram_draw_one_byte(x,y0,*BMP);
			}
			BMP++;
		}
		y0+=8;
	}
}

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
//__attribute__ ((section ("RAMCODE"))) //将该代码搬运到RAM运行, 能实现零等待高速运行
void lcd_draw_RLEbitmap(int16_t x0,int16_t y0,uint16_t sizex,uint16_t sizey,const uint8_t RLEBMP[])
{
	uint8_t rle_num;
	uint8_t rle_dat;
	uint16_t xmax;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	//此处可优化
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	uint16_t ymax;
	if((y0 >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))||(y0+sizey < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	ymax = (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8;
	#endif

	if((x0+sizex<0)||(x0>(SCREEN_WIDTH-1)))
	{
		return;
	}
	xmax = x0 + sizex;
	int16_t y=y0;
	int16_t x=x0;
	while(1)
  {
      rle_num = *RLEBMP++;
      if(rle_num==0x00)
      {
          break;//结束
      }
      rle_dat = *RLEBMP++;
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
}

/*--------------------------------------------------------------
  * 名称: lcd_fill_rbox(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max, int8_t r)
  * 传入: (x_min,y_min)起点 (x_max,y_max)终点 r:半径
  * 功能: 填充倒圆角矩形
----------------------------------------------------------------*/
void lcd_fill_rbox(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max, int8_t r)
{
	int16_t x_min_add_r;
	int16_t x_max_dec_r;
	int16_t y_min_add_r;
	int16_t y_max_dec_r;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if(y_max < ((lcd_driver.lcd_refresh_ypage*8)) || (y_min > ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8)))
	{
		return;
	}
	#endif
	if((x_min>=x_max)||(y_min>=y_max)){return;}//错误值,不绘画
	if(r<0){r=0;}
	while((x_max - x_min)<(r*2)){r--;}
	while((y_max - y_min)<(r*2)){r--;}
	x_min_add_r = x_min + r;
	x_max_dec_r = x_max - r;
	y_min_add_r = y_min + r;
	y_max_dec_r = y_max - r;
	lcd_fill_circel_part(x_min_add_r,y_min_add_r,r,C_QLU);//左上
	lcd_fill_circel_part(x_max_dec_r,y_min_add_r,r,C_QRU);//右上
	lcd_fill_circel_part(x_min_add_r,y_max_dec_r,r,C_QLD);//左下
	lcd_fill_circel_part(x_max_dec_r,y_max_dec_r,r,C_QRD);//右下
	#if ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if(y_max > ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))
	{
		y_max = (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8;
	}
	if(y_min < (lcd_driver.lcd_refresh_ypage*8))
	{
		y_min = lcd_driver.lcd_refresh_ypage*8;
	}
	#endif
	//左长条
	lcd_fill_box	(x_min,
								y_min_add_r,
								x_min_add_r,
								y_max_dec_r);
	//中间长条
	lcd_fill_box	(x_min_add_r,
								y_min,
								x_max_dec_r,
								y_max);
	//右长条
	lcd_fill_box	(x_max_dec_r,
								y_min_add_r,
								x_max,
								y_max_dec_r);

	//---专门处理用于反色驱动---
	if((lcd_driver.Write_GRAM == lcd_driver_Write_inv)||
		(lcd_driver.Write_GRAM == lcd_driver_Write_inv_inBox))
	{
		Lcd_Draw_HLine(x_min,y_min_add_r,x_min_add_r);
		Lcd_Draw_HLine(x_max_dec_r,y_min_add_r,x_max);
		Lcd_Draw_HLine(x_min,y_max_dec_r,x_min_add_r);
		Lcd_Draw_HLine(x_max_dec_r,y_max_dec_r,x_max);
		Lcd_Draw_VLine(x_min_add_r,y_min,y_max);
		Lcd_Draw_VLine(x_max_dec_r,y_min,y_max);
	}
}

/*--------------------------------------------------------------
  * 名称: lcd_draw_rbox(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max, uint16_t r)
  * 传入: (x_min,y_min)起点 (x_max,y_max)终点 r:半径
  * 功能: 绘制倒圆角镂空矩形
----------------------------------------------------------------*/
void lcd_draw_rbox(int16_t x_min,int16_t y_min, int16_t x_max, int16_t y_max, uint16_t r)
{
	int16_t x_min_add_r;
	int16_t x_max_dec_r;
	int16_t y_min_add_r;
	int16_t y_max_dec_r;
	if(y_max < y_min)
	{
		int8_t temp = y_min;
		y_min = y_max;
		y_max = temp;
	}
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((y_min >= ((lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))||(y_max < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	#endif
	if(x_max < x_min)
	{
		int8_t temp = x_min;
		x_min = x_max;
		x_max = temp;
	}
	if(x_min>=x_max){return;}//错误值,不绘画

	while((x_max - x_min)<(r*2)){r--;}
	while((y_max - y_min)<(r*2)){r--;}
	//if(r<0){r=0;}

	x_min_add_r = x_min + r;
	x_max_dec_r = x_max - r;
	y_min_add_r = y_min + r;
	y_max_dec_r = y_max - r;

	lcd_draw_circel_part(x_min_add_r,y_min_add_r,r,C_QLU);//左上
	lcd_draw_circel_part(x_max_dec_r,y_min_add_r,r,C_QRU);//右上
	lcd_draw_circel_part(x_min_add_r,y_max_dec_r,r,C_QLD);//左下
	lcd_draw_circel_part(x_max_dec_r,y_max_dec_r,r,C_QRD);//右下

	Lcd_Draw_VLine(x_min,y_min_add_r,y_max_dec_r);
	Lcd_Draw_VLine(x_max,y_min_add_r,y_max_dec_r);
	Lcd_Draw_HLine(x_min_add_r,y_min,x_max_dec_r);
	Lcd_Draw_HLine(x_min_add_r,y_max,x_max_dec_r);

	//---专门处理用于反色驱动---
	if((lcd_driver.Write_GRAM == lcd_driver_Write_inv)||
		(lcd_driver.Write_GRAM == lcd_driver_Write_inv_inBox))
	{
		lcd_draw_pixl(x_min,y_min_add_r);
		lcd_draw_pixl(x_min,y_max_dec_r);
		lcd_draw_pixl(x_min_add_r,y_min);
		lcd_draw_pixl(x_min_add_r,y_max);
		lcd_draw_pixl(x_max_dec_r,y_min);
		lcd_draw_pixl(x_max_dec_r,y_max);
		lcd_draw_pixl(x_max,y_min_add_r);
		lcd_draw_pixl(x_max,y_max_dec_r);
	}
}

/*--------------------------------------------------------------
  * 名称: lcd_draw_ascii(int16_t x,int16_t y,char chr)
  * 传入: (x,y)左上角坐标 char:字符
  * 功能: 绘制一个Ascii字符
----------------------------------------------------------------*/
void lcd_draw_ascii(int16_t x,int16_t y,char chr)
{
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((y > (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8)|| ((y + lcd_driver.fonts_ASCII->high) < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	#endif

	//不存在的值
	if((chr<0x20)||(chr>=0x7F))
	{
		return;
	}
	switch(lcd_driver.fonts_ASCII->store_type)
	{
		case ASCII_IN_MCU:
		{
			lcd_draw_bitmap(
			x,//左上角坐标x
			y,//左上角坐标y
			lcd_driver.fonts_ASCII->width,//字体宽度
			lcd_driver.fonts_ASCII->high ,//字体高度
			(uint8_t*)lcd_driver.fonts_ASCII->store_par.IN_MCU_par.font + lcd_driver.fonts_ASCII->store_par.IN_MCU_par.byte_size * (chr-0x20));
		}break;
		case ASCII_IN_EX_FLASH :return;
	}
}

/*--------------------------------------------------------------
  * 名称: void lcd_draw_int32(int16_t x,int16_t y,int32_t num)//写数字,自动长度,32位带符号
  * 传入: (x,y)左上角坐标 num带符号16位数字
  * 功能: 根据输入的num数字用对应的"ASCII字库"绘制到屏幕对应坐标上
----------------------------------------------------------------*/
void lcd_draw_int32(int16_t x,int16_t y,int32_t num)
{
	uint32_t sum;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存

	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((y > (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8)|| ((y + lcd_driver.fonts_ASCII->high) < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	#endif

	sum=1;
	if(num < 0)
	{
		lcd_draw_ascii(x,y,'-');
		x += lcd_driver.fonts_ASCII->width + lcd_driver.fonts_ASCII->scape;
		num = -num;
	}
	else if(num == 0)
	{
		lcd_draw_ascii(x,y,'0');
		return;
	}
	else if(num == 1)
	{
		lcd_draw_ascii(x,y,'1');
		return;
	}
	while(sum <= (uint16_t)num)
	{
		sum = sum*10;
	}
	while(sum>1)
	{
		sum=sum/10;
		lcd_draw_ascii(x,y,+ '0'+num/sum);
		x += lcd_driver.fonts_ASCII->width + lcd_driver.fonts_ASCII->scape;
		num = (uint16_t)num % sum;
	}
}



/*--------------------------------------------------------------
  * 名称: void lcd_draw_unicode(int16_t x,int16_t y,unicode_t unicode_id)
  * 传入: (x,y)左上角坐标 unicode_id
  * 功能: 根据输入的unicode_id寻找对应的"裁剪字库"绘制到屏幕坐标上
----------------------------------------------------------------*/
void lcd_draw_unicode(int16_t x,int16_t y,unicode_t unicode_id)
{
	uint16_t i;
	uint8_t* p;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存

	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((y > (lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8) || ((y + lcd_driver.fonts_UTF8->high) < (lcd_driver.lcd_refresh_ypage*8)))
	{
		return;
	}
	#endif

	switch(lcd_driver.fonts_UTF8->store_type)
	{
		case UTF8_IN_MCU_INDEX:
		{
			i = 0;
			p = lcd_driver.fonts_UTF8->store_par.IN_MCU_INDEX_par.unicode_index;
			while((*p!=0x00)/*&&(lcd_driver.fonts_UTF8->unicode_index[i+1]!=0x00)*/)
			{
				//-----------------A方式大小端读取方式根据MCU储存方式选择-------------------
				//if((*p == unicode_id.u8[1]) && (*(p+1) == unicode_id.u8[0]))
				//{
				//	break;
				//}

				//-----------------B方式大小端读取方式根据MCU储存方式选择-------------------
				if((*p == unicode_id.u8[0]) && (*(p+1) == unicode_id.u8[1]))
				{
					break;
				}

				p+=2;
				i++;
			}

			if((*p!=0x00)/*&&(lcd_driver.fonts_UTF8->unicode_index[i+1]!=0x00)*/)
			{
				lcd_draw_bitmap(
				x,
				y,
				lcd_driver.fonts_UTF8->width,
				lcd_driver.fonts_UTF8->high,
				lcd_driver.fonts_UTF8->store_par.IN_MCU_INDEX_par.font + lcd_driver.fonts_UTF8->store_par.IN_MCU_INDEX_par.byte_size*i);
			}
		}break;
		case UTF8_IN_EX_FLASH_INDEX:
		case UTF8_IN_EX_FLASH_CONTINUOUS:
		default: break;
	}

}


/*--------------------------------------------------------------
  * 名称: void lcd_draw_utf8_string(int16_t x,int16_t y,uint8_t *p)
  * 传入: (x,y)左上角坐标 *p字符串指针
  * 功能: 在指定坐标上按照系统设定字体格式绘制字符串
----------------------------------------------------------------*/
void lcd_draw_utf8_string(int16_t x,int16_t y,char *p)
{
	volatile int16_t x_0;
	//---------全屏缓存-----------
	#if ((LCD_MODE == _FULL_BUFF_FULL_UPDATE) || ((LCD_MODE == _FULL_BUFF_DYNA_UPDATE)))//全屏缓存
	if((x>SCREEN_WIDTH)||(y>(SCREEN_HIGH-1)))
	{
		return;
	}
	x_0 = x;
	while((*p!=0x00)&&(y<SCREEN_HIGH))
	//---------单页缓存-----------
	#elif ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))//页缓存
	if((x>SCREEN_WIDTH)||(y>(lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))
	{
		return;
	}
	x_0 = x;
	while((*p!=0x00)&&(y<(lcd_driver.lcd_refresh_ypage+GRAM_YPAGE_NUM)*8))
	#endif
	{
		uint8_t temp = *p & 0xF8;
		if(temp<=0x7F)//单字节
		//if(temp < 0xC0)
		{
			//单字节均是ASCII
			if(*p == '\r')
			{
				x = x_0;
				y += lcd_driver.newline_high;
				p+=1;
				if(*p == '\n'){p+=1;}
			}
			else if(*p == '\n')
			{
				x = x_0;
				y += lcd_driver.newline_high;
				p+=1;
			}
			else
			{
				lcd_draw_ascii(x,y,*p);
				x += lcd_driver.fonts_ASCII->width + lcd_driver.fonts_ASCII->scape;
				p+=1;
			}
		}
		else if(temp < 0xE0)//双字节
		{
			//双字节好像都是符号
			/*
			u16_u8_t unicode_id;
			unicode_id.u8[0]=(*p<<3)|((*(p+1)>>3)&0x07);
			unicode_id.u8[1]=(*(p+1)&0x07);
			lcd_draw_unicode(x,y,unicode_id);
			x += Wegui_font.fonts_HZ_With + Wegui_font.fonts_HZ_scape;
			*/
			p+=2;
		}
		else if(temp < 0xF0)//三字节(都是中文)
		{
			//三字节都是中文
			unicode_t unicode_id;
			unicode_id.u8[0]=(*p<<4)|((*(p+1)>>2)&0x0F);
			unicode_id.u8[1]=(*(p+1)<<6)|((*(p+2))&0x3F);
			lcd_draw_unicode(x,y,unicode_id);
			x += lcd_driver.fonts_UTF8->width + lcd_driver.fonts_UTF8->scape;
			p+=3;
		}
		else//四字节
		{
			p+=4;
		}
	}
}

/*--------------------------------------------------------------
  * 名称: lcd_get_utf8_string_xlen(char *p)
  * 传入: *p字符串指针
  * 功能: 返回字符串在系统字体上x方向的总像素长度
----------------------------------------------------------------*/
uint16_t lcd_get_utf8_string_xlen(char *p)
{
	uint16_t len=0;
	uint16_t temp_len=0;
	uint16_t endscape=0;
	while(1)
	{
		uint8_t temp = *p & 0xF8;
		if(*p==0x00)
		{
			if(temp_len>endscape)
			{
				temp_len -= endscape;
			}
			if(len < temp_len){len = temp_len;}
			temp_len = 0;
			break;
		}
		else if(*p == '\r')
		{
			p+=1;
			if(*p == '\n'){p+=1;}
			if(temp_len>endscape)
			{
				temp_len -= endscape;
			}
			if(len < temp_len){len = temp_len;}
			temp_len = 0;
		}
		else if(*p == '\n')
		{
			p+=1;
			if(temp_len>endscape)
			{
				temp_len -= endscape;
			}
			if(len < temp_len){len = temp_len;}
			temp_len = 0;
		}

		else if(temp<=0x7F)//单字节
		//if(temp < 0xC0)
		{
			//单字节均是ASCII
			temp_len += lcd_driver.fonts_ASCII->width+lcd_driver.fonts_ASCII->scape;

			endscape=lcd_driver.fonts_ASCII->scape;
			p+=1;
		}
		else if(temp < 0xE0)//双字节
		{
			//双字节好像都是符号
			/*
			u16_u8_t unicode_id;
			unicode_id.u8[0]=(*p<<3)|((*(p+1)>>3)&0x07);
			unicode_id.u8[1]=(*(p+1)&0x07);
			lcd_draw_unicode(x,y,unicode_id);
			x += Wegui_font.fonts_HZ_With + Wegui_font.fonts_HZ_scape;
			*/
			p+=2;
		}
		else if(temp < 0xF0)//三字节(都是中文)
		{
			//三字节都是中文
			temp_len += lcd_driver.fonts_UTF8->width + lcd_driver.fonts_UTF8->scape;
			endscape=lcd_driver.fonts_UTF8->scape;
			p+=3;
		}
		else//四字节
		{
			p+=4;
		}
	}
	return len;
}

/*--------------------------------------------------------------
  * 名称: lcd_get_utf8_yline(int16_t x,int16_t y,uint8_t chr)
  * 传入: *p字符串指针
  * 功能: 返回UTF8字符串换行的次数
----------------------------------------------------------------*/
uint16_t lcd_get_utf8_yline(char *p)
{
	uint16_t line=1;
	if (p == 0x00)
    {
     return  0;
    }
	while(1)
	{
		uint8_t temp = *p & 0xF8;
		if(*p==0x00)
		{
			break;
		}
		if(*p == '\r')
		{
			p+=1;
			if(*p == '\n'){p+=1;}
			line++;
		}
		else if(*p == '\n')
		{
			p+=1;
			line++;
		}
		else if(temp<=0x7F)//单字节
		//else if(temp < 0xC0)
		{
			//单字节均是ASCII
			p+=1;
		}
		else if(temp < 0xE0)//双字节
		{
			p+=2;
		}
		else if(temp < 0xF0)//三字节(都是中文)
		{
			p+=3;
		}
		else//四字节
		{
			p+=4;
		}
	}
	return line;
}

/*--------------------------------------------------------------
  * 名称: lcd_clear_gram(void)
	* 功能: 清显存,全部清为0x00
----------------------------------------------------------------*/
void lcd_clear_gram(void)
{
	memset(lcd_driver.LCD_GRAM, 0, sizeof(lcd_driver.LCD_GRAM));
}

/*--------------------------------------------------------------
  * 名称: lcd_fill_gram(uint8_t n)
	* 功能: 显存全部刷成"笔刷n"(lcd_driver.colour[n])的颜色
----------------------------------------------------------------*/
void lcd_fill_gram(uint8_t n)
{
	unsigned int i,j;
	uint8_t k;
	if(n&0x01){k=0xff;}
	else{k=0x00;}
	for(i=0;i<GRAM_YPAGE_NUM;i++)
	{
			for(j=0;j<SCREEN_WIDTH;j++)
			{
				lcd_driver.LCD_GRAM[i][j][0]=k;
			}
	}
	#if (LCD_COLOUR_BIT >= 2)
	if(n&0x02){k=0xff;}
	else{k=0x00;}
	for(i=0;i<GRAM_YPAGE_NUM;i++)
	{
			for(j=0;j<SCREEN_WIDTH;j++)
			{
				lcd_driver.LCD_GRAM[i][j][1]=k;
			}
	}
	#endif
	#if (LCD_COLOUR_BIT >= 3)
	if(n&0x04){k=0xff;}
	else{k=0x00;}
	for(i=0;i<GRAM_YPAGE_NUM;i++)
	{
			for(j=0;j<SCREEN_WIDTH;j++)
			{
				lcd_driver.LCD_GRAM[i][j][2]=k;
			}
	}
	#endif
}

/*--------------------------------------------------------------
  * 名称: lcd_driver_init()
  * 功能: 驱动初始化
----------------------------------------------------------------*/
void lcd_driver_init(void)
{
	//-------------初始化-------------
	#if(LCD_PORT!=0)
	lcd_port_init();//屏幕
	#endif
	
	#if(FLASH_PORT!=0)
	flash_port_init();//外挂flash
	#endif

	//------driver配置默认字体---------
	//---中英文字体high高度建议一致----
	lcd_driver.fonts_ASCII = STARTUP_FONTS_ASCII;//默认ASCII字体
	lcd_driver.fonts_UTF8 = STARTUP_FONTS_UTF8;//默认UTF8字体(裁切)
	lcd_driver.newline_high = lcd_driver.fonts_UTF8->high;//文本换行距离(选择ASCII字体和UTF8字体最大的一个)

#	if(LCD_TYPE == LCD_RGB565)
	//设置默认笔刷颜色 RGB屏幕在程序运行过程中允许修改笔刷颜色
#		if (LCD_COLOUR_BIT>=1)
			rgb_set_driver_colour(0,0x0000);//设置write_0驱动的笔刷颜色
			rgb_set_driver_colour(1,0xFFFF);//设置write_1驱动的笔刷颜色
#		endif
#		if (LCD_COLOUR_BIT>=2)
			rgb_set_driver_colour(2,0xFFFF);//设置write_2驱动的笔刷颜色
			rgb_set_driver_colour(3,0xFFFF);//设置write_3驱动的笔刷颜色
#		endif
#		if (LCD_COLOUR_BIT>=3)
			rgb_set_driver_colour(4,0xFFFF);//设置write_4驱动的笔刷颜色
			rgb_set_driver_colour(5,0xFFFF);//设置write_5驱动的笔刷颜色
			rgb_set_driver_colour(6,0xFFFF);//设置write_6驱动的笔刷颜色
			rgb_set_driver_colour(7,0xFFFF);//设置write_7驱动的笔刷颜色
#		endif
#	endif

	//----------选择默认"笔刷"-----------
	//lcd_set_driver_mode(write_0);            //写0模式(普通) 单色时黑点模式 多色时笔刷颜色为lcd_driver.colour[0]
	lcd_set_driver_mode(write_1);              //写1模式(普通) 单色时白点模式 多色时笔刷颜色为lcd_driver.colour[1]
	//lcd_set_driver_mode(write_2);            //写2模式(普通) 多色时笔刷颜色为lcd_driver.colour[2]
	//lcd_set_driver_mode(write_3);            //写3模式(普通) 多色时笔刷颜色为lcd_driver.colour[3]
	//lcd_set_driver_mode(write_4);            //写4模式(普通) 多色时笔刷颜色为lcd_driver.colour[4]
	//lcd_set_driver_mode(write_5);            //写5模式(普通) 多色时笔刷颜色为lcd_driver.colour[5]
	//lcd_set_driver_mode(write_6);            //写6模式(普通) 多色时笔刷颜色为lcd_driver.colour[6]
	//lcd_set_driver_mode(write_7);            //写7模式(普通) 多色时笔刷颜色为lcd_driver.colour[7]
	//lcd_set_driver_mode(write_inverse);      //反写模式(普通) 单色时0->1或1->0 2位色时0(0B00)->3(0B11) 或1(0B01)->2(0B10)... 3位色时0(0B000)->7(0B111)或2(0B001)->6(0B110)...

	//lcd_set_driver_mode(write_0_inBox);      //限制区域内写0模式(高级) 单色时黑点模式 多色笔刷颜色为lcd_driver.colour[0]
	//lcd_set_driver_mode(write_1_inBox);      //限制区域内写1模式(高级) 单色时白点模式 多色笔刷颜色为lcd_driver.colour[1]
	//lcd_set_driver_mode(write_2_inBox);      //限制区域内写1模式(高级) 多色时笔刷颜色为lcd_driver.colour[2]
	//lcd_set_driver_mode(write_3_inBox);      //限制区域内写1模式(高级) 多色时笔刷颜色为lcd_driver.colour[3]
	//lcd_set_driver_mode(write_4_inBox);      //限制区域内写1模式(高级) 多色时笔刷颜色为lcd_driver.colour[4]
	//lcd_set_driver_mode(write_5_inBox);      //限制区域内写1模式(高级) 多色时笔刷颜色为lcd_driver.colour[5]
	//lcd_set_driver_mode(write_6_inBox);      //限制区域内写1模式(高级) 多色时笔刷颜色为lcd_driver.colour[6]
	//lcd_set_driver_mode(write_7_inBox);      //限制区域内写1模式(高级) 多色时笔刷颜色为lcd_driver.colour[7]
	//lcd_set_driver_mode(write_inverse_inBox);//限制区域内反写模式(高级) 单色时0->1或1->0 2位色时0(0B00)->3(0B11)或1(0B01)->2(0B10)... 3位色时0(0B000)->7(0B111)或2(0B001)->6(0B110)...

	lcd_set_driver_box(0,0,SCREEN_WIDTH-1,SCREEN_HIGH-1);//设置"高级"驱动的"限制区域"


	//-----------显存初始化------------
	lcd_clear_gram();
	#if ((LCD_MODE == _PAGE_BUFF_FULL_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))
		//页缓存专用变量初始化
		lcd_driver.lcd_refresh_ypage=0;//页缓存专用变量, 记录当前刷的是第几页
	#endif
	#if((LCD_MODE == _FULL_BUFF_DYNA_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))
		//动态刷新才有crc
		lcd_reset_crc();//强制刷新一下crc动态刷新值, 保证下次会全刷一遍, 保证颜色正确
	#endif

	//-----------刷新屏幕------------
	while(LCD_Refresh()!=0){}
	lcd_bl_on();//开背光灯
		
}


