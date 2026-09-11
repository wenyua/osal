/*
 * @file main.c
 * @brief OSAL 运行例程入口 (CH32V003)
 * @date  2026-09-11
 */

#include "task_event.h"

int main(void)
{
    /* 时钟: 复位默认 HSI 24MHz, 如需 48MHz PLL 请自行初始化
       并同步修改 hal/timer.c 与 console.c 的 CORE_CLOCK */

    /* 板载 LED 初始化(PD6) */
    board_led_init();

    /* 启动 OSAL(串口在 print_task_init 中初始化) */
    osal_main();

    while(1)
    {
    }
}
