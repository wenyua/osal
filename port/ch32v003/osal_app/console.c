/*
 * @file console.c
 * @brief 轻量串口控制台 (CH32V003, USART1 TX=PD5, 复位时钟 HSI 24MHz)
 *        无 printf 依赖, 仅 putc/puts/putint, 节省 Flash 与栈
 * @date  2026-09-11
 */
#include "ch32v003_hw.h"
#include "task_event.h"

#define CONSOLE_BAUD            115200UL
#define CORE_CLOCK              24000000UL      /* 与 hal/timer.c 保持一致 */

void console_init(void)
{
    RCC_APB2PCENR |= RCC_IOPDEN | RCC_USART1EN;

    /* PD5: USART1-TX 复用推挽 (MODE=11, CNF=10) */
    GPIO_CFGR(GPIOD_BASE) = (GPIO_CFGR(GPIOD_BASE) & ~(0xFUL << (5 * 4)))
                            | (0xBUL << (5 * 4));

    USART_BRR   = (CORE_CLOCK + CONSOLE_BAUD / 2) / CONSOLE_BAUD;
    USART_CTLR1 = 0;
    USART_CTLR1 = USART_UE | USART_TE;
}

void console_putc(char c)
{
    while(!(USART_STATR & USART_TXE))
    {
    }
    USART_DATAR = (uint8_t)c;
}

void console_puts(const char *s)
{
    while(*s)
    {
        console_putc(*s++);
    }
}

/* 轻量十进制输出(避免 printf) */
void console_putint(int v)
{
    char buf[12];
    int i = 0;
    unsigned int u;

    if(v < 0)
    {
        console_putc('-');
        u = (unsigned int)(-v);
    }
    else
    {
        u = (unsigned int)v;
    }

    do
    {
        buf[i++] = (char)('0' + u % 10);
        u /= 10;
    } while(u && i < (int)sizeof(buf));

    while(i--)
    {
        console_putc(buf[i]);
    }
}
