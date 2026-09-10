/*
 * @file gd32e23x_board.h
 * @brief 板级引脚配置 (GD32E230, 标准外设库)
 * @date  2026-09-04
 */
#ifndef GD32E23X_BOARD_H
#define GD32E23X_BOARD_H

#include "gd32e23x.h"

/* ==== 板载 LED ==== */
/* 默认 PC13, 低电平点亮。请按实际原理图修改。 */
#define BOARD_LED_PORT          GPIOC
#define BOARD_LED_RCU           RCU_GPIOC
#define BOARD_LED_PIN           GPIO_PIN_13

/* ==== 板载按键 ==== */
/* 默认 PA0(KEY/WAKEUP), 按下接地为低电平。请按实际原理图修改。 */
#define BOARD_KEY_PORT          GPIOA
#define BOARD_KEY_RCU           RCU_GPIOA
#define BOARD_KEY_PIN           GPIO_PIN_0

#endif /* GD32E23X_BOARD_H */