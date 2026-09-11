/**
 * @file ch32v003_hw.h
 * @brief CH32V003 最小寄存器定义(不依赖 WCH 官方库)
 *        地址布局与 STM32F1 类似, 仅包含本例程用到的部分;
 *        正式项目建议替换为 WCH EVT 包中的 ch32v003.h。
 * @date  2026-09-11
 */
#ifndef CH32V003_HW_H
#define CH32V003_HW_H

#include <stdint.h>

#define REG32(addr)             (*(volatile uint32_t *)(addr))

/* ---- RCC ---- */
#define RCC_BASE                0x40021000UL
#define RCC_APB2PCENR           REG32(RCC_BASE + 0x18UL)
#define RCC_AFIOEN              (1UL << 0)
#define RCC_IOPCEN              (1UL << 4)      /* GPIOC 时钟 */
#define RCC_IOPDEN              (1UL << 5)      /* GPIOD 时钟 */
#define RCC_USART1EN            (1UL << 14)

/* ---- GPIO (F1 风格: CFGRL/CNFGR, OUTDR) ---- */
#define GPIOC_BASE              0x40011000UL
#define GPIOD_BASE              0x40011400UL
#define GPIO_CFGR(x)            REG32((x) + 0x00UL)     /* 模式/方向配置 */
#define GPIO_OUTDR(x)           REG32((x) + 0x0CUL)     /* 输出数据      */
#define GPIO_BSR(x)             REG32((x) + 0x10UL)     /* 置位          */
#define GPIO_BCR(x)             REG32((x) + 0x14UL)     /* 复位          */

/* ---- USART1 ---- */
#define USART1_BASE             0x40013800UL
#define USART_DATAR             REG32(USART1_BASE + 0x04UL)
#define USART_BRR               REG32(USART1_BASE + 0x08UL)
#define USART_CTLR1             REG32(USART1_BASE + 0x0CUL)
#define USART_STATR             REG32(USART1_BASE + 0x00UL)
#define USART_UE                (1UL << 13)
#define USART_TE                (1UL << 3)
#define USART_TC                (1UL << 6)
#define USART_TXE               (1UL << 7)

#endif /* CH32V003_HW_H */
