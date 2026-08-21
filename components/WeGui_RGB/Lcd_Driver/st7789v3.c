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

#if (LCD_IC == _ST7789V3)
#include "st7789v3.h"

void ST7789V3_Set_Addr(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	uint8_t i[]={0x2a,x1>>8,x1&0xff,x2>>8,x2&0xff};
	uint8_t j[]={0x2b,y1>>8,y1&0xff,y2>>8,y2&0xff};
	const uint8_t k[]={0x2c};
	lcd_send_nCmd((uint8_t*)i,sizeof(i)/sizeof(uint8_t));
	lcd_send_nCmd((uint8_t*)j,sizeof(j)/sizeof(uint8_t));
	lcd_send_nCmd((uint8_t*)k,sizeof(k)/sizeof(uint8_t));
}

void ST7789V3_Clear()//清除IC显示缓存
{
	uint32_t i;
	ST7789V3_Set_Addr(0,0,320-1,320-1);
	i=0;
	while(i++<(uint32_t)320*320)
	{
		lcd_send_1Dat(0x33);lcd_send_1Dat(0x33);//测试
		//lcd_send_1Dat(0x00);lcd_send_1Dat(0x00);
	}
}

/*--------------------------------------------------------------
  * 名称: ST7789V3_Init()
  * 传入: 无
  * 返回: 无
  * 功能: 初始化屏幕
  * 说明: 推荐更改为屏幕资料中的初始化指令
----------------------------------------------------------------*/
void ST7789V3_Init()
{

//	LCD_RES_Clr();
//	lcd_delay_ms(100);
//	LCD_RES_Set();
//	lcd_delay_ms(100);

	lcd_send_1Cmd(0x11); 
//	delay_1ms(120); 
	lcd_send_1Cmd(0x36); 
	lcd_send_1Dat(0x00);//方向1
	//lcd_send_1Dat(0xC0);//方向2
	//lcd_send_1Dat(0x70);//方向3
	//lcd_send_1Dat(0xA0);//方向4

	lcd_send_1Cmd(0x3A);
	lcd_send_1Dat(0x05);

	lcd_send_1Cmd(0xB2);
	lcd_send_1Dat(0x0C);
	lcd_send_1Dat(0x0C);
	lcd_send_1Dat(0x00);
	lcd_send_1Dat(0x33);
	lcd_send_1Dat(0x33); 

	lcd_send_1Cmd(0xB7); 
	lcd_send_1Dat(0x35);  

	lcd_send_1Cmd(0xBB);
	lcd_send_1Dat(0x35);

	lcd_send_1Cmd(0xC0);
	lcd_send_1Dat(0x2C);

	lcd_send_1Cmd(0xC2);
	lcd_send_1Dat(0x01);

	lcd_send_1Cmd(0xC3);
	lcd_send_1Dat(0x13);   

	lcd_send_1Cmd(0xC4);
	lcd_send_1Dat(0x20);  

	lcd_send_1Cmd(0xC6); 
	lcd_send_1Dat(0x0F);    

	lcd_send_1Cmd(0xD0); 
	lcd_send_1Dat(0xA4);
	lcd_send_1Dat(0xA1);

	lcd_send_1Cmd(0xD6); 
	lcd_send_1Dat(0xA1);

	lcd_send_1Cmd(0xE0);
	lcd_send_1Dat(0xF0);
	lcd_send_1Dat(0x00);
	lcd_send_1Dat(0x04);
	lcd_send_1Dat(0x04);
	lcd_send_1Dat(0x04);
	lcd_send_1Dat(0x05);
	lcd_send_1Dat(0x29);
	lcd_send_1Dat(0x33);
	lcd_send_1Dat(0x3E);
	lcd_send_1Dat(0x38);
	lcd_send_1Dat(0x12);
	lcd_send_1Dat(0x12);
	lcd_send_1Dat(0x28);
	lcd_send_1Dat(0x30);

	lcd_send_1Cmd(0xE1);
	lcd_send_1Dat(0xF0);
	lcd_send_1Dat(0x07);
	lcd_send_1Dat(0x0A);
	lcd_send_1Dat(0x0D);
	lcd_send_1Dat(0x0B);
	lcd_send_1Dat(0x07);
	lcd_send_1Dat(0x28);
	lcd_send_1Dat(0x33);
	lcd_send_1Dat(0x3E);
	lcd_send_1Dat(0x36);
	lcd_send_1Dat(0x14);
	lcd_send_1Dat(0x14);
	lcd_send_1Dat(0x29);
	lcd_send_1Dat(0x32);
	
// 	lcd_send_1Cmd(0x2A);
//	lcd_send_1Dat(0x00);
//	lcd_send_1Dat(0x22);
//	lcd_send_1Dat(0x00);
//	lcd_send_1Dat(0xCD);
//	lcd_send_1Dat(0x2B);
//	lcd_send_1Dat(0x00);
//	lcd_send_1Dat(0x00);
//	lcd_send_1Dat(0x01);
//	lcd_send_1Dat(0x3F);
//	lcd_send_1Cmd(0x2C);
	lcd_send_1Cmd(0x21); 
  
  lcd_send_1Cmd(0x11);
  lcd_delay_ms(120);	
	lcd_send_1Cmd(0x29); 
	ST7789V3_Clear();//清除IC显示缓存
}
#endif
