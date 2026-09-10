/*
 * @file osal_main.c
 * @brief OSAL 操作系统运行主函数, 添加任务在此文件
 * @date  2026-09-04
 */

#include "task_event.h"
#include "led_task.h"
#include "key_task.h"

void osal_main(void)
{
    /* 禁止中断 */
    HAL_DISABLE_INTERRUPTS();

    /* osal 操作系统初始化 */
    osal_init_system();

    /* 添加任务 */
    osal_add_Task(key_task_init, key_task_event_process, 1);
    osal_add_Task(led_task_init, led_task_event_process, 2);
    osal_add_Task(print_task_init, print_task_event_process, 3);
    osal_add_Task(statistics_task_init, statistics_task_event_process, 4);

    /* 添加的任务统一初始化 */
    osal_Task_init();

    osal_mem_kick();

    /* 允许中断 */
    HAL_ENABLE_INTERRUPTS();

    /* 启动 osal 系统, 不再返回 */
    osal_start_system();
}