/*
 * @file key_task.c
 * @brief 按键扫描任务: 周期定时器轮询按键, 软件消抖后区分短按/长按,
 *        以事件位通知本任务处理。演示多事件位 + 定时器轮询框架。
 * @date  2026-09-09
 */

#include <stdio.h>

#include "key_task.h"
#include "board.h"

uint8 key_task_id;              /* 记录按键任务的任务ID */

/* 内部扫描状态 */
static uint8  key_stable = 0;   /* 稳定状态: 1=按下 0=释放 */
static uint8  key_last   = 0;   /* 上次扫描电平(用于边沿判断) */
static uint8  key_debounce_cnt = 0;
static uint16 key_hold_ticks   = 0;   /* 按下持续 tick 计数 */
static uint8  key_long_fired   = 0;   /* 本次按下是否已触发过长按 */
static uint8  key_flash_cnt    = 0;   /* LED 快闪剩余翻转次数 */

void key_task_init(uint8 task_id)
{
    key_task_id = task_id;

    key_stable = 0;
    key_last = 0;
    key_debounce_cnt = 0;
    key_hold_ticks = 0;
    key_long_fired = 0;

    /* 周期定时器: 每 KEY_SCAN_TIMEOUT tick 触发一次扫描事件 */
    osal_start_reload_timer(key_task_id, KEY_SCAN_EVENT, KEY_SCAN_TIMEOUT);
}

/* 周期扫描: 电平->消抖->状态机, 检出短按/长按后转为自己事件 */
static void key_scan(void)
{
    uint8 now = (uint8)board_key_pressed();

    if (now != key_last)
    {
        /* 电平变化, 重新开始消抖计数 */
        key_debounce_cnt = 0;
        key_last = now;
    }
    else
    {
        if (key_debounce_cnt < KEY_DEBOUNCE_CNT)
        {
            key_debounce_cnt++;
        }
        else if (key_stable != now)
        {
            /* 消抖确认后更新稳定状态 */
            key_stable = now;

            if (key_stable)
            {
                /* 按下: 复位长按计时 */
                key_hold_ticks = 0;
                key_long_fired = 0;
            }
            else
            {
                /* 释放: 若未触发过长按则判定为短按 */
                if (!key_long_fired)
                {
                    osal_set_event(key_task_id, KEY_SHORT_EVENT);
                }
            }
        }
        else if (key_stable)
        {
            /* 按下保持中: 计时, 到达阈值触发一次长按事件 */
            if (key_hold_ticks < KEY_LONGPRESS_TICKS)
            {
                key_hold_ticks += KEY_SCAN_TIMEOUT;
                if (key_hold_ticks >= KEY_LONGPRESS_TICKS && !key_long_fired)
                {
                    key_long_fired = 1;
                    osal_set_event(key_task_id, KEY_LONG_EVENT);
                }
            }
        }
    }
}

uint16 key_task_event_process(uint8 task_id, uint16 task_event)
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

    if (task_event & KEY_SCAN_EVENT)            /* 周期扫描 */
    {
        key_scan();
        return task_event ^ KEY_SCAN_EVENT;
    }

    if (task_event & KEY_SHORT_EVENT)           /* 短按: 翻转 LED */
    {
        board_led_toggle();
        printf("Key short press, toggle LED\n");
        return task_event ^ KEY_SHORT_EVENT;
    }

    if (task_event & KEY_LONG_EVENT)            /* 长按: 启动 LED 快闪(定时器驱动) */
    {
        printf("Key long press!\n");

        /* 不用阻塞延时: 启动周期定时器, 每 KEY_FLASH_TICKS 触发一次翻转,
         * 共翻转 KEY_FLASH_TIMES*2 次后由 KEY_FLASH_EVENT 自动停表 */
        key_flash_cnt = KEY_FLASH_TIMES * 2U;
        osal_start_reload_timer(key_task_id, KEY_FLASH_EVENT, KEY_FLASH_TICKS);

        return task_event ^ KEY_LONG_EVENT;
    }

    if (task_event & KEY_FLASH_EVENT)           /* LED 快闪节拍 */
    {
        board_led_toggle();

        key_flash_cnt--;
        if (key_flash_cnt == 0)
        {
            /* 闪完, 停止定时器并确保 LED 回到灭 */
            osal_stop_timerEx(key_task_id, KEY_FLASH_EVENT);
            board_led_set(0);
        }

        return task_event ^ KEY_FLASH_EVENT;
    }

    return 0;
}