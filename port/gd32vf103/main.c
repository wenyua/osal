/*
 * main.c - milliForth-style minimal FORTH kernel for GD32VF103 (RISC-V).
 *
 * Wires the GD32VF103 firmware library's startup/runtime (init.c, start.S,
 * entry.S, handlers.c), initializes the USART0 console, seeds a minimal
 * FORTH dictionary, and enters the endless outer interpreter.
 *
 * Build with the GD32VF103 RISC-V toolchain, see Makefile.
 */
#include "gd32vf103.h"
#include "gd32vf103_usart.h"
#include "usart_console.h"
#include "forth.h"
#include <stdint.h>

/* Forth kernel entry points */
void forth_init(void);
void forth_outer(void);

int main(void)
{
    /* USART0 console @115200 8N1 on PA9/PA10 */
    board_uart_init();
    /* Onboard LED on PC13, active-low */
    board_led_init();

    /* Seed the FORTH dictionary + state struct */
    forth_init();

    /* Banner */
    board_emit('\r'); board_emit('\n');
    { const char *b = "milliForth-GD32VF103\r\n";
      while (*b) board_emit(*b++); }

    /* Run the interpreter forever */
    forth_outer();

    return 0;
}