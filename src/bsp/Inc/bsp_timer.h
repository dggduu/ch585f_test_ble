#ifndef __BSP_TIMER_H__
#define __BSP_TIMER_H__

#include "CH58x_common.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 软件定时器最大数量 */
#define BSP_TIMER_MAX_NUM  8

/* 定时器回调函数类型 */
typedef void (*bsp_timer_cb_t)(void *arg);

/* 软件定时器句柄 */
typedef int8_t bsp_timer_id_t;

/* 初始化定时器模块（需在系统时钟配置后调用） */
void BSP_Timer_Init(void);

/* 获取系统运行毫秒数（1ms 分辨率） */
uint32_t BSP_Timer_GetMillis(void);

/* 创建一个软件定时器
 * period_ms  : 定时周期（毫秒）
 * repeat     : true = 周期性，false = 单次
 * cb         : 回调函数（可为 NULL）
 * arg        : 回调参数
 * 返回值：定时器 ID（0 ~ BSP_TIMER_MAX_NUM-1），失败返回 -1
 */
bsp_timer_id_t BSP_Timer_Create(uint32_t period_ms, bool repeat, bsp_timer_cb_t cb, void *arg);

/* 启动定时器（如果已启动则重新计时） */
bool BSP_Timer_Start(bsp_timer_id_t id);

/* 停止定时器 */
bool BSP_Timer_Stop(bsp_timer_id_t id);

/* 删除定时器（释放资源） */
bool BSP_Timer_Delete(bsp_timer_id_t id);

/* 主循环调用：检查并处理到期定时器 */
void BSP_Timer_Tick(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TIMER_H__ */