/*
 * @file led_task.h
 * @brief LED 闪烁任务接口 (定时器 + GPIO 演示)
 * @date  2026-09-04
 */
#ifndef LED_TASK_H
#define LED_TASK_H

#include "task_event.h"

/* LED 闪烁周期(每边/半周期), 单位: 次数= ms/TICK_PERIOD_MS */
#define LED_BLINK_TIMEOUT       (500U / TICK_PERIOD_MS)   /* 500ms 翻转一次 */

/* 触发 LED 闪烁事件 */
#define LED_BLINK_EVENT         0x0002

extern uint8 led_task_id;

void led_task_init(uint8 task_id);
uint16 led_task_event_process(uint8 task_id, uint16 task_event);

#endif /* LED_TASK_H */