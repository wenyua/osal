/*
 * @file task_event.h
 * @brief OSAL 任务事件定义与接口声明 (CH32V003 精简移植)
 * @date  2026-09-11
 */
#ifndef APPLICATION_H
#define APPLICATION_H

#include "osal.h"
#include "osal_timer.h"
#include "osal_event.h"
#include "osal_memory.h"
#include "osal_msg.h"

typedef struct
{
    osal_event_hdr_t  hdr;          /* 操作系统事件结构 */
    unsigned char *data;            /* 命令帧操作数 */
} general_msg_data_t;               /* 自定义通用消息格式 */

/* 任务ID声明 */
extern uint8 led_task_id;
extern uint8 print_task_id;

/* 任务初始化函数声明 */
void led_task_init(uint8 task_id);
void print_task_init(uint8 task_id);

/* 任务事件处理函数声明 */
uint16 led_task_event_process(uint8 task_id, uint16 task_event);
uint16 print_task_event_process(uint8 task_id, uint16 task_event);

/* 板级接口 */
void board_led_init(void);
void board_led_toggle(void);
void console_init(void);
void console_putc(char c);
void console_puts(const char *s);
void console_putint(int v);

/* 任务事件定义 */
#define SYS_EVENT_MSG       0x8000   /* 系统消息事件 */
#define LED_BLINK_EVENT     0x0001   /* LED 翻转事件 */
#define PRINT_STR_EVENT     0x0002   /* 打印事件 */

void osal_main(void);

#endif /* APPLICATION_H */
