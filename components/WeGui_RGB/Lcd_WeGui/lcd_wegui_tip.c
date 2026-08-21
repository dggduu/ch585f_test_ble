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
#include "lcd_wegui_tip.h"

//----消息弹窗----
#define TIP_BOX_SHADOW_THICK 2   //边框阴影的厚度
#define TIP_BOX_R 3           //倒圆角
#define TIP_LR_Scape 4				//文字和边框左右的间距
#define TIP_TB_Scape 4				//文字和边框上下的间距
//----滑条弹窗----
#define BAR_TIP_TOP_SCAPE 4  //弹窗顶部预留空间(顶部到文字)
#define BAR_TIP_BOT_SCAPE 4  //弹窗底部预留空间(底部到进度条)
#define BAR_TO_CHAR_SCAPE 4  //进度条到文字的空间
#define BAR_TIP_SIDE_SCAPE 4 //弹窗到进度侧边的距离

#define BAR_WIDTH (SCREEN_WIDTH-32) //进度条的宽度
#define BAR_HIGHT 3					//进度条的高度

#define BAR_TIP_HIGHT (wegui.tip.fonts_high*string_yline + BAR_TIP_TOP_SCAPE + BAR_TO_CHAR_SCAPE + BAR_HIGHT + BAR_TIP_BOT_SCAPE)	//弹窗高度
#define BAR_TIP_WIDTH (BAR_WIDTH + BAR_TO_CHAR_SCAPE*2) //弹窗宽度, 双数优先



/*--------------------------------------------------------------
  * 名称: wegui_push_message_tip(int16_t y, uint8_t* string, uint16_t time)
  * 传入1: y显示y位置
  * 传入2: string推送的字符串
  * 传入3: time推送显示时间
  * 功能: 向wegui屏幕推送一条消息
----------------------------------------------------------------*/
void wegui_push_message_tip(int16_t y, char* string, uint16_t time)//(推送y位置,推送字符串,推送显示时间)
{
	uint16_t string_xlen;
	uint16_t string_yline;
	if(string[0] != 0x00)
	{
		wegui.tip.fonts_high = 16;
		//--文字高度--
		switch(wegui.setting.language)
		{
			case zh_CN://中文
				if(lcd_driver.fonts_UTF8 != 0x00)
				{
					wegui.tip.fonts_high = lcd_driver.fonts_UTF8->high;
					break;
				}
			case en_US://英语
			default:
				if(lcd_driver.fonts_ASCII != 0x00)
				{
					wegui.tip.fonts_high = lcd_driver.fonts_ASCII->high;
				}
				break;
		}
		string_xlen = lcd_get_utf8_string_xlen(string);
		string_yline = lcd_get_utf8_yline(string);//字符串分了多少行

		wegui.tip.state = ENTERING;
		wegui.tip.type=message;
		wegui.tip.pos_y = y;																							//动画目标位置y
		wegui.tip.pos_x = -(int16_t)string_xlen/2 - TIP_LR_Scape + ((SCREEN_WIDTH/2)-1);	//动画目标位置x

		//初始位置隐藏在上方
		//wegui.tip.cur_y = SCREEN_HIGH -(-TIP_TB_Scape-TIP_TB_Scape - wegui.tip.fonts_high);	//开始位置Y
		wegui.tip.cur_y = (-TIP_TB_Scape-TIP_TB_Scape - wegui.tip.fonts_high*string_yline);	//开始位置Y
		wegui.tip.cur_x = wegui.tip.pos_x;												            //开始位置x

		wegui.tip.time	=	time;//推送显示时间ms
		wegui.tip.string = string;//推送字符内容
	}
}

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
void wegui_push_slider_tip(int16_t y, char* string, int16_t *p_value,int16_t value_min ,uint16_t value_max,value_change_t change_way,void(*Change_func)(),void(*Finish_func)())
{
	uint16_t string_yline = lcd_get_utf8_yline(wegui.tip.string);//字符串分了多少行
	//--更新文字高度--
	switch(wegui.setting.language)
	{
		case zh_CN://中文
			if(lcd_driver.fonts_UTF8 != 0x00)
			{
				wegui.tip.fonts_high = lcd_driver.fonts_UTF8->high;
				break;
			}
		case en_US://英语
		default:
			if(lcd_driver.fonts_ASCII != 0x00)
			{
				wegui.tip.fonts_high = lcd_driver.fonts_ASCII->high;
			}
			break;
	}

	wegui.tip.state = ENTERING;
	wegui.tip.type = slider;
	wegui.tip.pos_x = (SCREEN_WIDTH - BAR_TIP_WIDTH)/2;//弹窗目标位置x(默认居中)
	wegui.tip.pos_y = y;//弹窗目标位置y
	wegui.tip.cur_x = (SCREEN_WIDTH - BAR_TIP_WIDTH)/2;//起始位置x
	wegui.tip.cur_y = -BAR_TIP_HIGHT-TIP_BOX_SHADOW_THICK;//起始位置y
	wegui.tip.time  = 0;
	if(string!=0x00)
		wegui.tip.string=string;
	else
		wegui.tip.string = (char *)" ";
	wegui.tip.time = 0;//当前动画时间
	wegui.tip.Change_Value = Change_func;
	wegui.tip.Finish_Value = Finish_func;
	wegui.tip.change_way = change_way;

	if(p_value==0x00)
	{
		//tip.string=wegui_get_string(p_ValueError_String,wegui.setting.language);
		wegui.tip.string=(char *)"pErr";
		wegui.tip.pvalue = 0x00;
		wegui.tip.show_Value=0;
		wegui.tip.pvalue_min = 0;
		wegui.tip.pvalue_max = 0;
	}
	else
	{
		wegui.tip.pvalue = p_value;
		wegui.tip.show_Value=*p_value;
		wegui.tip.pvalue_min = value_min;
		wegui.tip.pvalue_max = value_max;
	}
}



//保存值并且退出
void wegui_tip_save_and_quit()
{
	switch(wegui.tip.type)
	{
		case message:
		{
			wegui.tip.state = EXITING;
		}break;
		case slider:
		{
			wegui.tip.state = EXITING;
			if(wegui.tip.change_way == VALUE_DONE_AND_UPDATE)
			{
				if(wegui.tip.pvalue != 0x00)
				{
					*wegui.tip.pvalue = wegui.tip.show_Value;
				}
			}
			if(wegui.tip.Finish_Value!=0x00)
			wegui.tip.Finish_Value();
		}break;
	}
}


//直接退出
void wegui_tip_quit()
{
	switch(wegui.tip.type)
	{
		case message:
		{
			wegui.tip.state = EXITING;
		}break;
		case slider:
		{
			wegui.tip.state = EXITING;
		}break;
	}
}

//加值
uint8_t wegui_tip_value_add()
{
	switch(wegui.tip.type)
	{
		case message:break;
		case slider:
		{
			if(wegui.tip.show_Value<wegui.tip.pvalue_max)
			{
				wegui.tip.show_Value += 1;
				if(wegui.tip.change_way == VALUE_CHANGE_AND_UPDATE)//实时更新值
				{
					*wegui.tip.pvalue = wegui.tip.show_Value;
				}
				if(wegui.tip.Change_Value!=0x00)
					wegui.tip.Change_Value();
			}
			else
			{
				return 0;
			}
		}break;
	}
	return 1;//操作有效
}

//减值
uint8_t wegui_tip_value_dec()
{
	switch(wegui.tip.type)
	{
		case message:break;
		case slider:
		{
			if(wegui.tip.show_Value>wegui.tip.pvalue_min)
			{
				wegui.tip.show_Value -= 1;
				if(wegui.tip.change_way == VALUE_CHANGE_AND_UPDATE)//实时更新值
				{
					*wegui.tip.pvalue = wegui.tip.show_Value;
				}
				if(wegui.tip.Change_Value!=0x00)
					wegui.tip.Change_Value();
			}
			else
			{
				return 0;
			}
		}break;
	}
	return 1;//操作有效
}



void wegui_show_tip(uint16_t farmes, uint16_t Tms)
{
	uint8_t string_yline;
	if(wegui.tip.state != FREE)
	{
		if(farmes != 0)
		{
			uint8_t pid_p;
			switch(wegui.tip.state)
			{
				default:
				case ENTERING://弹窗正在进入
					pid_p = 2;//进入动画更快
					break;
				case DISPLAYING://弹窗正在展示
					pid_p = 2;//进入动画更快
					break;
				case EXITING://弹窗正在退出
					pid_p = 3;//退出动画更慢
					break;
			}
			//使用P(PID)的方式,使当前值接近目标值
			//(cur_value:当前变量, target_value目标值, P:[0快:15慢], count:连续处理count次)
			//Value_Change_PID_P(cur_value,target_value,P,count)
			Value_Change_PID_P( wegui.tip.cur_x,//当前x位置
													wegui.tip.pos_x,//目标x位置
													pid_p,
													farmes);

			Value_Change_PID_P( wegui.tip.cur_y,//当前y位置
													wegui.tip.pos_y,//目标y位置
													pid_p,
													farmes);
		}
		string_yline = lcd_get_utf8_yline(wegui.tip.string);//字符串分了多少行
		switch(wegui.tip.type)
		{
			case message:
			{
				uint8_t string_xlen = lcd_get_utf8_string_xlen(wegui.tip.string);

				//----清空区域----
				lcd_set_driver_mode(write_0);//选择笔刷颜色
				lcd_fill_rbox		(wegui.tip.cur_x-TIP_BOX_SHADOW_THICK,
													wegui.tip.cur_y-TIP_BOX_SHADOW_THICK,
													wegui.tip.cur_x+string_xlen+TIP_LR_Scape+TIP_LR_Scape+TIP_BOX_SHADOW_THICK,
													wegui.tip.cur_y+string_yline*wegui.tip.fonts_high+TIP_TB_Scape+TIP_TB_Scape+TIP_BOX_SHADOW_THICK,
													TIP_BOX_R+TIP_BOX_SHADOW_THICK);

				//----绘画边框----
				lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_MESS_TIP_BOX_BORDER);//选择笔刷颜色
				lcd_draw_rbox	(wegui.tip.cur_x ,
															wegui.tip.cur_y,
															wegui.tip.cur_x+string_xlen+TIP_LR_Scape+TIP_LR_Scape,
															wegui.tip.cur_y+string_yline*wegui.tip.fonts_high+TIP_TB_Scape+TIP_TB_Scape,
															TIP_BOX_R);


				//--居中显示提示文字--
				#if(COLOUR_MESS_TIP_TEXT != COLOUR_MESS_TIP_BOX_BORDER)
					lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_MESS_TIP_TEXT);//选择笔刷颜色
				#endif
				lcd_draw_utf8_string	(wegui.tip.cur_x+TIP_LR_Scape,
															wegui.tip.cur_y+TIP_TB_Scape,
															wegui.tip.string);


				if(wegui.tip.time > Tms)
				{
					wegui.tip.time -= Tms;
				}
				else
				{
					wegui.tip.time = 0;
					wegui.tip.state = EXITING;//弹窗正在退出
				}

			}break;
			case slider:
			{
					uint16_t i;
					char string[7];
				
					//----清空区域----
					lcd_set_driver_mode(write_0);//选择笔刷颜色
					lcd_fill_rbox		(wegui.tip.cur_x-TIP_BOX_SHADOW_THICK ,
														wegui.tip.cur_y-TIP_BOX_SHADOW_THICK,
														wegui.tip.cur_x+BAR_TIP_WIDTH+TIP_BOX_SHADOW_THICK,
														wegui.tip.cur_y+BAR_TIP_HIGHT+TIP_BOX_SHADOW_THICK,
														TIP_BOX_R+TIP_BOX_SHADOW_THICK);

					//----绘画边框----
					lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_SLIDER_TIP_BOX_BORDER);//选择笔刷颜色
					lcd_draw_rbox	(wegui.tip.cur_x ,
																wegui.tip.cur_y,
																wegui.tip.cur_x+BAR_TIP_WIDTH,
																wegui.tip.cur_y+BAR_TIP_HIGHT,
																TIP_BOX_R);


					//----文字----
					#if(COLOUR_SLIDER_TIP_BOX_BORDER != COLOUR_SLIDER_TIP_TEXT)
						lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_SLIDER_TIP_TEXT);//选择笔刷颜色
					#endif
					lcd_draw_utf8_string	(wegui.tip.cur_x + BAR_TIP_SIDE_SCAPE,
													wegui.tip.cur_y + BAR_TIP_TOP_SCAPE/* + (string_yline-1)*string_yline*/,
													wegui.tip.string);
					//----数字----
					#if(COLOUR_SLIDER_TIP_TEXT != COLOUR_SLIDER_TIP_NUM)
						lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_SLIDER_TIP_NUM);//选择笔刷颜色
					#endif
					
					my_itoa(wegui.tip.show_Value,string,10);//数值转10进制字符串, 传递回给字符串指针
					lcd_draw_utf8_string	(wegui.tip.cur_x + BAR_TIP_SIDE_SCAPE + BAR_WIDTH - lcd_get_utf8_string_xlen(string),
													wegui.tip.cur_y + BAR_TIP_TOP_SCAPE + (string_yline-1)*wegui.tip.fonts_high,
													string);



					//----进度条进度----
					#if(COLOUR_SLIDER_TIP_NUM != COLOUR_SLIDER_TIP_BAR_PROG)
						lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_SLIDER_TIP_BAR_PROG);//选择笔刷颜色
					#endif
					i = wegui.tip.pvalue_max-wegui.tip.pvalue_min;
					if(i<=0){i=1;}//防止分母等于0
					lcd_fill_rbox				(wegui.tip.cur_x + BAR_TIP_SIDE_SCAPE,
																wegui.tip.cur_y + wegui.tip.fonts_high*string_yline + BAR_TIP_TOP_SCAPE + BAR_TO_CHAR_SCAPE,
																wegui.tip.cur_x  + BAR_TIP_SIDE_SCAPE + (BAR_WIDTH*(wegui.tip.show_Value-wegui.tip.pvalue_min)/i),
																wegui.tip.cur_y + wegui.tip.fonts_high*string_yline + BAR_TIP_TOP_SCAPE + BAR_TO_CHAR_SCAPE + BAR_HIGHT,
																2);

					//----进度条边框----
					#if(COLOUR_SLIDER_TIP_BAR_PROG != COLOUR_SLIDER_TIP_BAR_BORDER)
						lcd_set_driver_mode((lcd_driver_mode_t)COLOUR_SLIDER_TIP_BAR_BORDER);//选择笔刷颜色
					#endif

					lcd_draw_rbox	(wegui.tip.cur_x + BAR_TIP_SIDE_SCAPE,
																wegui.tip.cur_y + wegui.tip.fonts_high*string_yline + BAR_TIP_TOP_SCAPE + BAR_TO_CHAR_SCAPE,
																wegui.tip.cur_x  + BAR_TIP_SIDE_SCAPE + BAR_WIDTH,
																wegui.tip.cur_y + wegui.tip.fonts_high*string_yline + BAR_TIP_TOP_SCAPE + BAR_TO_CHAR_SCAPE + BAR_HIGHT,
																2);


			}break;
		}
		switch(wegui.tip.state)
		{
			default:
			case ENTERING://弹窗正在进入
				if((wegui.tip.cur_x == wegui.tip.pos_x)&&(wegui.tip.cur_y == wegui.tip.pos_y))
				{
					wegui.tip.state = DISPLAYING;//位置已到达
				}
				break;
			case DISPLAYING://弹窗正在展示
				break;
			case EXITING://弹窗正在退出
				switch(wegui.tip.type)
				{
					case message:wegui.tip.pos_y = -TIP_TB_Scape-TIP_TB_Scape - wegui.tip.fonts_high*string_yline;break;//退到屏幕外
					case slider:wegui.tip.pos_y = -BAR_TIP_HIGHT - TIP_BOX_SHADOW_THICK - TIP_BOX_SHADOW_THICK;break;//退到屏幕外
				}
				if((wegui.tip.cur_x == wegui.tip.pos_x)&&(wegui.tip.cur_y == wegui.tip.pos_y))
				{
					wegui.tip.state = FREE;//弹窗结束
				}
				break;
		}
	}
}


