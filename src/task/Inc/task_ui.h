// app_ui_task.h
#ifndef APP_UI_TASK_H
#define APP_UI_TASK_H

#include "CH58xBLE_LIB.h"

#define UI_UPDATE_EVENT   0x0001   // 定时刷新事件
#define UI_KEY_EVENT      0x0002   // 按键触发的刷新事件（可选）

extern tmosTaskID uiTaskID;

void UI_Task_Init(void);
uint16_t UI_ProcessEvent(tmosTaskID task_id, tmosEvents events);

#endif