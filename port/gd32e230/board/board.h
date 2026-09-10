/*
 * @file board.h
 * @brief GD32E230 板级初始化接口 (基于 GD32 标准外设库)
 * @date  2026-09-04
 */
#ifndef BOARD_H
#define BOARD_H

#include "gd32e23x.h"

/*
 * 系统时钟由 CMSIS system_gd32e23x.c 的 SystemInit() 完成初始化,
 * 本文件无需手动配置。
 */

/* USART0 初始化, 用于 printf 输出 (PA9=TX) */
void board_usart0_init(uint32_t baudrate);

/* LED 初始化并配置输出模式 */
void board_led_init(void);

/* 设置 LED 状态: 1=亮 0=灭 */
void board_led_set(uint32_t on);

/* 翻转 LED 状态 */
void board_led_toggle(void);

/* 按键初始化(输入, 上拉, 按下为低电平) */
void board_key_init(void);

/* 读取按键当前电平: 返回 1=按下 0=释放 */
uint32_t board_key_pressed(void);

#endif /* BOARD_H */