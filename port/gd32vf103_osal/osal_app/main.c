/*
 * @file main.c
 * @brief OSAL 运行例程入口 (GD32VF103)
 * @date  2026-09-11
 */

#include "task_event.h"

void board_uart_init(uint32_t baud);

int main(void)
{
    /* 时钟(SystemInit)与 ECLIC 已由启动流程 _init() 完成 */

    /* USART0 初始化, 用于 printf 输出, 波特率 115200 */
    board_uart_init(115200);

    /* 板载 LED 初始化(PC13) */
    board_led_init();

    /* 启动 OSAL, 不再返回 */
    osal_main();

    while(1)
    {
    }
}
