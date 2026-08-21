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
/*************************************************
* 1.需要多国语言显示必须使用UTF8编码
* Edit->Configuration->Encoding->"Encod in UTF8"
*
* 2.为防止中文字符串出现编译问题,
* 在ARMV5编译器下,需要调整编译规则
* Project -> Oprions for Target-> C/C++ ->
* Misc Contiols -> 填入"--no-multibyte-chars"
*************************************************/
#include "wegui_menu_demo.h"
#include "flash_driver.h"
#include "flash_img_demo.h"

uint8_t demo_bool=1;
uint16_t demo_value=0;
uint32_t demo_timer=0;
int16_t slider_demo_value=-5;
char demo_string_1[7] = "";
char demo_string_2[7] = "";
char demo_string_3[3] = "--";

//------------------------------------------蜂鸣器音乐播放器--------------------------------------------
void MusicPlayer_App_Begin()//进入APP执行一次
{
	demo_timer = 0;
}
void MusicPlayer_App_Loop()//相当于在主循环执行
{

}
void MusicPlayer_App_Quit()//退出菜单执行一次
{

}

uint8_t MusicPlayer_APP_Refresh(uint8_t ui_farmes,uint16_t time_count)//刷新屏幕时执行,放绘图函数
{
	uint32_t i;
	uint16_t hour,minute,sec,ms;
	char string[12];
	
	(void)ui_farmes;//防止警告
	//(void)time_count;//防止警告

	//--------播放计时-------
	#if ((WEGUI_PORT != 0)&&((WEGUI_PORT == _STM32F103X_4KEY_BZ_PORT)||(WEGUI_PORT == _STM32F103X_EC_BZ_PORT)))
	if(buzz_music_player_is_busy())//使用了蜂鸣器相关接口
	#else
	if(0)
	#endif
	{
		demo_timer+=time_count;
	}
	else
	{
		demo_timer = 0;
	}

	//------字符串处理-----
	

	i = demo_timer;
	hour = i / 3600000;
	i %= 3600000;
	minute = i / 60000;
	i %= 60000;
	sec = i / 1000;
	i %= 1000;
	ms = i;

	string[0]='0' + hour/10;
	string[1]='0' + hour%10;
	string[3]='0' + minute/10;
	string[4]='0' + minute%10;
	string[6]='0' + sec/10;
	string[7]='0' + sec%10;
	string[8]=0;
	if(ms > 500)
	{
		string[2]=' ';
		string[5]=' ';
	}
	else
	{
		string[2]=':';
		string[5]=':';
	}

	//---绘图---
	{
		//0.---等待发送完毕---
		while(lcd_is_busy()!=0);//DMA方式专用,其他模式可省略
		//1.---清空缓存---
		lcd_clear_gram();
		//2.-----绘图-----
		lcd_set_driver_mode(write_1);//选择笔刷颜色1
		lcd_draw_utf8_string(0,0,"Music Player");
		lcd_draw_utf8_string(0,lcd_driver.newline_high,string);

		#if ((WEGUI_PORT != 0)&&((WEGUI_PORT == _STM32F103X_4KEY_BZ_PORT)||(WEGUI_PORT == _STM32F103X_EC_BZ_PORT)))
		if(buzz_music_player_is_busy())//使用了蜂鸣器相关接口
		#else
		if(0)
		#endif
		{
			lcd_draw_utf8_string(0,lcd_driver.newline_high*2,"Playing");
		}
		else
		{
			lcd_draw_utf8_string(0,lcd_driver.newline_high*2,"Pause");
		}
	}

	return 1;//返回1正常刷屏 返回0不刷屏(0可在上方强制显示一些gui外的界面)
}

//------------------------------------------矩形碰碰车游戏--------------------------------------------
uint8_t en_save;
void Game_App_Begin()//进入APP执行一次
{
	demo_timer = 0;
	en_save = wegui.setting.debug_windows_en;
	wegui.setting.debug_windows_en = 0;
}
void Game_App_Loop()//相当于在主循环执行
{

}
void Game_App_Quit()//退出菜单执行一次
{
	wegui.setting.debug_windows_en = en_save;
}

#if ((WEGUI_PORT != 0)&&((WEGUI_PORT == _STM32F103X_4KEY_BZ_PORT)||(WEGUI_PORT == _STM32F103X_EC_BZ_PORT)))
extern const bz_Music_t game_ctrl[];//碰撞音效
#endif

uint8_t Game_APP_Refresh(uint8_t ui_farmes,uint16_t time_count)//刷新屏幕时执行,放绘图函数
{
	//--设置矩形大小--
	#if (SCREEN_HIGH<SCREEN_WIDTH)
		#define GMAE_BOX_WIDTH (SCREEN_HIGH/3)
	#else
		#define GMAE_BOX_WIDTH (SCREEN_WIDTH/3)
	#endif
	#define GAME_BOX_HIGHT (GMAE_BOX_WIDTH)

	static uint8_t xdir=0;
	static uint8_t ydir=0;
	static int16_t x=SCREEN_WIDTH/3;
	static int16_t y=0;
	static uint32_t i=0;
	static uint8_t colour=0;
	static uint16_t timer=0;
	uint8_t t;
	
	(void)ui_farmes;//防止警告 运行过的帧数
	//(void)time_count;//防止警告 运行过的时间
	
	timer+=time_count;//时间累加
	t = timer/6;//6ms
	timer %= 6;
	
	//移动矩形
	while(t)//6ms移动一次
	{
		t--;
		if(xdir){x++;}else{x--;}
		if(ydir){y++;}else{y--;}
		#if ((WEGUI_PORT != 0)&&((WEGUI_PORT == _STM32F103X_4KEY_BZ_PORT)||(WEGUI_PORT == _STM32F103X_EC_BZ_PORT)))
		//碰撞音效
		if(x>=SCREEN_WIDTH-GMAE_BOX_WIDTH-1){xdir=0;i++;colour++;buzz_play_music(game_ctrl);}else if(x<0){xdir=1;i++;colour++;buzz_play_music(game_ctrl);}
		if(y>=SCREEN_HIGH-GAME_BOX_HIGHT-1){ydir=0;i++;colour++;buzz_play_music(game_ctrl);}else if(y<0){ydir=1;i++;colour++;buzz_play_music(game_ctrl);}
		#else
		if(x>=SCREEN_WIDTH-GMAE_BOX_WIDTH-1){xdir=0;i++;colour++;}else if(x<0){xdir=1;i++;colour++;}
		if(y>=SCREEN_HIGH-GAME_BOX_HIGHT-1){ydir=0;i++;colour++;}else if(y<0){ydir=1;i++;colour++;}
		#endif
	}

	//显示矩形和数字
	{
		//0.---等待发送完毕---
		while(lcd_is_busy()!=0);//DMA方式专用,其他模式可省略
		//1.---清空缓存---
		lcd_clear_gram();
		//2.-----绘图-----
		#if(LCD_COLOUR_BIT == 2)
		switch(colour)
		{
			default:
			case 0:colour=1;
			case 1:lcd_set_driver_mode(write_1);break;
			case 2:lcd_set_driver_mode(write_2);break;
			case 3:lcd_set_driver_mode(write_3);break;
		}
		#elif(LCD_COLOUR_BIT >= 3)
		switch(colour)
		{
			default:
			case 0:colour=1;
			case 1:lcd_set_driver_mode(write_1);break;
			case 2:lcd_set_driver_mode(write_2);break;
			case 3:lcd_set_driver_mode(write_3);break;
			case 4:lcd_set_driver_mode(write_4);break;
			case 5:lcd_set_driver_mode(write_5);break;
			case 6:lcd_set_driver_mode(write_6);break;
			case 7:lcd_set_driver_mode(write_7);break;
		}
		#endif
		lcd_fill_box(x,y,x+GMAE_BOX_WIDTH, y+GAME_BOX_HIGHT);
		lcd_set_driver_mode(write_inverse);

		char str[11];
		my_itoa(i,str,10);//数值转10进制字符串, 传递回给字符串指针
		//居中显示
		lcd_draw_utf8_string	((SCREEN_WIDTH-(lcd_get_utf8_string_xlen(str)))/2,
													(SCREEN_HIGH-lcd_driver.fonts_ASCII->high)/2,
													str);
	}

	return 1;//返回1正常刷屏 返回0不刷屏(0可在上方强制显示一些gui外的界面)
}

//------------------------------------------flash视频播放器--------------------------------------------
void m_App_VideoPleayer_Begin()//进入APP执行一次
{
//	//--清屏--
//	lcd_clear_gram();
//	while(LCD_Refresh()!=0);
}
void m_App_VideoPleayer_Loop()//相当于在主循环执行
{

}
void m_App_VideoPleayer_Quit()//退出菜单执行一次
{
//	#if((LCD_MODE == _FULL_BUFF_DYNA_UPDATE) || (LCD_MODE == _PAGE_BUFF_DYNA_UPDATE))
//		lcd_reset_crc();//强制刷新一下crc动态刷新值, 确保能再全刷一遍, 保证颜色正确
//	#endif
//	lcd_clear_gram();
//	while(LCD_Refresh()!=0);
}

uint8_t m_App_VideoPleayer_Refresh(uint8_t ui_farmes,uint16_t time_count)//刷新屏幕时执行,放绘图函数
{
	(void)ui_farmes;//防止警告
	(void)time_count;//防止警告
	static uint16_t id=0;
	(void)id;//防止警告
	/*
		取模格式:[点阵]往下8位对齐,逐行扫描
		增加信息头√
	*/
	//0.---等待发送完毕---
	while(lcd_is_busy()!=0);//DMA方式专用,其他模式可省略
	//1.---清空缓存---
	lcd_clear_gram();
	#if(FLASH_PORT != _F_NO_PORT)
	flash_draw_img(0,0,img_flash_addr[id]);
	if(ui_farmes)
	{
		if(++id >= (sizeof(img_flash_addr)/sizeof(img_flash_addr[0])))
		{
			id=0;
		}
	}
	#endif
	return 1;//返回1正常刷屏 返回0不刷屏(0可在上方强制显示一些gui外的界面)
}

//------------------------------------------控件演示Demo--------------------------------------------
void m_wDemo_begin_fun()
{
	wegui_push_message_tip(2, "begin_fun();", 1000);//推送提示信息, (推送y位置, 提示内容字符串, 展示时间ms)
}
void m_wDemo_loop_fun()
{
	//菜单实时更新"demo_value"值显示到菜单
	my_itoa(demo_value,demo_string_1,10);//demo_value转成10进制带符号的字符串存进demo_string_1
}
void m_wDemo_quit_fun()
{
	wegui_push_message_tip(2, "quit_fun();", 1000);//推送提示信息, (推送y位置, 提示内容字符串, 展示时间ms)
}

void m_wDemo_wMessage_Pres_func()//菜单按下弹窗
{
	my_itoa(demo_value,demo_string_2,10);//demo_value转成10进制带符号的字符串存进demo_string_2
}

void m_wDemo_wMessage_Pres2_func()//菜单按下更换文字
{
	static uint8_t num=0;
	if(num < 2)
	{
		num++;
	}
	else
	{
		num=0;
	}

	switch(num)
	{
		case 0:
			m_wDemo_wMessage_Pres2.menuPar.wMessage_Par.Value_string[0]='A';
			m_wDemo_wMessage_Pres2.menuPar.wMessage_Par.Value_string[1]='a';
		break;
		case 1:
			m_wDemo_wMessage_Pres2.menuPar.wMessage_Par.Value_string[0]='B';
			m_wDemo_wMessage_Pres2.menuPar.wMessage_Par.Value_string[1]='b';
		break;
		case 2:
			m_wDemo_wMessage_Pres2.menuPar.wMessage_Par.Value_string[0]='C';
			m_wDemo_wMessage_Pres2.menuPar.wMessage_Par.Value_string[1]='c';
		break;
		case 3:
			m_wDemo_wMessage_Pres2.menuPar.wMessage_Par.Value_string[0]='D';
			m_wDemo_wMessage_Pres2.menuPar.wMessage_Par.Value_string[1]='d';
		break;
	}
}

//------------------------------------------UI&主题--------------------------------------------
int16_t theme_select = 1;
void set_theme(void)
{
	#if (LCD_COLOUR_BIT == 1)
		//未适配
	#elif (LCD_COLOUR_BIT == 2)
		const static uint16_t theme[6][4]=
		{
			//默认设置:
			//[0]背景色
			//[1]文字
			//[2]文字光标色/滑条数字色
			//[3]光标色/滑条进度色
			{0xfd80,0x4a48,0xffdb,0x3c57},//---配色1 大橘为重---
			{0x0000,0xFFFF,0x630c,0xFFFF},//---配色2 黑底白字---
			{0xFFFF,0x0000,0x9492,0x0000},//---配色3 白底黑字---
			{0xffbe,0x4395,0x9538,0x1169},//---配色4 白云晴空---
			{0x530d,0xefbe,0xc6bb,0x2104},//---配色5 深海蔚蓝---
			{0xd124,0x2986,0xef7d,0x3a09},//---配色6 血色记忆---
		};
		rgb_set_driver_colour(0,theme[theme_select-1][0]);
		rgb_set_driver_colour(1,theme[theme_select-1][1]);
		rgb_set_driver_colour(2,theme[theme_select-1][2]);
		rgb_set_driver_colour(3,theme[theme_select-1][3]);
	#elif (LCD_COLOUR_BIT == 3)
		//未适配
	#endif

}

//------------------------------------------设置亮度--------------------------------------------
#if (LCD_TYPE == LCD_RGB565)
void update_Wegui_screen_brightness()
{
	
}
#else
void update_Wegui_screen_brightness()
{
	lcd_set_bright(wegui.setting.brightness);
}
#endif

//------------------------------------------设置语言--------------------------------------------
void Set_language_Chinese()
{
	wegui.setting.language=zh_CN;
}
void Set_language_English()
{
	wegui.setting.language=en_US;
}



//-------------------m主菜单------------------------
menu_t m_main =
{
	.fatherMenu=0x00,//父菜单
	.subMenu=&m_wDemo,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="主菜单",//中文标题
	  .str_en_US="MAIN",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="",//中文描述
	  .str_en_US="",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};

menu_t m_wDemo =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=&m_wDemo_wMessage_Tip,//(首个)子菜单
	.nextMenu=&m_App_MusicPlayer,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="控件Demo",//中文标题
	  .str_en_US="wDemo",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="1.控件Demo",//中文描述
	  .str_en_US="1.Widget demo",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=m_wDemo_begin_fun,//菜单进入 执行一次
		.loop_fun=m_wDemo_loop_fun, //菜单功能 持续执行
		.quit_fun=m_wDemo_quit_fun, //菜单退出 执行一次
	}
};

menu_t m_App_MusicPlayer =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_AppGame,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="",//中文标题
	  .str_en_US="",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="2.音乐播放器",//中文描述
	  .str_en_US="2.Music",//英文描述
	},
	.menuType=mPorgram,//菜单类型
	.menuPar.mPorgram_Par=
	{
		.begin_fun=MusicPlayer_App_Begin,//菜单进入 执行一次
		.loop_fun=MusicPlayer_App_Loop, //菜单功能 持续执行
		.quit_fun=MusicPlayer_App_Quit, //菜单退出 执行一次
		.refresh_fun = MusicPlayer_APP_Refresh//刷新屏幕时执行,放绘图函数
	}
};

menu_t m_AppGame =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_App_VideoPleayer,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="",//中文标题
	  .str_en_US="",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="3.矩形碰碰车",//中文描述
	  .str_en_US="3.Game",//英文描述
	},
	.menuType=mPorgram,//菜单类型
	.menuPar.mPorgram_Par=
	{
		.begin_fun=Game_App_Begin,//菜单进入 执行一次
		.loop_fun=Game_App_Loop, //菜单功能 持续执行
		.quit_fun=Game_App_Quit, //菜单退出 执行一次
		.refresh_fun = Game_APP_Refresh//刷新屏幕时执行,放绘图函数
	}
};

menu_t m_App_VideoPleayer =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_App_Setting,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="",//中文标题
	  .str_en_US="",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="4.视频播放器",//中文描述
	  .str_en_US="4.Video",//英文描述
	},
	.menuType=mPorgram,//菜单类型
	.menuPar.mPorgram_Par=
	{
		.begin_fun=m_App_VideoPleayer_Begin,//菜单进入 执行一次
		.loop_fun=m_App_VideoPleayer_Loop, //菜单功能 持续执行
		.quit_fun=m_App_VideoPleayer_Quit, //菜单退出 执行一次
		.refresh_fun = m_App_VideoPleayer_Refresh//刷新屏幕时执行,放绘图函数
	}
};

menu_t m_App_Setting =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=&m_Setting_Display,//(首个)子菜单
	.nextMenu=&m_App_6,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="设置",//中文标题
	  .str_en_US="SETTING",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="5.设置",//中文描述
	  .str_en_US="5.Setting",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};

menu_t m_App_6 =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_App_7,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="应用6",//中文标题
	  .str_en_US="APP 6",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="6.应用6",//中文描述
	  .str_en_US="6.App 6",//英文描述
	},
	.menuType=mPorgram,//菜单类型
	.menuPar.mPorgram_Par=
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
		.refresh_fun = 0x00//刷新屏幕时执行,放绘图函数
	}
};
menu_t m_App_7 =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_App_8,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="应用7",//中文标题
	  .str_en_US="APP 7",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="7.应用7",//中文描述
	  .str_en_US="7.App 7",//英文描述
	},
	.menuType=mPorgram,//菜单类型
	.menuPar.mPorgram_Par=
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
		.refresh_fun = 0x00//刷新屏幕时执行,放绘图函数
	}
};

menu_t m_App_8 =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_App_9,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="应用8",//中文标题
	  .str_en_US="APP 8",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="8.应用8",//中文描述
	  .str_en_US="8.App 8",//英文描述
	},
	.menuType=mPorgram,//菜单类型
	.menuPar.mPorgram_Par=
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
		.refresh_fun = 0x00//刷新屏幕时执行,放绘图函数
	}
};

menu_t m_App_9 =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_App_10,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="应用9",//中文标题
	  .str_en_US="APP 9",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="9.应用9",//中文描述
	  .str_en_US="9.App 9",//英文描述
	},
	.menuType=mPorgram,//菜单类型
	.menuPar.mPorgram_Par=
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
		.refresh_fun = 0x00//刷新屏幕时执行,放绘图函数
	}
};

menu_t m_App_10 =
{
	.fatherMenu=&m_main,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="应用10",//中文标题
	  .str_en_US="APP10",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="10.应用10",//中文描述
	  .str_en_US="10.App 10",//英文描述
	},
	.menuType=mPorgram,//菜单类型
	.menuPar.mPorgram_Par=
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
		.refresh_fun = 0x00//刷新屏幕时执行,放绘图函数
	}
};

//-------------------m.m_Widget_Demo------------------------
menu_t m_wDemo_wMessage_Tip=
{
	.fatherMenu=&m_wDemo,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_wDemo_wCheckbox,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	  .str_en_US=" ",//英文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	},
	.discribe=
	{
		.str_zh_CN="wMessage Tip",//中文描述
	  .str_en_US="wMessage Tip",//英文描述
	},
	.menuType=wMessage,//菜单类型
	.menuPar.wMessage_Par.Tip_string=
	{
		.str_zh_CN="wMessage\n支持换行\n222",//中文标题
	  .str_en_US="wMessage\nnewline A\nnewline B",//英文标题
	},
};

//-------------------m.m_wDemo_wMessage------------------------
menu_t m_wDemo_wCheckbox=
{
	.fatherMenu=&m_wDemo,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_wDemo_wMessage_ADC,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	  .str_en_US=" ",//英文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	},
	.discribe=
	{
		.str_zh_CN="wCheckBox",//中文描述
	  .str_en_US="wCheckBox",//英文描述
	},
	.menuType=wCheckBox,//菜单类型
	.menuPar.wCheckBox_Par =
	{
		.pstr=&demo_bool,  //复选框控制的数据指针
		.Change_Value=0x00,//值被修改 执行一次
	},
};


menu_t m_wDemo_wMessage_ADC=
{
	.fatherMenu=&m_wDemo,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_wDemo_wMessage_Pres,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="",//中文标题  该菜单(控件)默认不可进入, 可不设置标题名称
	  .str_en_US="",//英文标题  该菜单(控件)默认不可进入, 可不设置标题名称
	},
	.discribe=
	{
		.str_zh_CN="wMessage",//中文描述
	  .str_en_US="wMessage",//英文描述
	},
	.menuType=wMessage,//菜单类型
	.menuPar.wMessage_Par=
	{
		.Press_func=0x00,
		.Value_string=demo_string_1,
		.Tip_string=
		{
			.str_zh_CN="演示\ndemo_value",//中文提示框
			.str_en_US="demo_value\nincrease",//英文提示框
		},
	},
};

menu_t m_wDemo_wMessage_Pres=
{
	.fatherMenu=&m_wDemo,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_wDemo_wMessage_Pres2,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	  .str_en_US=" ",//英文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	},
	.discribe=
	{
		.str_zh_CN="wMes 按下",//中文描述
	  .str_en_US="wMes pres",//英文描述
	},
	.menuType=wMessage,//菜单类型
	.menuPar.wMessage_Par=
	{
		.Press_func=m_wDemo_wMessage_Pres_func,
		.Value_string=demo_string_2,
		.Tip_string=
		{
			.str_zh_CN="",//中文提示框
			.str_en_US="",//英文提示框
		},
	},
};

menu_t m_wDemo_wMessage_Pres2=
{
	.fatherMenu=&m_wDemo,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_wDemo_wSlider1,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	  .str_en_US=" ",//英文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	},
	.discribe=
	{
		.str_zh_CN="wMes 按下",//中文描述
	  .str_en_US="wMes pres",//英文描述
	},
	.menuType=wMessage,//菜单类型
	.menuPar.wMessage_Par=
	{
		.Press_func=m_wDemo_wMessage_Pres2_func,
		.Value_string=demo_string_3,
		.Tip_string=
		{
			.str_zh_CN="",//中文提示框
			.str_en_US="",//英文提示框
		},
	},
};

menu_t m_wDemo_wSlider1 =
{
	.fatherMenu=&m_wDemo,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_wDemo_wSlider2,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	  .str_en_US=" ",//英文标题 该菜单(控件)默认不可进入, 可不设置标题名称
	},
	.discribe=
	{
		.str_zh_CN="wSlider",//中文描述 列表里显示的名称
	  .str_en_US="wSlider",//英文描述 列表里显示的名称
	},
	.menuType=wSlider,//菜单类型
	.menuPar.wSliderTip_Par =
	{
		.Push_tip_func=0x00,      //控件进入 执行一次
		.Change_Value_func=0x00,  //值被修改 执行一次
		.End_tip_func=0x00,       //控件退出 执行一次
		.tip_string=              //弹窗提示的字符串
		{
			.str_zh_CN="demo1",//中文标题 弹窗里显示的文字
			.str_en_US="demo1",//英文标题 弹窗里显示的文字
		},
		.pvalue = &slider_demo_value,
		.min = -100,
		.max = 100,
	},
};

menu_t m_wDemo_wSlider2 =
{
	.fatherMenu=&m_wDemo,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 弹窗里显示的名称
	  .str_en_US=" ",//英文标题 弹窗里显示的名称
	},
	.discribe=
	{
		.str_zh_CN="wSlider",//中文描述 列表里显示的名称
	  .str_en_US="wSlider",//英文描述 列表里显示的名称
	},
	.menuType=wSlider,//菜单类型
	.menuPar.wSliderTip_Par =
	{
		.Push_tip_func=0x00,    //控件进入 执行一次
		.Change_Value_func=0x00,//值被修改 执行一次
		.End_tip_func=0x00,     //控件退出 执行一次
		.tip_string=            //弹窗提示的字符串
		{
			.str_zh_CN="---demo2----\nnewline\nnewline\nnewline",//中文标题 弹窗里显示的文字
			.str_en_US="---demo2----\nnewline\nnewline\nnewline",//英文标题 弹窗里显示的文字
		},
		.pvalue = &slider_demo_value,
		.min = -100,
		.max = 100,
	},
};

//-------------------m.m_App_Setting------------------------
menu_t m_Setting_Display =
{
	.fatherMenu=&m_App_Setting,//父菜单
	.subMenu=&m_Setting_Display_Brightness,//(首个)子菜单
	.nextMenu=&m_Setting_Time,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="显示",//中文标题
	  .str_en_US="DISPLAY",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="1.显示",//中文描述
	  .str_en_US="1.Display",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};

menu_t m_Setting_Time =
{
	.fatherMenu=&m_App_Setting,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_Setting_Speaker,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="时间",//中文标题
	  .str_en_US="Time&Clock",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="2.时间",//中文描述
	  .str_en_US="2.Time&Clock",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};

menu_t m_Setting_Speaker =
{
	.fatherMenu=&m_App_Setting,//父菜单
	.subMenu=&m_Setting_Speaker_Volume,//(首个)子菜单
	.nextMenu=&m_Setting_UI,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="音频",//中文标题
	  .str_en_US="SPEAKER",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="3.音频",//中文描述
	  .str_en_US="3.Speaker",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};


menu_t m_Setting_UI =
{
	.fatherMenu=&m_App_Setting,//父菜单
	.subMenu=&m_Setting_UI_colour,//(首个)子菜单
	.nextMenu=&m_Setting_Language,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="UI&主题",//中文标题
	  .str_en_US="UI THEME",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="5.UI&主题",//中文描述
	  .str_en_US="5.UI Theme",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};




menu_t m_Setting_Language =
{
	.fatherMenu=&m_App_Setting,//父菜单
	.subMenu=&m_Setting_Language_English,//(首个)子菜单
	.nextMenu=&m_Setting_Developer,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="系统:简体中文 ",//中文标题
	  .str_en_US="SYS:English",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="6.语言",//中文描述
	  .str_en_US="6.Language",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};

menu_t m_Setting_Developer =
{
	.fatherMenu=&m_App_Setting,//父菜单
	.subMenu=&m_Setting_Developer_wDebugInfoEn,//(首个)子菜单
	.nextMenu=&m_Setting_About,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="开发者选项",//中文标题
	  .str_en_US="DEVELOPER",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="7.开发者选项",//中文描述
	  .str_en_US="7.Developer",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};

menu_t m_Setting_About=
{
	.fatherMenu=&m_App_Setting,//父菜单
	.subMenu=&m_Setting_About_wCpu,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="关于",//中文标题
	  .str_en_US="ABOUT",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="8.关于",//中文描述
	  .str_en_US="8.About",//英文描述
	},
	.menuType=mList,//菜单类型
	.menuPar.mList_Par =
	{
		.begin_fun=0x00,//菜单进入 执行一次
		.loop_fun=0x00, //菜单功能 持续执行
		.quit_fun=0x00, //菜单退出 执行一次
	},
};

//-------------------m.Setting.Language语言------------------------
void Set_language_English(void);
menu_t m_Setting_Language_English =
{
	.fatherMenu=&m_Setting_Language,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_Setting_Language_Chinese,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="ENGLISH",//中文标题
	  .str_en_US="ENGLISH",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="1.English",//中文描述
	  .str_en_US="1.English",//英文描述
	},

	.menuType=wMessage,//菜单类型
	.menuPar.wMessage_Par =
	{
		.Press_func=Set_language_English,//按下执行一次
		.Value_string=0x00,              //末尾显示的值 0x00不显示
		.Tip_string=
		{
			.str_zh_CN="error",//中文提示框
			.str_en_US="English now!",//英文提示框
		},
	},
};

menu_t m_Setting_Language_Chinese =
{
	.fatherMenu=&m_Setting_Language,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="简体中文",//中文标题
	  .str_en_US="简体中文",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="2.简体中文",//中文描述
	  .str_en_US="2.简体中文",//英文描述
	},
	.menuType=wMessage,//菜单类型
	.menuPar.wMessage_Par =
	{
		.Press_func=Set_language_Chinese, //按下执行一次
		.Value_string=0x00,               //末尾显示的值 0x00不显示
		.Tip_string=
		{
			.str_zh_CN="已设置简体中文",//中文提示框
			.str_en_US="error",//英文提示框
		},
	},
};

//-------------------m.Setting.Display显示------------------------
menu_t m_Setting_Display_Brightness =
{
	.fatherMenu=&m_Setting_Display,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_Setting_Display_ScreenFPS,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题
	  .str_en_US=" ",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="1.对比度",//中文描述
	  .str_en_US="1.Bright",//英文描述
	},
	.menuType=wSlider,//菜单类型
	.menuPar.wSliderTip_Par =
	{
		.Push_tip_func=0x00,                                //控件进入 执行一次
		.Change_Value_func=update_Wegui_screen_brightness,  //值被修改 执行一次
		.End_tip_func=0x00,                                 //控件退出 执行一次
		.tip_string=            //弹窗提示的字符串
		{
			.str_zh_CN="对比度",//中文标题 弹窗里显示的文字
			.str_en_US="Bright",//英文标题 弹窗里显示的文字
		},
		.pvalue = &wegui.setting.brightness,
		.min = 0,
		.max = 255,
	},
};


menu_t m_Setting_Display_ScreenFPS =
{
	.fatherMenu=&m_Setting_Display,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_Setting_Display_UI_Speed,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题
	  .str_en_US=" ",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="2.屏幕帧率",//中文描述
	  .str_en_US="2.Screen FPS",//英文描述
	},
	.menuType=wSlider,//菜单类型
	.menuPar.wSliderTip_Par =
	{
		.Push_tip_func=0x00,      //控件进入 执行一次
		.Change_Value_func=0x00,  //值被修改 执行一次
		.End_tip_func=0x00,       //控件退出 执行一次
		.tip_string=            //弹窗提示的字符串
		{
			.str_zh_CN="屏幕刷新时间\nms",//中文标题 弹窗里显示的文字
			.str_en_US="Screen ms",//英文标题 弹窗里显示的文字
		},
		.pvalue = &wegui.setting.screen_fps_ms,
		.min = 1,
		.max = 100,
	},
};


menu_t m_Setting_Display_UI_Speed =
{
	.fatherMenu=&m_Setting_Display,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN="UI刷新时间\nms",//中文标题
	  .str_en_US="UI ms",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="3.动画帧率 ",//中文描述
	  .str_en_US="3.UI FPS ",//英文描述
	},
	.menuType=wSlider,//菜单类型
	.menuPar.wSliderTip_Par =
	{
		.Push_tip_func=0x00,      //控件进入 执行一次
		.Change_Value_func=0x00,  //值被修改 执行一次
		.End_tip_func=0x00,       //控件退出 执行一次
		.tip_string=            //弹窗提示的字符串
		{
			.str_zh_CN="UI刷新时间\nms",//中文标题 弹窗里显示的文字
			.str_en_US="UI ms",//英文标题 弹窗里显示的文字
		},
		.pvalue = &wegui.setting.ui_fps_ms,
		.min = 1,
		.max = 100,
	},
};

//-------------------m.Setting.Speaker声音------------------------
menu_t m_Setting_Speaker_Volume =
{
	.fatherMenu=&m_Setting_Display,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题
	  .str_en_US=" ",//英文标题
	},
	.discribe=
	{
		.str_zh_CN="1.音量",//中文描述
	  .str_en_US="1.Volume",//英文描述
	},
	.menuType=wSlider,//菜单类型
	.menuPar.wSliderTip_Par =
	{
		.Push_tip_func=0x00,      //控件进入 执行一次
		.Change_Value_func=0x00,  //值被修改 执行一次
		.End_tip_func=0x00,       //控件退出 执行一次
		.tip_string=            //弹窗提示的字符串
		{
			.str_zh_CN="音量",//中文标题 弹窗里显示的文字
			.str_en_US="Volume",//英文标题 弹窗里显示的文字
		},
		.pvalue = &wegui.setting.voice_volume,
		.min = 0,
		.max = 10,
	},
};


//-------------------m_setting_fontsColour字体颜色------------------------
menu_t m_Setting_UI_colour =
{
	.fatherMenu=&m_Setting_UI,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
	  .str_en_US="",//英文标题
		.str_zh_CN="",//中文标题
	},
	.discribe=
	{
	  .str_en_US="1.Theme",//英文描述
		.str_zh_CN="1.主题",//中文描述
	},
	.menuType=wSlider,//菜单类型
	.menuPar.wSliderTip_Par =
	{
		.Push_tip_func=0x00,      //弹窗打开 执行一次
		.Change_Value_func=set_theme,  //值被修改 执行一次
		.End_tip_func=0x00,       //弹窗退出 执行一次
		.pvalue = &theme_select,
		.tip_string=
		{
			.str_en_US = "theme",
			.str_zh_CN = "主题",
		},
		.min = 1,
		.max = 6,
	},
};

//---------------m.Setting.Developer开发者选项---------------------
menu_t m_Setting_Developer_wDebugInfoEn=
{
	.fatherMenu=&m_Setting_Developer,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 控件可不设置标题
	  .str_en_US=" ",//英文标题 控件可不设置标题
	},
	.discribe=
	{
		.str_zh_CN="1.调试窗口",//中文描述
	  .str_en_US="1.Debug",//英文描述
	},
	.menuType=wCheckBox,//菜单类型
	.menuPar.wCheckBox_Par =
	{
		.pstr=&wegui.setting.debug_windows_en,//复选框控制的数据指针
		.Change_Value=0x00,//值被修改 执行一次
	},
};

//---------------m_Setting_About开发者选项---------------------
menu_t m_Setting_About_wCpu=
{
	.fatherMenu=&m_Setting_About,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=&m_Setting_About_wSoft,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 wMessage可不设置
	  .str_en_US=" ",//英文标题 wMessage可不设置
	},
	.discribe=
	{
		.str_zh_CN="主控",//中文描述
	  .str_en_US="MCU",//英文描述
	},
	.menuType=wMessage,//菜单类型
	.menuPar.wMessage_Par.Value_string = STR_MCU_CLASS,
	.menuPar.wMessage_Par.Tip_string=
	{
		.str_zh_CN=STR_MCU_MODEL,//中文标题
	  .str_en_US=STR_MCU_MODEL,//英文标题
	},
};

menu_t m_Setting_About_wSoft=
{
	.fatherMenu=&m_Setting_About,//父菜单
	.subMenu=0x00,//(首个)子菜单
	.nextMenu=0x00,//同级下一个菜单
	.titel=
	{
		.str_zh_CN=" ",//中文标题 wMessage可不设置
	  .str_en_US=" ",//英文标题 wMessage可不设置
	},
	.discribe=
	{
		.str_zh_CN="Software",//中文描述
	  .str_en_US="Software",//英文描述
	},
	.menuType=wMessage,//菜单类型
	.menuPar.wMessage_Par.Value_string = STR_WEGUI_VERSION_CLASS,
	.menuPar.wMessage_Par.Tip_string =
	{
		.str_zh_CN=STR_WEGUI_VERSION,//中文标题
	  .str_en_US=STR_WEGUI_VERSION,//英文标题
	},
};

