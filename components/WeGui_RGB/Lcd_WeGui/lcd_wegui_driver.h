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
#ifndef _LCD_WEGUI_DRIVER_H_
#define _LCD_WEGUI_DRIVER_H_

#include "lcd_wegui_config.h"

#ifndef MENU_DEEP
	#define MENU_DEEP (5)//设置菜单的最大层数(用于分配变量,储存光标历史位置,取小了返回菜单时可能不会回到上一次的位置)
#endif

#define fps_2_ms(x) ((uint16_t)1000/x) //输入FPS返回毫秒,不能是0

//-----非线性动画1-----
/*--------------------------------------------------------------
  * 名称: #define Value_Change_PID_P(cur_value,target_value,P,count)
  * 传入1: cur_value 当前值
  * 传入2: target_value目标值
	* 传入3: P:[0:15] 0:变化最慢 16变化最慢
	* 传入4: count:连续处理count次
  * 功能: 使用P(PID)的方式,使当前值逼近目标值
----------------------------------------------------------------*/
#define Value_Change_PID_P(cur_value,target_value,P,count) \
do                                                   \
{                                                    \
	uint8_t f=count;                                   \
	while(f>0)                                         \
	{                                                  \
		f--;                                             \
		if(cur_value > target_value)                     \
		{                                                \
			uint16_t i = cur_value - target_value;         \
			i = i>>P;                                      \
			if(i==0){i=1;}                                 \
			cur_value-=i;                                  \
		}                                                \
		else if (cur_value < target_value)               \
		{                                                \
			uint16_t i = target_value - cur_value;         \
			i = i>>P;                                      \
			if(i==0){i=1;}                                 \
			cur_value+=i;                                  \
		}                                                \
	}                                                  \
}while(0)


//--------------------语言--------------------
//-------语言类型--------
typedef enum language
{
	en_US=0,//英语
	zh_CN,//简体中文
	zh_TC,//繁体中文
}langage_t;

//-----多语言字符串------
typedef struct language_string_t
{
	/*const*/ char * str_en_US;//ENGLISH
	/*const*/ char * str_zh_CN;//简体
	/*const*/ char * str_zh_TC;//繁体
}wegui_string_t;
//--------------------设置--------------------
typedef struct Wegui_t_setting
{
	langage_t language;    //系统语言
	int16_t brightness;   //系统亮度
	int16_t ui_fps_ms;    //ui绘制时间(建议与刷屏同步)
	int16_t screen_fps_ms;//屏幕刷新时间(建议与UI同步)
	uint8_t debug_windows_en;//调试窗口使能
	int16_t voice_volume;//音量

}setting_t;
//-----------------系统信息------------------
typedef struct Wegui_sysInfo
{
	uint16_t info_fps;//实时刷新率
	uint16_t cpu_load;//CPU负载
	uint16_t cpu_time;//刷屏函数总占用时间
	uint16_t fps_time;//屏幕刷新时间
}sysInfo_t;
//------------------菜单&控件------------------

//----菜单&控件类型----
typedef enum menuType
{
	mList,     //列表菜单
  mPorgram,  //自定义功能APP
	wCheckBox, //控件:复选框
	wSlider,   //控件:滑条
	wMessage,  //控件:消息
}menuType_t;

//------mList参数------
typedef struct mList_Propertys
{
	void (*begin_fun)();//菜单进入 执行一次
	void (*loop_fun)(); //菜单功能 持续执行
	void (*quit_fun)(); //菜单退出 执行一次
	void (*pstr_control_func)(uint16_t value);
}mList_Par_t;

//-----mPorgram参数-----
typedef struct mPorgram_Propertys
{
	void (*begin_fun)();//菜单进入 执行一次
	void (*loop_fun)(); //菜单功能 持续执行
	void (*quit_fun)(); //菜单退出 执行一次
	uint8_t (*refresh_fun)(uint8_t ui_farmes,uint16_t time_count); //刷新屏幕时执行,放绘图函数 //返回0不刷屏 返回其他值正常刷屏
	//1.ui_farmes计时帧,动画用
	//假设设定了屏幕刷新率为60帧(16ms),
	//等于1时表示走了16ms,等于2时表示走了32ms,可用于动画切帧,
	//等于0时表示与上一次刷新屏幕还未计到16ms,此时可以不切帧
	//2.time_count表示与上次刷屏过了多少ms,可用于时间计时
}mPorgram_Par_t;

//----wCheckBox参数-----
typedef struct wCheckBox_Propertys
{
	uint8_t *pstr;         //复选框控制的数据指针
	void (*Change_Value)();//值被修改 执行一次
}wCheckBox_Par_t;

//------wSlider参数------
typedef struct wSliderTip_Propertys
{
	void (*Push_tip_func)();    //控件进入 执行一次
	void (*Change_Value_func)();//值被修改 执行一次
	void (*End_tip_func)();     //控件退出 执行一次
	int16_t *pvalue;              //滑条控制的数据指针
	wegui_string_t tip_string;    //提示框字符串
	int16_t min;  //整数最小值
	int16_t max;  //整数最大值

}wSliderTip_Par_t;

typedef struct wMessage_Propertys
{
	void (*Press_func)();              //按下执行一次
	char* Value_string;                //末尾显示的值 0x00不显示
	wegui_string_t Tip_string;         //提示框字符串
}wMessage_Par_t;

//-------参数共同体-------
typedef union menuType_Propertys
{
  mList_Par_t      mList_Par;
	mPorgram_Par_t   mPorgram_Par;
  wCheckBox_Par_t	 wCheckBox_Par;
	wSliderTip_Par_t wSliderTip_Par;
	wMessage_Par_t wMessage_Par;
}menuType_Par_t;


//-------菜单结构体-------
#if (MENU_CONST_DIS)
	typedef struct menu_t
	{
	
			struct menu_t * fatherMenu;//父菜单
			struct menu_t * subMenu;//(首个)子菜单
			struct menu_t * nextMenu;//同级下一个菜单
			wegui_string_t titel;//多语言标题
			wegui_string_t discribe;//多语言描述
			menuType_t menuType;//菜单类型
			menuType_Par_t menuPar;//菜单属性
	}menu_t;
#else
	typedef const struct menu_t
	{
		
			const struct menu_t * fatherMenu;//父菜单
			const struct menu_t * subMenu;//(首个)子菜单
			const struct menu_t * nextMenu;//同级下一个菜单
			const wegui_string_t titel;//多语言标题
			const wegui_string_t discribe;//多语言描述
			const menuType_t menuType;//菜单类型
			const menuType_Par_t menuPar;//菜单属性
	}menu_t;
#endif

//------------------弹窗结构体------------------

typedef enum tip_type
{
	message,//消息弹窗
	slider,//滑条弹窗
}tip_type_t;
typedef enum value_change_type
{
	VALUE_CHANGE_AND_UPDATE,//值实时更新
	VALUE_DONE_AND_UPDATE,//值点击"确定"后才更新
}value_change_t;
typedef enum tip_state
{
	FREE = 0x00,//弹窗空闲
	ENTERING,//弹窗正在进入
	DISPLAYING,//弹窗正在展示
	EXITING,//弹窗正在退出
}tip_state_t;
typedef struct Wegui_tip
{
	tip_state_t state;//有无弹窗
	tip_type_t type;//当前弹窗的类型
	uint8_t fonts_high;

	int16_t   pos_y;//目标位置y
	int16_t   pos_x;//目标位置x
	int16_t   cur_y;//当前位置y
	int16_t   cur_x;//当前位置x
	int16_t   time;//时间
	char*  string;//字符串


	int16_t* pvalue;//待修改的参数的指针(调值弹窗有效)
	int16_t show_Value;//弹窗显示值
	int16_t pvalue_min;//待修改的参数的最小值(调值弹窗有效)
	int16_t pvalue_max;//待修改的参数的最大值(调值弹窗有效)
	value_change_t change_way;//改值方式

	void (*Change_Value)();
	void (*Finish_Value)();

}Wegui_tip_t;
//#include "user_wegui_menu.h"
//-------菜单历史记录结构体-------
typedef struct menu_history
{
	uint8_t cursor_id;//光标历史记录
	int16_t posi;//位置历史记录
}menu_history_t;
//-------环形历史记录缓冲区-------
typedef struct
{
    uint8_t Tail;
    uint8_t Lenght;
    menu_history_t Buff[MENU_DEEP];//深度
}HistoryRing_t;
extern HistoryRing_t HistoryRing;


//------------------主结构体------------------
typedef struct wegui_t
{
	menu_t *menu;//菜单
	HistoryRing_t menuHistoryRing;//菜单历史记录环形缓冲区 记录光标位置和菜单位置
	Wegui_tip_t tip;//弹窗
	
	setting_t setting;
	sysInfo_t sysInfo;
	
	uint16_t ms_stick;//运行时间基准
}wegui_t;

#include "lcd_driver.h"
#include "lcd_res.h"
#include "lcd_wegui_menu_mlist.h"
#include "lcd_wegui_tip.h"

/*--------------------------------------------------------------
  * 名称: *my_itoa(int16_t num,uint8_t *str,uint8_t radix)
  * 传入1: num 数字
  * 传入2: *str (空)字符串指针
	* 传入3: radix 进制数
  * 功能: 数字转成字符串,保存到*str里
  * 说明:iota = integer to alphanumeric 把整型数转换成字符串
----------------------------------------------------------------*/
char *my_itoa(int16_t num,char *str,uint8_t radix);

/*--------------------------------------------------------------
  * 名称: Get_submenu_sum(menu_t* m)
  * 传入: m:查询的菜单
  * 功能: 获取菜单中子菜单的总数
----------------------------------------------------------------*/
uint8_t Get_submenu_sum(menu_t* m);//获取菜单中子菜单的总数

/*--------------------------------------------------------------
  * 名称: uint8_t* wegui_get_string(wegui_string_t object,langage_t language)
  * 传入1: object 语言包
  * 传入2: language 语言
  * 功能: 返回"语言包"里对应的"language语言"字符串指针,
----------------------------------------------------------------*/
char* wegui_get_string(wegui_string_t object,langage_t language);

/*--------------------------------------------------------------
  * 名称: menu_history_t* Pop_menu_historyPar(void)
  * 传入: 无
  * 返回: "出栈"读取最新的一个数据
  * 功能: 往HistoryRing的Buff里顺序回读一个数据的指针
  * 说明: 当Buff空时,返回0x00
----------------------------------------------------------------*/
menu_history_t* Pop_menu_historyPar(void);

/*--------------------------------------------------------------
  * 名称: void Push_menu_historyPar(menu_history_t i)
  * 传入: menu_history_t i需要储存的数据体
  * 返回: 0写入失败 1写入成功
  * 功能: 往mlistHistoryRing的Buff"压"入一个数据
  * 说明: "压"满了则覆盖掉原来的数据
----------------------------------------------------------------*/
void Push_menu_historyPar(menu_history_t i);

/*--------------------------------------------------------------
  * 名称: wegui_enter_menu(menu_t* p)
  * 传入: p:菜单结构体
  * 功能: 进入p菜单
----------------------------------------------------------------*/
void wegui_enter_menu(menu_t* p);//进入指定菜单或打开指定控件

void wegui_loop_func(void);//放到主循环
void wegui_1ms_stick(void);//放到1ms中断
void lcd_wegui_init(void);//主循环前执行一次

extern wegui_t wegui;

#endif
