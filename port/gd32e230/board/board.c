/*
 * @file board.c
 * @brief GD32E23x 板级实现, 基于 GD32 标准外设库 (gd32e23x_std_peripheral)
 * @date  2026-09-04
 */
#include "board.h"
#include "gd32e23x_board.h"

/*
 * 系统时钟由 CMSIS system_gd32e23x.c 的 SystemInit() 配置。
 * 默认 72MHz(HXTAL)或 IRC8M, 由 system_gd32e23x.c 中宏选择。
 * 此处不重复配置时钟, 保持简洁。
 */

/*
 * USART0 初始化 (PA9=TX): 8N1。
 */
void board_usart0_init(uint32_t baudrate)
{
    /* 使能 GPIOA / USART0 时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    /* PA9 复位 + 复用功能 AF7(USART0_TX) */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* USART0 复位 + 8N1 + 波特率 */
    usart_deinit(USART0);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_baudrate_set(USART0, baudrate);
    usart_receive_config(USART0, USART_RECEIVE_DISABLE);   /* 仅 TX */
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}

/*
 * LED 初始化: 配置 BOARD_LED_PORT / BOARD_LED_PIN 为推挽输出, 初始为灭。
 * 默认 PC13, 见 gd32e23x_board.h。
 */
void board_led_init(void)
{
    rcu_periph_clock_enable(BOARD_LED_RCU);

    gpio_mode_set(BOARD_LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BOARD_LED_PIN);
    gpio_output_options_set(BOARD_LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BOARD_LED_PIN);

    board_led_set(0);
}

/* 置 LED 状态(低电平点亮) */
void board_led_set(uint32_t on)
{
    if (on)
    {
        gpio_bit_reset(BOARD_LED_PORT, BOARD_LED_PIN);
    }
    else
    {
        gpio_bit_set(BOARD_LED_PORT, BOARD_LED_PIN);
    }
}

void board_led_toggle(void)
{
    gpio_bit_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
}

/*
 * 按键初始化: 输入模式, 上拉(按下接地为低电平)。
 * 引脚由 BOARD_KEY_PORT / BOARD_KEY_PIN 决定(默认 PA0), 见 gd32e23x_board.h。
 */
void board_key_init(void)
{
    rcu_periph_clock_enable(BOARD_KEY_RCU);

    gpio_mode_set(BOARD_KEY_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, BOARD_KEY_PIN);
}

/* 读取按键: 低电平=按下, 返回 1=按下 0=释放 */
uint32_t board_key_pressed(void)
{
    return (gpio_input_bit_get(BOARD_KEY_PORT, BOARD_KEY_PIN) == RESET) ? 1U : 0U;
}