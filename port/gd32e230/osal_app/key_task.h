/*
 * @file key_task.h
 * @brief 按键扫描任务接口 (轮询消抖 + 短按/长按事件)
 * @date  2026-09-09
 */
#ifndef KEY_TASK_H
#define KEY_TASK_H

#include "task_event.h"

/* 按键扫描周期(轮询) */
#define KEY_SCAN_TIMEOUT        (10U / TICK_PERIOD_MS)    /* 10ms 轮询一次 */

/* 消抖确认次数: 连续 N 次扫描到同一状态才认为有效 */
#define KEY_DEBOUNCE_CNT        2U                        /* 20ms 消抖 */

/* 长按判定: 按下保持超过该 tick 数为长按 */
#define KEY_LONGPRESS_TICKS     (1000U / TICK_PERIOD_MS)  /* 1s 长按 */

/* 按键任务事件 */
#define KEY_SCAN_EVENT          0x0004    /* 周期扫描事件 */
#define KEY_SHORT_EVENT         0x0008    /* 短按事件 */
#define KEY_LONG_EVENT          0x0010    /* 长按事件 */
#define KEY_FLASH_EVENT         0x0020    /* LED 快闪事件(定时器驱动) */

/* 长按提示: LED 快闪次数与节拍 */
#define KEY_FLASH_TIMES         3U                              /* 闪 3 下 */
#define KEY_FLASH_TICKS         (100U / TICK_PERIOD_MS)         /* 100ms 翻转一次 */

extern uint8 key_task_id;

void key_task_init(uint8 task_id);
uint16 key_task_event_process(uint8 task_id, uint16 task_event);

#endif /* KEY_TASK_H */