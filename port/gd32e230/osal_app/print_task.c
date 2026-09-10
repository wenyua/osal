/*
 * @file print_task.c
 * @brief 打印任务 (GD32E230 移植)
 * @date  2026-09-04
 */

#include <stdio.h>
#include <stdlib.h>

#include "task_event.h"

uint8 print_task_id;   /* 记录打印任务的任务ID */

void print_task_init(uint8 task_id)
{
    print_task_id = task_id;

    /* 开启循环定时器, 每秒向打印任务发送 PRINTF_STR 事件 */
    osal_start_reload_timer(print_task_id, PRINTF_STR, 1000 / TICK_PERIOD_MS);
}

uint16 print_task_event_process(uint8 task_id, uint16 task_event)
{
    if (task_event & SYS_EVENT_MSG)     /* 系统消息事件 */
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

    if (task_event & PRINTF_STR)
    {
        static int print_count = 0;
        printf("Print task printing, total memory : %d byte, used memory : %d byte !\n",
               MAXMEMHEAP, osal_heap_mem_used());

        print_count++;
        if (print_count % 5 == 0 && print_count != 0)
        {
            general_msg_data_t *msg;
            msg = (general_msg_data_t *)osal_msg_allocate(sizeof(general_msg_data_t) + sizeof(int));
            if (msg != NULL)
            {
                msg->data = (unsigned char *)(msg + 1);

                msg->hdr.event = PRINTF_STATISTICS;
                msg->hdr.status = 0;
                *((int *)msg->data) = print_count;

                osal_msg_send(statistics_task_id, (uint8 *)msg);
            }
        }

        return task_event ^ PRINTF_STR;
    }

    return 0;
}