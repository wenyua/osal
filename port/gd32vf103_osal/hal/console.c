/*
 * @file console.c
 * @brief USART0 控制台初始化: 配合固件库 stubs/write.c 的 _put_char
 *        实现 printf 输出 (PA9-TX / PA10-RX, 115200-8-N-1)
 * @date  2026-09-11
 */
#include "gd32vf103.h"

/*
 * @brief 初始化 USART0 为 printf 控制台
 * @param baud 波特率(如 115200)
 */
void board_uart_init(uint32_t baud)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    /* PA9: TX 复用推挽, PA10: RX 浮空输入 */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    usart_deinit(USART0);
    usart_baudrate_set(USART0, baud);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}
