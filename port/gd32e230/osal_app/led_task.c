/*
 * @file led_task.c
 * @brief LED 闪烁任务: 使用 OSAL 重复定时器周期性翻转板载 LED,
 *        演示 osal_start_reload_timer 定时器 + 板级 GPIO 输出。
 * @date  2026-09-04
 */

#include <stdio.h>
#include <stdlib.h>

#include "led_task.h"
#include "board.h"

uint8 led_task_id;          /* 记录 LED 任务的任务ID */

void led_task_init(uint8 task_id)
{
    led_task_id = task_id;

    /* 开启一个重复定时器: 每 LED_BLINK_TIMEOUT 个 tick 触发一次翻转事件 */
    osal_start_reload_timer(led_task_id, LED_BLINK_EVENT, LED_BLINK_TIMEOUT);
}

uint16 led_task_event_process(uint8 task_id, uint16 task_event)
{
    if (task_event & SYS_EVENT_MSG)             /* 系统消息事件(本任务未使用) */
    {
        osal_sys_msg_t *msg_pkt;
        msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);

        while (msg_pkt)
        {
            switch (msg_pkt->hdr.event)
            {
                default:
                    break;
            }
            osal_msg_deallocate((uint8 *)msg_pkt);
            msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);
        }
        return (task_event ^ SYS_EVENT_MSG);
    }

    if (task_event & LED_BLINK_EVENT)           /* LED 翻转事件 */
    {
        board_led_toggle();
        printf("LED blink, system clock : %lu ms\n",
               (unsigned long)osal_GetSystemClock());
        return task_event ^ LED_BLINK_EVENT;
    }

    return 0;
}