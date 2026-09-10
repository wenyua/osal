/*
 * usart_console.h - terminal (console) driver for the milliForth port.
 */
#ifndef USART_CONSOLE_H
#define USART_CONSOLE_H

void board_uart_init(void);
int  board_key(void);
void board_emit(int ch);

/* onboard active-low LED on PC13 */
void board_led_init(void);
void board_led_set(int on);
void board_led_set_nonzero(uint32_t on);
uint32_t board_led_octl_addr(void);
void forth_ms_c(uint32_t ms);
uint32_t forth_clock(void);
void forth_dec_c(uint32_t n);
void forth_bench_c(void);

#endif /* USART_CONSOLE_H */