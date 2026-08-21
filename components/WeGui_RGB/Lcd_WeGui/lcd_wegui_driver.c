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
#include "lcd_wegui_config.h"
#include "lcd_wegui_driver.h"

wegui_t wegui;

/*--------------------------------------------------------------
  * 名称: *my_itoa(int16_t num,uint8_t *str,uint8_t radix)
  * 传入1: num 数字
  * 传入2: *str (空)字符串指针
  * 传入3: radix 进制数
  * 功能: 数字转字符串,保存到*str里
  * 说明:iota = integer to alphanumeric 把整型数转换成字符串
----------------------------------------------------------------*/
char *my_itoa(int16_t num,char *str,uint8_t radix)
{
    const char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
    uint32_t unum;//存放要转换的整数的绝对值,转换的整数可能是负数
    uint32_t i=0,j,k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。
		uint8_t temp;//临时变量，交换两个值时用到
	
    //获取要转换的整数的绝对值
    if(radix==10&&num<0)//要转换成十进制数并且是负数
    {
        unum=(unsigned)-num;//将num的绝对值赋给unum
        str[i++]='-';//在字符串最前面设置为'-'号，并且索引加1
    }
    else unum=/*(unsigned)*/num;//若是num为正，直接赋值给unum
    //转换部分，注意转换后是逆序的
    do
    {
        str[i++]=index[unum%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
        unum/=radix;//unum去掉最后一位

    }while(unum);//直至unum为0退出循环

    str[i]='\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。

    //将顺序调整过来
    if(str[0]=='-') k=1;//如果是负数，符号不用调整，从符号后面开始调整
    else k=0;//不是负数，全部都要调整

    
    for(j=k;j<=(i-1)/2;j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];//头部赋值给临时变量
        str[j]=str[i-1+k-j];//尾部赋值给头部
        str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
    }
    return str;//返回转换后的字符串
}

/*--------------------------------------------------------------
  * 名称: char* wegui_get_string(wegui_string_t object,langage_t language)
  * 传入1: object 语言包
  * 传入2: language 语言
  * 功能: 返回"语言包"里对应的"language语言"字符串指针,
----------------------------------------------------------------*/
char* wegui_get_string(wegui_string_t object,langage_t language)
{
	char* i=0;
	switch(language)
	{
		case zh_CN:i=(char*)object.str_zh_CN;break;
		case zh_TC:i=(char*)object.str_zh_TC;break;
		default:
		case en_US:i=(char*)object.str_en_US;break;
	}
	if(i!=0x00)
	{
		return i;
	}
	else
	{
		return (char*)object.str_en_US;
	}
}

/*--------------------------------------------------------------
  * 名称: Get_submenu_sum(menu_t* m)
  * 传入: m:查询的菜单
  * 功能: 获取菜单中子菜单的总数
----------------------------------------------------------------*/
uint8_t Get_submenu_sum(menu_t* m)//获取菜单中子菜单的总数
{
	uint8_t num=0;
	if(m!=0x00)
	{
		if(m->subMenu != 0x00)
		{
			m = m->subMenu;
			num++;
			while(m->nextMenu != 0x00)
			{
				m = m->nextMenu;
				num++;
			}
		}
	}
	return num;
}

/*--------------------------------------------------------------
  * 名称: void Push_menu_historyPar(menu_history_t i)
  * 传入: menu_history_t i需要储存的数据体
  * 返回: 0写入失败 1写入成功
  * 功能: 往mlistHistoryRing的Buff"压"入一个数据
  * 说明: "压"满了则覆盖掉原来的数据
----------------------------------------------------------------*/
void Push_menu_historyPar(menu_history_t i)
{
	wegui.menuHistoryRing.Tail = (wegui.menuHistoryRing.Tail+1)%MENU_DEEP;//数组下标+1,满了就回到0
	if(wegui.menuHistoryRing.Lenght < MENU_DEEP)
  {
    wegui.menuHistoryRing.Lenght++;//储存深度没有满+1
  }

  wegui.menuHistoryRing.Buff[wegui.menuHistoryRing.Tail].cursor_id = i.cursor_id;
	wegui.menuHistoryRing.Buff[wegui.menuHistoryRing.Tail].posi      = i.posi;
}

/*--------------------------------------------------------------
  * 名称: menu_history_t* Pop_menu_historyPar(void)
  * 传入: 无
  * 返回: "出栈"读取最新的一个数据
  * 功能: 往wegui.menuHistoryRing的Buff里顺序回读一个数据的指针
  * 说明: 当Buff空时,返回0x00
----------------------------------------------------------------*/
menu_history_t* Pop_menu_historyPar(void)
{
	static menu_history_t i;
	if(wegui.menuHistoryRing.Lenght > 0)//储存有数据
	{
		wegui.menuHistoryRing.Lenght--;
		i.cursor_id = wegui.menuHistoryRing.Buff[wegui.menuHistoryRing.Tail].cursor_id;
		i.posi      = wegui.menuHistoryRing.Buff[wegui.menuHistoryRing.Tail].posi;

		if(wegui.menuHistoryRing.Tail > 0)
		{
			wegui.menuHistoryRing.Tail--;
		}
		else
		{
			wegui.menuHistoryRing.Tail = MENU_DEEP-1;
		}
		return &i;
	}
	else//没有储存数据了
	{
		return 0x00;
	}
}

/*--------------------------------------------------------------
  * 名称: wegui_enter_menu(menu_t* p)
  * 传入: p:菜单结构体
  * 功能: 进入p菜单
----------------------------------------------------------------*/
void wegui_enter_menu(menu_t* p)
{
		switch(p->menuType)
		{
			case wSlider:
			{
				wegui_push_slider_tip		(8, //Y位置
				                         wegui_get_string(p->menuPar.wSliderTip_Par.tip_string,wegui.setting.language), //提示文字
				                         p->menuPar.wSliderTip_Par.pvalue, //参数指针
				                         p->menuPar.wSliderTip_Par.min ,//最小值
				                         p->menuPar.wSliderTip_Par.max,//最大值
				                         VALUE_CHANGE_AND_UPDATE,//实时更新值
				                         p->menuPar.wSliderTip_Par.Change_Value_func,//改变数值执行的函数的指针
				                         p->menuPar.wSliderTip_Par.End_tip_func);//确定数值执行的函数的指针
				if(p->menuPar.wSliderTip_Par.Push_tip_func !=0x00)
					p->menuPar.wSliderTip_Par.Push_tip_func();//执行函数
			}break;
			case mList:
			{
				if(p==0x00){mList_par.cursor_id=0;mList_par.list_y_offset_target=0;return;}//没有菜单

				switch(wegui.menu->menuType)
				{
					case mList:
					{
						if(wegui.menu->menuPar.mList_Par.quit_fun!=0x00)
							wegui.menu->menuPar.mList_Par.quit_fun();//执行函数
					}break;
					case mPorgram:
					{
						if(wegui.menu->menuPar.mPorgram_Par.quit_fun!=0x00)
							wegui.menu->menuPar.mPorgram_Par.quit_fun();//执行函数
					}break;
					default:break;
				}
				
				wegui.menu = p;//切进新的菜单
				wegui_mList_Init();

				if(wegui.menu->menuPar.mList_Par.begin_fun!=0x00)
					wegui.menu->menuPar.mList_Par.begin_fun();//执行函数

			}break;
			case mPorgram:
			{
				//保存当前菜单位置
				switch(wegui.menu->menuType)
				{
					case mList:
					{
						if(wegui.menu->menuPar.mList_Par.quit_fun!=0x00)
							wegui.menu->menuPar.mList_Par.quit_fun();//执行函数
					}break;
					case mPorgram:
					{
						if(wegui.menu->menuPar.mPorgram_Par.quit_fun!=0x00)
							wegui.menu->menuPar.mPorgram_Par.quit_fun();//执行函数
					}break;
					default:break;
				}
				wegui.menu = p;//切进新的菜单
				if(wegui.menu->menuPar.mPorgram_Par.begin_fun!=0x00)
					wegui.menu->menuPar.mPorgram_Par.begin_fun();//执行函数

				if(wegui.menu->menuPar.mPorgram_Par.refresh_fun == 0x00)
				{
					//--清屏--
					lcd_clear_gram();
					while(LCD_Refresh()!=0);
				}
			}break;
			case wMessage:
			{
				char* str;
				if(p->menuPar.wMessage_Par.Press_func != 0x00)
				{
					p->menuPar.wMessage_Par.Press_func();
				}
				str = wegui_get_string(p->menuPar.wMessage_Par.Tip_string,wegui.setting.language);
				if(str != 0x00)
					wegui_push_message_tip(8, str, 3000);//推送提示信息, (推送y位置, 提示内容字符串, 展示时间ms)

			}break;
			default:break;
		}
}

//刷新屏幕帧率
static void reflash_fps(uint16_t stick)
{
	static uint16_t display_count = 0;//更新数据计时
	static uint8_t  sum_count=0;//次数
	static uint16_t sum=1;//总数
	display_count += stick;
	sum += wegui.sysInfo.fps_time;
	sum_count++;

	if(display_count > 100)//更新时间ms
	{
		//帧率 = 1000/(总时间/次数)
		wegui.sysInfo.info_fps = (uint32_t)1000 * sum_count / sum;
		//wegui.sysInfo.info_fps = 1000 / (sum / sum_count);
		//if(wegui.sysInfo.info_fps > 99){wegui.sysInfo.info_fps = 99;}
		display_count = 0;
		sum_count = 0;
		sum = 1;
	}
}

//刷新CPU负载百分比
static void reflash_cpuLoad(uint16_t stick)
{
	static uint16_t display_count = 0;
	static uint8_t  sum_count=0;//次数
	static uint16_t sum=0;//总数
	display_count += stick;
	sum += wegui.sysInfo.cpu_time;
	sum_count++;
	if(display_count > 151)//更新时间ms
	{
		//占用cpu_load = 100% / (刷屏占用时间/设置帧率时间)
		if(wegui.setting.screen_fps_ms != 0)
		{
			wegui.sysInfo.cpu_load = ((uint16_t)100*sum / sum_count)/wegui.setting.screen_fps_ms;
		}
		else
		{
			wegui.sysInfo.cpu_load=100;
		}
		if(wegui.sysInfo.cpu_load>100){wegui.sysInfo.cpu_load = 100;}

		display_count = 0;
		sum_count = 0;
		sum = 0;
	}
}


//更新显示系统信息,调试信息,帧率,cpu
static void wegui_update_info()
{
	//右上角实时显示帧数
		char str[10];
		uint8_t y=0;

	//--调整体字---
	//const fonts_ascii_t *i  = lcd_driver.fonts_ASCII;
	//lcd_driver.fonts_ASCII = &mcu_fonts_ascii_songti_6X12;

	//---1.边框---
	lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_DEBUG_BAR_BORDER);
	lcd_draw_box( SCREEN_WIDTH - 1 - ((lcd_driver.fonts_ASCII->width + lcd_driver.fonts_ASCII->scape)* 7-lcd_driver.fonts_ASCII->scape )-1,
	                SCREEN_HIGH - 1 - lcd_driver.fonts_ASCII->high * 2-1,
	                SCREEN_WIDTH - 1,
	                SCREEN_HIGH - 1);

	//---2.底色---
	#if (COLOUR_DEBUG_BAR_BORDER != COLOUR_DEBUG_BAR_BG)//判断有无需要调整笔刷
		lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_DEBUG_BAR_BG);
	#endif
	lcd_fill_box( SCREEN_WIDTH - 1 - ((lcd_driver.fonts_ASCII->width + lcd_driver.fonts_ASCII->scape)* 7-lcd_driver.fonts_ASCII->scape),
	                SCREEN_HIGH - 1 - lcd_driver.fonts_ASCII->high * 2,
	                SCREEN_WIDTH - 1,
	                SCREEN_HIGH - 1);

	//---3.文字---
	#if (COLOUR_DEBUG_BAR_BORDER != COLOUR_DEBUG_BAR_BG)//判断有无需要调整笔刷
		lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_DEBUG_BAR_BG);
	#endif
	lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_DEBUG_BAR_TEXT);
	//--------显示主频---------

//		str[0]='F';str[1]='R';str[2]=':';itoa(SystemCoreClock/1000000,&str[3],10);str[6]='M';str[7]='\0';
//		lcd_draw_utf8_string	( SCREEN_WIDTH - 1 - lcd_get_utf8_string_xlen(str),//x
//														y++*lcd_driver.fonts_ASCII->high,//y
//														str);
		//--------显示CPU---------
		str[0]='C';str[1]='P';str[2]='U';str[3]=':';
		my_itoa(wegui.sysInfo.cpu_load,&str[4],10);
		lcd_draw_utf8_string(  SCREEN_WIDTH - 1 - lcd_get_utf8_string_xlen(str),//x
														SCREEN_HIGH - 1 - lcd_driver.fonts_ASCII->high - y++*lcd_driver.fonts_ASCII->high,//y
														str);



	//--------显示PFS---------
		str[0]='F';str[1]='P';str[2]='S';str[3]=':';
		my_itoa(wegui.sysInfo.info_fps,&str[4],10);
		lcd_draw_utf8_string(  SCREEN_WIDTH - 1 - lcd_get_utf8_string_xlen(str),//x
														SCREEN_HIGH - 1 - lcd_driver.fonts_ASCII->high - y++*lcd_driver.fonts_ASCII->high,//y
														str);

	//--恢复体字---
	//lcd_driver.fonts_ASCII = i;
}

void wegui_loop_func()
{
	//两者帧率参数设为一致,获得最佳的视觉和性能体验

	//可变帧率
	#define UI_DRAW_TIME_MS        wegui.setting.ui_fps_ms     //ui绘制速度   10ms=100hz 5ms=200hz 4=240hz 3=333hz 2=500hz 1=1000hz
	#define SCREEN_REFRESH_TIME_MS wegui.setting.screen_fps_ms //屏幕刷新速度 10ms=100hz 5ms=200hz 4=240hz 3=333hz 2=500hz 1=1000hz

	//固定帧率
	//#define UI_DRAW_TIME_MS        fps_2_ms(100)  //ui绘制速度   100Hz
	//#define SCREEN_REFRESH_TIME_MS fps_2_ms(100) //屏幕刷新速度  100Hz

	static uint16_t ui_time_count  = 0; //ui绘图计时器
	static uint16_t fps_time_count = 0; //帧率计时器
	static uint8_t ui_farmes;
	static uint16_t time_count=0;
	uint8_t fps_farmes;
	uint8_t refresh = 1;
	uint16_t stick = wegui.ms_stick;
	wegui.ms_stick = 0;

	ui_time_count  += stick;
	time_count += stick;

	if(UI_DRAW_TIME_MS!=0)
	{
		ui_farmes  += ui_time_count / UI_DRAW_TIME_MS;
		ui_time_count %= UI_DRAW_TIME_MS;
	}
	else
	{
		ui_farmes  = 255;
	}

	fps_time_count += stick;
	fps_farmes = fps_time_count / SCREEN_REFRESH_TIME_MS;

	#if (0) //ui没刷新,禁止屏幕刷新
	if((lcd_is_busy()==0)&&(fps_farmes > 0)&&(ui_farmes > 0))
	{
	#else  //就算ui没刷新,屏幕也会重新刷一遍(浪费资源)
	if((lcd_is_busy()==0)&&(fps_farmes > 0))
	{
	#endif
		{
			//------------------屏幕刷新前自定义的操作---------------------

			//------------------------开始刷屏--------------------------
			do
			{
				while(lcd_is_busy()!=0);
				//--------------------绘制对应菜单------------------------
				switch (wegui.menu->menuType)
				{
					case mPorgram:  //自定义功能APP菜单
					{
						if(wegui.menu->menuPar.mPorgram_Par.refresh_fun != 0x00)
						{
							refresh = wegui.menu->menuPar.mPorgram_Par.refresh_fun(ui_farmes,time_count);
						}
						else
						{
							lcd_clear_gram();
							refresh = 1;
						}
						//--------------------绘制弹窗------------------------
						wegui_show_tip(ui_farmes,time_count);
						//-------------------绘制调试窗-----------------------
						if(wegui.setting.debug_windows_en)
						{
							wegui_update_info();
						}
					}break;
					case mList:     //列表菜单菜单
					{
						lcd_clear_gram();
						//--------------------绘制菜单------------------------
						wegui_show_mList(ui_farmes);
						//--------------------绘制弹窗------------------------
						wegui_show_tip(ui_farmes,time_count);
						//-------------------绘制调试窗-----------------------
						if(wegui.setting.debug_windows_en)
						{
							wegui_update_info();
						}
					}break;
					case wCheckBox: //控件(不会进入该菜单):复选框菜单
					case wSlider:   //控件(不会进入该菜单):滑条菜单
					default:
						break;
				}
				ui_farmes = 0;
				time_count = 0;
				if(refresh==0){break;}//0不刷新屏幕
			}while(LCD_Refresh());



			//---刷新调试窗的信息---
			{
				static uint16_t i;
				wegui.sysInfo.cpu_time = wegui.ms_stick ;//更新"刷屏一次cpu占用时间"
				reflash_cpuLoad(fps_time_count-i);//更新CPU负载百分比
				reflash_fps(fps_time_count);//更新帧率
				wegui.sysInfo.fps_time = fps_time_count - i; //更新"两次刷屏间隔时间"
				fps_time_count %= SCREEN_REFRESH_TIME_MS;
				i = fps_time_count;
			}
		}
	}
	//-------------------------菜单的自定义LOOP任务---------------------------
	switch (wegui.menu->menuType)
	{
			case 	mList:     //列表菜单
			{
				if(wegui.menu->menuPar.mList_Par.loop_fun != 0x00)
				{
					wegui.menu->menuPar.mList_Par.loop_fun();
				}
			}break;
			case mPorgram:  //自定义功能APP
			{
				if(wegui.menu->menuPar.mPorgram_Par.loop_fun != 0x00)
				{
					wegui.menu->menuPar.mPorgram_Par.loop_fun();
				}
			}break;
			case wCheckBox: //控件:复选框
			case wSlider:   //控件:滑条
			default:
				break;
	}
	//-------------------------菜单的LOOP任务---------------------------


	if(stick>0)//stick含义: 自上次运行到此, 过了stick毫秒
	{
		#if(WEGUI_PORT != 0)
		wegui_port_task(stick);//按键/编码器/蜂鸣器/自定义
		#endif
	}

	#if (WEGUI_UART_EN)
		wegui_uart_rx_stick(stick);//串口处理 stick传递时间用于计时 (需要持续判断防止漏码)
	#endif
}




/*
 * @brief: 1ms定时器中断处理函数
 * @param: None
 * @retval: None
*/
void wegui_1ms_stick()
{
	if(wegui.ms_stick < 65535)
	{
		wegui.ms_stick++;
	}

	//-------交互接口定时中断----------
	#if(WEGUI_PORT != 0)
	wegui_port_ms_irq();//按键/编码器/蜂鸣器/自定义中断
	#endif
}

void lcd_wegui_init()
{
	//-------交互接口初始化----------
	#if(WEGUI_PORT != 0)
	wegui_port_init();//按键/编码器/蜂鸣器/自定义
	#endif
	
	//-------//UART通讯接口----------
	#if (WEGUI_UART_EN)
	wegui_uart_port_init();//uart接口初始化,缓冲区初始化
	#endif

	wegui.menu = 0x00;//开机初始菜单menu
	wegui.setting.language = zh_CN; //默认语言设置 en_US  zh_CN

	wegui.setting.screen_fps_ms = fps_2_ms(1000);//屏幕帧率设置(建议两者一致) 建议60~100
	wegui.setting.ui_fps_ms = fps_2_ms(60);//UI帧率设置(建议两者一致) 建议60~100
	wegui.setting.debug_windows_en = STARTUP_DEBUG_WINDOWS_EN;//右小角调试窗口使能
	wegui.setting.voice_volume = 5;//开机音量

	wegui.sysInfo.info_fps = 0;//CPU负载
	wegui.sysInfo.cpu_load = 0;//实时刷新率
	wegui.sysInfo.cpu_time = 0;//刷屏资源占用时间
	wegui.sysInfo.fps_time = 0;//刷屏间隔时间

	wegui.setting.brightness = 127;//亮度

	wegui_mList_Init();

	//--设置默认主题颜色--
	#if (LCD_TYPE == LCD_RGB565)
		#if (LCD_COLOUR_BIT>=1)
		rgb_set_driver_colour(0,COLOUR_MLIST_DEFAULT_0);
		rgb_set_driver_colour(1,COLOUR_MLIST_DEFAULT_1);
		#endif
		#if (LCD_COLOUR_BIT>=2)
		rgb_set_driver_colour(2,COLOUR_MLIST_DEFAULT_2);
		rgb_set_driver_colour(3,COLOUR_MLIST_DEFAULT_3);
		#endif
		#if (LCD_COLOUR_BIT>=3)
		rgb_set_driver_colour(4,COLOUR_MLIST_DEFAULT_4);
		rgb_set_driver_colour(5,COLOUR_MLIST_DEFAULT_5);
		rgb_set_driver_colour(6,COLOUR_MLIST_DEFAULT_6);
		rgb_set_driver_colour(7,COLOUR_MLIST_DEFAULT_7);
		#endif
	#endif
	wegui.ms_stick = 0;
}
