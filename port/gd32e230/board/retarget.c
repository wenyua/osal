/*
 * @file retarget.c
 * @brief printf 重定向到 USART0 (基于 GD32 标准外设库)。
 *        newlib 的 printf/puts 最终调用 _write, 此处转发到固件库收发函数。
 * @date  2026-09-04
 */
#include <stdint.h>
#include <stdio.h>

#include "gd32e23x.h"

/* 发送单字节, 阻塞等待发送完成 */
static void usart_putchar(uint8_t ch)
{
    usart_data_transmit(USART0, ch);
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
}

/* 供 newlib printf/puts 调用 */
int _write(int fd, char *ptr, int len)
{
    (void)fd;
    int i;
    for (i = 0; i < len; i++)
    {
        if (ptr[i] == '\n')
        {
            usart_putchar('\r');
        }
        usart_putchar((uint8_t)ptr[i]);
    }
    return len;
}