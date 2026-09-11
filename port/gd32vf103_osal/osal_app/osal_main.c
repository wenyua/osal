/*
 * @file osal_main.c
 * @brief OSAL 操作系统运行主函数, 添加任务在此文件
 * @date  2026-09-11
 */

#include "task_event.h"

void osal_main(void)
{
    /* 禁止中断 */
    HAL_DISABLE_INTERRUPTS();

    /* osal 操作系统初始化 */
    osal_init_system();

    /* 添加任务(数值越大优先级越高) */
    osal_add_Task(led_task_init, led_task_event_process, 1);
    osal_add_Task(print_task_init, print_task_event_process, 2);
    osal_add_Task(statistics_task_init, statistics_task_event_process, 3);

    /* 添加的任务统一初始化 */
    osal_Task_init();

    osal_mem_kick();

    /* 允许中断(机器定时器心跳自此开始工作) */
    HAL_ENABLE_INTERRUPTS();

    /* 启动 osal 系统, 不再返回 */
    osal_start_system();
}
