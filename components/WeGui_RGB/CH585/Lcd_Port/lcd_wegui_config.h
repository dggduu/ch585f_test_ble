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

#ifndef LCD_WEGUI_CONFIG_H
#define LCD_WEGUI_CONFIG_H

#include "lcd_driver_config.h"

//字符串 在DEMO菜单中调用
#define STR_MCU_CLASS "CH585F"
#define STR_MCU_MODEL "WCH CH585F"
#define STR_WEGUI_VERSION_CLASS "V0.5"
#define STR_WEGUI_VERSION "V0.5.7"

//--------------------------1.设置菜单--------------------
#define MENU_CONST_DIS              (0) //菜单结构是否允许修改 0不可修改(省RAM) 1可修改
#define MENU_DEEP                   (3) //设置菜单的最大层数(用于分配变量,储存光标历史位置,取小了返回菜单时可能不会回到上一次的位置)
#define MLIST_MENU_CURSOR_UP_LOOP   (0) //mlist菜单光标往上循环 0关闭循环 1使能循环
#define MLIST_MENU_CURSOR_DOWN_LOOP (0) //mlist菜单光标往下循环 0关闭循环 1使能循环
#define MLIST_MENU_YSCAPE           (mList_par.list_font_high/2) //自定义mlist菜单上下间隙 (0)紧靠 (mList_par.list_font_high/2)根据utf8字体大小自动间隙
#define STARTUP_DEBUG_WINDOWS_EN    (1) //开机右下角是否有调试窗口 0关闭 1打开

//--------------------------2.选择一个菜单交互方式-----------------------------
//1.菜单交互接口使用 lcd_wegui_menu_mlist.h 的函数声明
//2.弹窗交互接口使用 lcd_wegui_tip.h 的函数声明
//CH585评估板无板载按键, 暂使用无接口模式(仅显示菜单); 有按键后可移植按键接口
#define _NO_GUI_PORT            (0) //没有接口
#define _GUI_KEY_PORT           (1) //按键接口(示例) 编码器接口(示例)
#define WEGUI_PORT     _NO_GUI_PORT //选择一个交互接口

#if(WEGUI_PORT == _GUI_KEY_PORT)
	//#include ".h"
	#define wegui_port_task(stick) do{}while(0)//填入主循环查询接口 stick是ms时间基准
	#define wegui_port_ms_irq()    do{}while(0)//填入中断查询接口 1ms中断一次
	#define wegui_port_init()      do{}while(0)//填入初始化接口 开机执行一次
#endif

//--------------------------3.设置菜单主题-----------------------------
#if ((LCD_TYPE == LCD_GRAY) || (LCD_TYPE == LCD_OLED))
	#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色
	#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
	#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
	#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
	#define COLOUR_MLIST_WCHECKBOX_INTER  (1)//菜单列表wCheckbox内填充颜色
	#define COLOUR_MLIST_SCROOL_BAR       (1)//菜单列表滚动条颜色
	#define COLOUR_MESS_TIP_BOX_BORDER    (1)//wMessage提示弹窗边缘颜色
	#define COLOUR_MESS_TIP_TEXT          (1)//wMessage提示弹窗文字颜色
	#define COLOUR_SLIDER_TIP_BOX_BORDER  (1)//wSlider滑条弹窗边缘颜色
	#define COLOUR_SLIDER_TIP_TEXT        (1)//wSlider滑条弹窗文字颜色
	#define COLOUR_SLIDER_TIP_NUM         (1)//wSlider滑条弹窗数字颜色
	#define COLOUR_SLIDER_TIP_BAR_BORDER  (1)//wSlider滑条弹窗的滑条边框颜色
	#define COLOUR_SLIDER_TIP_BAR_PROG    (1)//wSlider滑条弹窗的进度颜色
	#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
	#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
	#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色

#elif (LCD_TYPE == LCD_RGB565)
	#if (LCD_COLOUR_BIT == 1)//1位色时的主题 只有2种颜色
		//---配色1 白云晴空---
		//#define COLOUR_MLIST_DEFAULT_0  0xffbe //默认菜单背景色0 00  //背景色默认为0
		//#define COLOUR_MLIST_DEFAULT_1  0x4395 //默认菜单画笔色1 01
		//#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色
		//#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
		//#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
		//#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
		//#define COLOUR_MLIST_WCHECKBOX_INTER  (1)//菜单列表wCheckbox内填充颜色
		//#define COLOUR_MLIST_SCROOL_BAR       (1)//菜单列表滚动条颜色
		//#define COLOUR_MESS_TIP_BOX_BORDER    (1)//wMessage提示弹窗边缘颜色
		//#define COLOUR_MESS_TIP_TEXT          (1)//wMessage提示弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_BOX_BORDER  (1)//wSlider滑条弹窗边缘颜色
		//#define COLOUR_SLIDER_TIP_TEXT        (1)//wSlider滑条弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_NUM         (1)//wSlider滑条弹窗数字颜色
		//#define COLOUR_SLIDER_TIP_BAR_BORDER  (1)//wSlider滑条弹窗的滑条边框颜色
		//#define COLOUR_SLIDER_TIP_BAR_PROG    (1)//wSlider滑条弹窗的进度颜色
		//#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
		//#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
		//#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色

		//---配色2 大橘为重---
		//#define COLOUR_MLIST_DEFAULT_0  0xfd80 //背景色0 00  //背景色默认为0
		//#define COLOUR_MLIST_DEFAULT_1  0x4a48 //画笔色1 01
		//#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色
		//#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
		//#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
		//#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
		//#define COLOUR_MLIST_WCHECKBOX_INTER  (1)//菜单列表wCheckbox内填充颜色
		//#define COLOUR_MLIST_SCROOL_BAR       (1)//菜单列表滚动条颜色
		//#define COLOUR_MESS_TIP_BOX_BORDER    (1)//wMessage提示弹窗边缘颜色
		//#define COLOUR_MESS_TIP_TEXT          (1)//wMessage提示弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_BOX_BORDER  (1)//wSlider滑条弹窗边缘颜色
		//#define COLOUR_SLIDER_TIP_TEXT        (1)//wSlider滑条弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_NUM         (1)//wSlider滑条弹窗数字颜色
		//#define COLOUR_SLIDER_TIP_BAR_BORDER  (1)//wSlider滑条弹窗的滑条边框颜色
		//#define COLOUR_SLIDER_TIP_BAR_PROG    (1)//wSlider滑条弹窗的进度颜色
		//#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
		//#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
		//#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色

		//---配色3 深海蔚蓝---
		#define COLOUR_MLIST_DEFAULT_0  0x530d //背景色0 00  //背景色默认为0
		#define COLOUR_MLIST_DEFAULT_1  0xefbe //画笔色1 01
		#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色
		#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
		#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
		#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
		#define COLOUR_MLIST_WCHECKBOX_INTER  (1)//菜单列表wCheckbox内填充颜色
		#define COLOUR_MLIST_SCROOL_BAR       (1)//菜单列表滚动条颜色
		#define COLOUR_MESS_TIP_BOX_BORDER    (1)//wMessage提示弹窗边缘颜色
		#define COLOUR_MESS_TIP_TEXT          (1)//wMessage提示弹窗文字颜色
		#define COLOUR_SLIDER_TIP_BOX_BORDER  (1)//wSlider滑条弹窗边缘颜色
		#define COLOUR_SLIDER_TIP_TEXT        (1)//wSlider滑条弹窗文字颜色
		#define COLOUR_SLIDER_TIP_NUM         (1)//wSlider滑条弹窗数字颜色
		#define COLOUR_SLIDER_TIP_BAR_BORDER  (1)//wSlider滑条弹窗的滑条边框颜色
		#define COLOUR_SLIDER_TIP_BAR_PROG    (1)//wSlider滑条弹窗的进度颜色
		#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
		#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
		#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色

	#elif (LCD_COLOUR_BIT == 2)//2位色时的主题 只有4种颜色

		//---配色1 白云晴空---
		//#define COLOUR_MLIST_DEFAULT_0  0xffbe //默认菜单背景色0 00  //背景色默认为0
		//#define COLOUR_MLIST_DEFAULT_1  0x4395 //默认菜单画笔色1 01
		//#define COLOUR_MLIST_DEFAULT_2  0xdf1d //默认菜单画笔色2 10
		//#define COLOUR_MLIST_DEFAULT_3  0x1169 //默认菜单画笔色3 11
		//#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色(光标色取反,即2)
		//#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
		//#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
		//#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
		//#define COLOUR_MLIST_WCHECKBOX_INTER  (3)//菜单列表wCheckbox内填充颜色
		//#define COLOUR_MLIST_SCROOL_BAR       (1)//菜单列表滚动条颜色
		//#define COLOUR_MESS_TIP_BOX_BORDER    (1)//wMessage提示弹窗边缘颜色
		//#define COLOUR_MESS_TIP_TEXT          (1)//wMessage提示弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_BOX_BORDER  (1)//wSlider滑条弹窗边缘颜色
		//#define COLOUR_SLIDER_TIP_TEXT        (1)//wSlider滑条弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_NUM         (1)//wSlider滑条弹窗数字颜色
		//#define COLOUR_SLIDER_TIP_BAR_BORDER  (1)//wSlider滑条弹窗的滑条边框颜色
		//#define COLOUR_SLIDER_TIP_BAR_PROG    (3)//wSlider滑条弹窗的进度颜色
		//#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
		//#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
		//#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色

		//---配色2 大橘为重---
		#define COLOUR_MLIST_DEFAULT_0  0xfd80 //背景色0 00  //背景色默认为0
		#define COLOUR_MLIST_DEFAULT_1  0x4a48 //画笔色1 01
		#define COLOUR_MLIST_DEFAULT_2  0xffdb //画笔色2 10
		#define COLOUR_MLIST_DEFAULT_3  0x3c57 //画笔色3 11
		#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色(光标色取反,即2)
		#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
		#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
		#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
		#define COLOUR_MLIST_WCHECKBOX_INTER  (1)//菜单列表wCheckbox内填充颜色
		#define COLOUR_MLIST_SCROOL_BAR       (1)//菜单列表滚动条颜色
		#define COLOUR_MESS_TIP_BOX_BORDER    (1)//wMessage提示弹窗边缘颜色
		#define COLOUR_MESS_TIP_TEXT          (1)//wMessage提示弹窗文字颜色
		#define COLOUR_SLIDER_TIP_BOX_BORDER  (1)//wSlider滑条弹窗边缘颜色
		#define COLOUR_SLIDER_TIP_TEXT        (1)//wSlider滑条弹窗文字颜色
		#define COLOUR_SLIDER_TIP_NUM         (2)//wSlider滑条弹窗数字颜色
		#define COLOUR_SLIDER_TIP_BAR_BORDER  (1)//wSlider滑条弹窗的滑条边框颜色
		#define COLOUR_SLIDER_TIP_BAR_PROG    (3)//wSlider滑条弹窗的进度颜色
		#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
		#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
		#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色

		//---配色3 深海蔚蓝---
		//#define COLOUR_MLIST_DEFAULT_0  0x530d //背景色0 00  //背景色默认为0
		//#define COLOUR_MLIST_DEFAULT_1  0xefbe //画笔色1 01
		//#define COLOUR_MLIST_DEFAULT_2  0xc6bb //画笔色2 10
		//#define COLOUR_MLIST_DEFAULT_3  0x2104 //画笔色3 11
		//#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色(光标色取反,即2)
		//#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
		//#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
		//#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
		//#define COLOUR_MLIST_WCHECKBOX_INTER  (1)//菜单列表wCheckbox内填充颜色
		//#define COLOUR_MLIST_SCROOL_BAR       (1)//菜单列表滚动条颜色
		//#define COLOUR_MESS_TIP_BOX_BORDER    (2)//wMessage提示弹窗边缘颜色
		//#define COLOUR_MESS_TIP_TEXT          (2)//wMessage提示弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_BOX_BORDER  (2)//wSlider滑条弹窗边缘颜色
		//#define COLOUR_SLIDER_TIP_TEXT        (2)//wSlider滑条弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_NUM         (2)//wSlider滑条弹窗数字颜色
		//#define COLOUR_SLIDER_TIP_BAR_BORDER  (2)//wSlider滑条弹窗的滑条边框颜色
		//#define COLOUR_SLIDER_TIP_BAR_PROG    (1)//wSlider滑条弹窗的进度颜色
		//#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
		//#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
		//#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色

		//---配色4 血色记忆---
		//#define COLOUR_MLIST_DEFAULT_0  0xd124 //背景色0 00  //背景色默认为0
		//#define COLOUR_MLIST_DEFAULT_1  0x2986 //画笔色1 01
		//#define COLOUR_MLIST_DEFAULT_2  0xef7d //画笔色2 10
		//#define COLOUR_MLIST_DEFAULT_3  0x3a09 //画笔色3 11
		//#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色(光标色取反,即2)
		//#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
		//#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
		//#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
		//#define COLOUR_MLIST_WCHECKBOX_INTER  (3)//菜单列表wCheckbox内填充颜色
		//#define COLOUR_MLIST_SCROOL_BAR       (3)//菜单列表滚动条颜色
		//#define COLOUR_MESS_TIP_BOX_BORDER    (1)//wMessage提示弹窗边缘颜色
		//#define COLOUR_MESS_TIP_TEXT          (1)//wMessage提示弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_BOX_BORDER  (1)//wSlider滑条弹窗边缘颜色
		//#define COLOUR_SLIDER_TIP_TEXT        (1)//wSlider滑条弹窗文字颜色
		//#define COLOUR_SLIDER_TIP_NUM         (2)//wSlider滑条弹窗数字颜色
		//#define COLOUR_SLIDER_TIP_BAR_BORDER  (1)//wSlider滑条弹窗的滑条边框颜色
		//#define COLOUR_SLIDER_TIP_BAR_PROG    (3)//wSlider滑条弹窗的进度颜色
		//#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
		//#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
		//#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色

	#elif (LCD_COLOUR_BIT == 3)//3位色时的主题 支持8种颜色
		//---配色2 大橘为重---
		#define COLOUR_MLIST_DEFAULT_0  0xfd80 //背景色0 000  //背景色默认为0
		#define COLOUR_MLIST_DEFAULT_1  0x4a48 //画笔色1 001
		#define COLOUR_MLIST_DEFAULT_2  0xaa05 //画笔色2 010
		#define COLOUR_MLIST_DEFAULT_3  0x3186 //画笔色3 011
		#define COLOUR_MLIST_DEFAULT_4  0xef77 //画笔色4 100
		#define COLOUR_MLIST_DEFAULT_5  0x0000 //画笔色5 101
		#define COLOUR_MLIST_DEFAULT_6  0xffdb //画笔色6 110
		#define COLOUR_MLIST_DEFAULT_7  0x3c57 //画笔色7 111
		#define COLOUR_MLIST_NORMAL_TEXT      (1)//菜单列表文字色(光标色取反,即6)
		#define COLOUR_MLIST_WSLIDER_NUM      (1)//菜单列表wSlider数字颜色
		#define COLOUR_MLIST_WMESSAGE_TEXT    (1)//菜单列表wMessage字符颜色
		#define COLOUR_MLIST_WCHECKBOX_BORDER (1)//菜单列表wCheckbox外框颜色
		#define COLOUR_MLIST_WCHECKBOX_INTER  (1)//菜单列表wCheckbox内填充颜色
		#define COLOUR_MLIST_SCROOL_BAR       (1)//菜单列表滚动条颜色
		#define COLOUR_MESS_TIP_BOX_BORDER    (3)//wMessage提示弹窗边缘颜色
		#define COLOUR_MESS_TIP_TEXT          (3)//wMessage提示弹窗文字颜色
		#define COLOUR_SLIDER_TIP_BOX_BORDER  (3)//wSlider滑条弹窗边缘颜色
		#define COLOUR_SLIDER_TIP_TEXT        (3)//wSlider滑条弹窗文字颜色
		#define COLOUR_SLIDER_TIP_NUM         (4)//wSlider滑条弹窗数字颜色
		#define COLOUR_SLIDER_TIP_BAR_BORDER  (1)//wSlider滑条弹窗的滑条边框颜色
		#define COLOUR_SLIDER_TIP_BAR_PROG    (2)//wSlider滑条弹窗的进度颜色
		#define COLOUR_DEBUG_BAR_BG           (0)//调试窗的底色
		#define COLOUR_DEBUG_BAR_BORDER       (1)//调试窗的边框颜色
		#define COLOUR_DEBUG_BAR_TEXT         (1)//调试窗的文字颜色
	#else
		#error("not support LCD_COLOUR_BIT!")
	#endif
#endif

//------------编译-----------

#endif
