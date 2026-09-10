/*
 * @file main.c
 * @brief OSAL 运行例程入口 (GD32E230)
 * @date  2026-09-04
 */

#include "task_event.h"
#include "board.h"

int main(void)
{
    /* 系统时钟由 SystemInit()(启动文件调用)配置, 无需在此初始化 */

    /* USART0 初始化, 用于 printf 输出, 波特率 115200 */
    board_usart0_init(115200);

    /* 板载 LED 初始化 */
    board_led_init();

    /* 板载按键初始化 */
    board_key_init();

    /* 启动 OSAL */
    osal_main();

    /* 不应到达此处 */
    while (1)
    {
    }
}