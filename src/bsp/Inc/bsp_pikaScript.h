#ifndef __BSP_PIKASCRIPT_H__
#define __BSP_PIKASCRIPT_H__

#include "CH58x_common.h"

/* ============================ 配置宏 ============================ */
/* 置1启用 PikaPython 脚本功能，置0完全关闭 */
#define PIKASCRIPT_ENABLE   1

#if PIKASCRIPT_ENABLE

#include "pikaScript.h"

/* 初始化脚本系统，并设置串口输出 */
void BSP_PikaScript_Init(void);

/* 执行一段 Python 字符串代码 */
void BSP_PikaScript_RunString(const char *script);

#endif /* PIKASCRIPT_ENABLE */

#endif /* __BSP_PIKASCRIPT_H__ */