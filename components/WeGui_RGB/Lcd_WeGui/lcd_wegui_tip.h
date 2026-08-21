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
#ifndef _LCD_WEGUI_TIP_H_
#define _LCD_WEGUI_TIP_H_

#include "lcd_wegui_driver.h"

//wegui内部显示调用
void wegui_show_tip(uint16_t farmes, uint16_t Tms);
//减值 操作成功返回1 操作失败返回0
uint8_t wegui_tip_value_dec(void);
//加值 操作成功返回1 操作失败返回0
uint8_t wegui_tip_value_add(void);
//直接退出
void wegui_tip_quit(void);
//保存值并且退出
void wegui_tip_save_and_quit(void);


/*--------------------------------------------------------------
  * 名称: wegui_push_message_tip(int16_t y, uint8_t* string, uint16_t time)
  * 传入1: y显示y位置
  * 传入2: string推送的字符串
  * 传入3: time推送显示时间
  * 功能: 像wegui屏幕推送一条消息
----------------------------------------------------------------*/
void wegui_push_message_tip(int16_t y, char* string, uint16_t time);//(推送y位置,推送字符串,推送显示时间)

/*--------------------------------------------------------------
  * 名称: wegui_push_slider_tip(int16_t y, uint8_t* string, int16_t *p_value,int16_t value_min ,uint16_t value_max,value_change_t change_way,void(*Change_func)(),void(*Finish_func)())
  * 传入1: y:推送显示y位置
  * 传入2: string推送的字符串
  * 传入3: *p_value 待修改的数据指针
  * 传入4: value_min 待修改的数据的最小值
  * 传入5: value_max 待修改的数据的最大值
  * 传入6: change_way 改值方式
  * 传入7: *Change_func 滑条数据改变执行一次的函数指针
  * 传入8: *Finish_func 滑条数据确认后执行一次的函数指针
  * 功能: 向wegui屏幕推送一条滑条控制窗
----------------------------------------------------------------*/
void wegui_push_slider_tip(int16_t y, char* string, int16_t *p_value,int16_t value_min ,uint16_t value_max,value_change_t change_way,void(*Change_func)(),void(*Finish_func)());

#endif
