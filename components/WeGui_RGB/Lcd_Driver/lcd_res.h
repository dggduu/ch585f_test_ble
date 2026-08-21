/*
	Copyright 2025 Lu Zhihao
	本程序仅供学习用途, 暂不公开对其他用途的授权
*/
#ifndef LCD_RES_H
#define LCD_RES_H

#include "stdint.h"
#include "lcd_driver_config.h"

/*
	最新字库取模,可使用最新v0.4.4以上的上位机一键导出字库!!
	--------------------------------------------------
	或者使用旧教程,旧教程如下:
	!详细的教程前往git获取doc文件!
	
	UTF8多国语言裁剪字符取模方式:
	1.---------下载取模软件---------
	取模软件FontLab-v1.0.13.2309
	下载地址https://download.csdn.net/download/qq_37749995/90930109
	软件弊端:需要注册登录,但免费
	
	2.------获取unicode_index-------
	(1)选择菜单栏的"字符编码"
	(2)选择输入"文字"
	(3)填写入需要取模的文字,例如"主页游戏时间帧率屏幕刷新"
	(4)取模方式选择"连续",字符编码选择Unicode,点击"查询"
	(5)生成的数组,即为unicode_index的数组
	
	3.---------获取font字库----------
	(1)选择菜单栏的"字库生成器"
	(2)字体样式栏中,选择需要的字体,字号
	(2)字符集栏中,选择"自定义字符",打开"编辑码表"
	(3)输入需要取模的文字,顺序需要与2.相同,例如"主页游戏时间帧率屏幕刷新新",确认
	注意:该取模软件有BUG,最后一个字会遗漏,在此处文字最后补上一个字,如"新"
	(4)排值方式栏中,选择"字节竖置"
	(5)字节方式栏中,选择"低位在前"
	(6)导出字库栏中,选择C文件
	(7)点阵样式栏中,设置取模宽高,和取模位置偏移相关设置
	(8)点击"生成
	(9)生成的数组,即为font字库的数组
	(10)可在生成的字库末尾,找到广告字节并删除
	
	//广告字节:
	0xB1,0xBE,0xCE,0xC4,0xBC,0xFE,0xD3,0xC9,    0x46,0x6F,0x6E,0x74,0x4C,0x61,0x62,0x81,
	0x30,0x85,0x33,0xC9,0xFA,0xB3,0xC9,0xA3,    0xAC,0xBD,0xF6,0xD3,0xC3,0xD3,0xDA,0xBC,
	0xBC,0xCA,0xF5,0xCA,0xB5,0xD1,0xE9,0xA3,    0xAC,0xBD,0xFB,0xD6,0xB9,0xC9,0xCC,0xD2,
	0xB5,0xCA,0xB9,0xD3,0xC3,0xA1,0xA3,
*/

/*
	单字大小byte_size计算方式:
	byte_size = width * (high+7 / 8);
*/

//----------------------------ASCII字体--------------------------------
typedef enum //ascii字体储存方式
{
	ASCII_IN_MCU = 0,  //字体在MCU内部
	ASCII_IN_EX_FLASH, //字体在外部FLASH
}ascii_store_t;

typedef struct
{
	uint8_t byte_size; //单字大小
	uint8_t* font;    //字库
}ascii_IN_MCU_par_t;

typedef struct
{
	uint8_t byte_size;   //单字大小
	uint32_t flash_addr; //字库flash地址
}ascii_in_EX_FLASH_par_t;

typedef union
{
	ascii_IN_MCU_par_t      IN_MCU_par;
	ascii_in_EX_FLASH_par_t IN_EX_FLASH_par;
}ascii_par_t;

typedef const struct 
{
	uint8_t width;    //宽
	uint8_t high;     //高
	uint8_t scape;    //间隔(默认0)
	ascii_store_t store_type; //字体储存类型
	ascii_par_t   store_par;  //字库及参数
}fonts_ascii_t;


//----------------------------UTF8字体--------------------------------
typedef enum //字体储存方式
{
	UTF8_IN_MCU_INDEX=0,           //字体在MCU内部   索引型
	UTF8_IN_EX_FLASH_INDEX,        //字体在外部FLASH 索引型
	UTF8_IN_EX_FLASH_CONTINUOUS,   //字体在外部FLASH 连续型
}utf8_store_t;

typedef struct
{
	uint8_t byte_size;     //单字占用
	uint8_t *unicode_index;//索引数量
	uint8_t index_size;    //索引大小
	uint8_t *font;         //字库
}utf8_in_mcu_index_par_t;

typedef struct
{
	uint8_t byte_size;      //单字占用
	uint8_t *unicode_index; //索引
	uint8_t index_size;     //索引数量
	uint32_t flash_addr;    //字库flash地址
}utf8_in_ex_flash_index_par_t;

typedef struct
{
	uint8_t byte_size;      //单字大小
	uint16_t unicode_start; //unicode范围:起始
	uint8_t unicode_end;    //unicode范围:结束
	uint32_t flash_addr;    //字库flash地址
}utf8_in_ex_flash_continuous_par_t;

typedef union
{
	utf8_in_mcu_index_par_t           IN_MCU_INDEX_par;
	utf8_in_ex_flash_index_par_t      IN_EX_FLASH_INDEX_par;
	utf8_in_ex_flash_continuous_par_t IN_EX_FLASH_CONTINUOUS_par;
}utf8_par_t;

typedef const struct
{
	uint8_t width;    //宽
	uint8_t high;     //高
	uint8_t scape;    //间隔(默认0)
	utf8_store_t store_type;//字体储存类型
	utf8_par_t   store_par; //字库及参数;
}fonts_utf8_t;



//----------------------------字体声明--------------------------------
extern fonts_ascii_t mcu_fonts_ascii_songti_6X12;
extern fonts_ascii_t mcu_fonts_ascii_songti_8X16;
extern fonts_ascii_t mcu_fonts_ascii_songti_12X24;

extern fonts_utf8_t mcu_fonts_utf8_songti_12X12;
extern fonts_utf8_t mcu_fonts_utf8_songti_16X16;
extern fonts_utf8_t mcu_fonts_utf8_songti_24X24;


extern fonts_utf8_t fonts_user_utf8_16X16;//自定义字体(随时修改)

#endif
