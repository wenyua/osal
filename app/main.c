/**
 * @file main.c
 * @brief osal 运行例程入口（GD32E230 适配）
 * @version 0.1
 * @date 2026-09-02
 */

#include "task_event.h"

/**
 * @brief 程序入口
 * @note  在 GD32E230 上由启动文件调用 Reset_Handler -> main。
 *        若使用 printf 需要提前初始化串口 USART0，并重定向 fputc。
 */
int main(void)
{
    osal_main();
}
