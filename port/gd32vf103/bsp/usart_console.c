/*
 * usart_console.c - terminal I/O driver for the milliForth port on GD32VF103.
 *
 * Uses USART0 (PA9 = TX, PA10 = RX) at 115200 8N1, polled.  Exposes the
 * two endpoints the Forth kernel needs:
 *     int  board_key(void);   blocking read of one byte
 *     void board_emit(int c); write one byte
 */
#include "gd32vf103.h"
#include "gd32vf103_usart.h"
#include "gd32vf103_gpio.h"
#include "gd32vf103_rcu.h"
#include <stdint.h>

void board_uart_init(void)
{
    /* Enable clocks */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_USART0);

    /* USART0 TX = PA9 (AF push-pull), RX = PA10 (floating input). */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}

int board_key(void)
{
    /* Wait for a received byte. */
    while (usart_flag_get(USART0, USART_FLAG_RBNE) == RESET) {
    }
    return (int)usart_data_receive(USART0);
}

void board_emit(int c)
{
    /* Wait for TBE then transmit. */
    while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET) {
    }
    usart_data_transmit(USART0, (uint16_t)(c & 0xff));
}

/* ---------------------------------------------------------------------
 * Onboard LED driver.
 * GD32VF boards commonly put the "debug" LED on PC13, active-low.
 * Configure it as push-pull output here so the FORTH script never has to
 * poke the RCU clock or control registers (which would risk clobbering the
 * console), only the small OCTL bit.
 * ------------------------------------------------------------------- */
void board_led_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);                 /* GPIOC clock   */
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13);
    /* drive high now -> LED off (active-low) */
    gpio_bit_set(GPIOC, GPIO_PIN_13);
}

void board_led_set(int on)
{
    if (on) gpio_bit_reset(GPIOC, GPIO_PIN_13);   /* low => LED on  */
    else    gpio_bit_set(GPIOC, GPIO_PIN_13);     /* high => LED off */
}

/* The absolute address the FORTH script can store to directly:
 *   GPIOC_OCTL  0x4001080C , bit13 = LED (1=off, 0=on). */
uint32_t board_led_octl_addr(void){ return (uint32_t)(GPIOC + 0x0CU); }

/* C helper spoken by the `led` FORTH primitive: nonzero arg => LED on. */
void board_led_set_nonzero(uint32_t on){ board_led_set(on != 0); }

/* Busy-wait using the RISC-V mcycle CSR.  mcycle increments every cycle. */
static inline uint64_t read_mcycle(void)
{
    uint32_t lo, hi;
    __asm__ volatile("csrr %0, mcycle"  : "=r"(lo));
    __asm__ volatile("csrr %0, mcycleh" : "=r"(hi));
    return ((uint64_t)hi << 32) | lo;
}

/* Approximate milliseconds from an assumed 48 MHz core clock. */
void forth_ms_c(uint32_t ms)
{
    uint64_t start = read_mcycle();
    uint64_t target = start + ((uint64_t)ms * 48000u);
    while (read_mcycle() < target) {
    }
}

/* Read the raw RISC-V mcycle (low 32 bits); wraps at ~89s @48MHz. */
uint32_t forth_clock(void)
{
    return (uint32_t)read_mcycle();
}

/* Print a 32-bit unsigned value in decimal followed by a space, using
 * reverse digit collection so the empire is trivial and dependency-free. */
static void print_dec(uint32_t v)
{
    char buf[12];
    int  i = 0;
    do {
        buf[i++] = (char)('0' + (v % 10));
        v /= 10;
    } while (v);
    while (i) board_emit(buf[--i]);
    board_emit(' ');
}

/* FORTH `dec` word:  ( n -- )  prints the decimal value of n. */
void forth_dec_c(uint32_t n)
{
    print_dec(n);
}

/* FORTH `bench` word: ( -- )  runs the hot inner loop for a fixed number of
 * iterations and prints the mcycle delta.  Done in C so we can have a real
 * loop and subtraction (which the toy Forth vocabulary lacks).  Reports:
 *     <mcycle delta> <delta / N> cycles-per-iteration
 */
void forth_bench_c(void)
{
    uint32_t start = (uint32_t)read_mcycle();
    volatile uint32_t acc = 0;
    const uint32_t N = 100000u;
    for (uint32_t i = 0; i < N; i++) {
        acc += 1u;            /* the operation under test            */
    }
    uint32_t delta = (uint32_t)read_mcycle() - start;
    uint32_t per = delta / N;
    print_dec(delta);
    board_emit('/');
    print_dec(N);
    board_emit('=');
    print_dec(per);
    board_emit(' ');
    board_emit('\r');
    board_emit('\n');
    (void)acc;
}