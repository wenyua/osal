/*
 * @file statistics_task.c
 * @brief 统计任务 (GD32E230 移植)
 * @date  2026-09-04
 */

#include <stdio.h>
#include <stdlib.h>

#include "task_event.h"

uint8 statistics_task_id;   /* 记录统计任务的任务ID */

void statistics_task_init(uint8 task_id)
{
    statistics_task_id = task_id;
}

uint16 statistics_task_event_process(uint8 task_id, uint16 task_event)
{
    if (task_event & SYS_EVENT_MSG)     /* 系统消息事件 */
    {
        osal_sys_msg_t *msg_pkt;
        msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);

        while (msg_pkt)
        {
            switch (msg_pkt->hdr.event)
            {
                case PRINTF_STATISTICS:
                {
                    int count = *(int *)(((general_msg_data_t *)msg_pkt)->data);
                    printf("Statistics task receive print task printf count : %d\n", count);
                    break;
                }

                default:
                    break;
            }

            osal_msg_deallocate((uint8 *)msg_pkt);
            msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);
        }

        return (task_event ^ SYS_EVENT_MSG);
    }

    return 0;
}