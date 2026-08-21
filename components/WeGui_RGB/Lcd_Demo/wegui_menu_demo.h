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
#ifndef _WEGUI_MENU_DEMO_H_
#define _WEGUI_MENU_DEMO_H_

#include "stdint.h"
#include "lcd_wegui_driver.h"

extern menu_t m_main;
//-------------------m_main主菜单------------------------
extern menu_t m_wDemo;
extern menu_t m_App_MusicPlayer;
extern menu_t m_AppGame;
extern menu_t m_App_VideoPleayer;
extern menu_t m_App_Setting;
extern menu_t m_App_6;
extern menu_t m_App_7;
extern menu_t m_App_8;
extern menu_t m_App_9;
extern menu_t m_App_10;

//----------------m.wDemo控件Demo-----------------------
extern menu_t m_wDemo_wMessage_Tip;
extern menu_t m_wDemo_wMessage_ADC;
extern menu_t m_wDemo_wMessage_Pres;
extern menu_t m_wDemo_wMessage_Pres2;
extern menu_t m_wDemo_wCheckbox;
extern menu_t m_wDemo_wSlider1;
extern menu_t m_wDemo_wSlider2;

//-------------------m.Setting设置------------------------
extern menu_t m_Setting_Display;
extern menu_t m_Setting_Time;
extern menu_t m_Setting_Speaker;
extern menu_t m_Setting_UI;
extern menu_t m_Setting_Developer;
extern menu_t m_Setting_Language;
extern menu_t m_Setting_About;

//-------------------m.Setting.Language语言------------------------
extern menu_t m_Setting_Language_English;
extern menu_t m_Setting_Language_Chinese;

//-------------------m.Setting.Display显示------------------------
extern menu_t m_Setting_Display_ScreenFPS;
extern menu_t m_Setting_Display_Brightness;
extern menu_t m_Setting_Display_UI_Speed;

//-------------------m_Setting_UI主题------------------------
extern menu_t m_Setting_UI_colour;

//-------------------m.Setting.Speaker声音------------------------
extern menu_t m_Setting_Speaker_Volume;

//---------------m.Setting.Developer开发者选项---------------------
extern menu_t m_Setting_Developer_wDebugInfoEn;//调试窗口

//---------------m.Setting.About关于---------------------
extern menu_t m_Setting_About_wCpu;
extern menu_t m_Setting_About_wSoft;



extern uint32_t demo_timer;
extern uint16_t demo_value;
extern uint8_t demo_bool;
#endif

