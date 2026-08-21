#include "bsp_pikaScript.h"

#if PIKASCRIPT_ENABLE

#include "bsp_uart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* PikaPython 主对象 */
static PikaObj *g_pikaMainObj = NULL;

/* ================== 串口重定向实现 ================== */
void pika_platform_printf(char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    BSP_UART_SendString((uint8_t *)buf, strlen(buf));
}

/* ================== 初始化 ================== */
void BSP_PikaScript_Init(void)
{
    g_pikaMainObj = pikaScriptInit();
    if (g_pikaMainObj == NULL) {
        return;
    }
    //pika_platform_printf("PikaPython ready!\r\n");
}

/* ================== 执行字符串脚本 ================== */
void BSP_PikaScript_RunString(const char *script)
{
    if (g_pikaMainObj == NULL || script == NULL) {
        return;
    }
    obj_run(g_pikaMainObj, (char *)script);
}

#endif /* PIKASCRIPT_ENABLE */