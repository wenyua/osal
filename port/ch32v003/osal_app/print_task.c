/*
 * @file print_task.c
 * @brief 打印任务 (CH32V003 精简版): 不使用 printf, 用轻量串口输出,
 *        每 2s 打印一次堆内存使用情况
 * @date  2026-09-11
 */
#include "task_event.h"

uint8 print_task_id;

void print_task_init(uint8 task_id)
{
    print_task_id = task_id;
    console_init();

    /* 每 2s 打印一次: 2000ms / 10ms tick */
    osal_start_reload_timer(print_task_id, PRINT_STR_EVENT, 2000 / TICK_PERIOD_MS);
}

uint16 print_task_event_process(uint8 task_id, uint16 task_event)
{
    if(task_event & SYS_EVENT_MSG)
    {
        osal_sys_msg_t *msg_pkt;
        msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);

        while(msg_pkt)
        {
            osal_msg_deallocate((uint8 *)msg_pkt);
            msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);
        }
        return (task_event ^ SYS_EVENT_MSG);
    }

    if(task_event & PRINT_STR_EVENT)
    {
        console_puts("heap used: ");
        console_putint((int)osal_heap_mem_used());
        console_puts(" / 512 byte, sysclk(ms): ");
        console_putint((int)osal_GetSystemClock());
        console_puts("\r\n");
        return task_event ^ PRINT_STR_EVENT;
    }

    return 0;
}
