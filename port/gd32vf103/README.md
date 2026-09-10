# milliForth for GD32VF103 (RISC-V)

A port of [fuzzballcat/milliForth](https://github.com/fuzzballcat/milliForth)
(a 336-byte minimal FORTH) to the GigaDevice GD32VF103 Cortex-RISC-V
microcontroller.  The inner interpreter and the primitive words are written
in RISC-V assembly; the outer interpreter (tokenizer, dictionary search,
`: ;` compilation) is C.

## What it provides

The same core vocabulary as milliForth (see `README.md` of the upstream repo):

```
@  !  sp@  rp@  0#  +  nand  exit  key  emit  s@  :  ;
```

plus a literal (`LIT`) so numbers can be compiled/interpreted.  With `:`
definitions and these words you can write Turing-complete FORTH programs
(the upstream `bf.FORTH` is a good test).

## Layout

```
port/gd32vf103/
  main.c                    banner + call into the interpreter
  Makefile                  MinGW/Windows-friendly GNU make
  forth/
    forth.S                 NEXT, DOCOL, EXIT and all primitives (RISC-V asm)
    forth_boot.S            forth_run() bootstrap (machine stacks)
    forth.c                 outer interpreter (parse, find, compile)
    forth.h                 schema + layout constants
  bsp/
    usart_console.c(.h)     USART0 115200 8N1 console driver (PA9/PA10)
  firmware/                 GD32VF103 firmware library (copied) + RISCV env
```

Memory map (inside the 0x20000000 20k RAM):

| Area       | Address      | Purpose                         |
|------------|--------------|---------------------------------|
| TIB        | 0x20000000   | terminal input buffer           |
| STATE      | 0x20001000   | state struct (state/in/latest/here) |
| RSP/PSP    | 0x20000e00-0xff0 | machine return & data stacks |
| dictionary | from ~0x20001800 | headers + colon bodies grows up |
| LIT scratch| 0x20001e00   | transient interpret-literal body |

## Threaded-code model

- `NEXT`: `a0 = *(t3); t3 += 4; jalr a0`
- A primitive token is the address of an assembly routine in `forth.S`.
- Calling a colon word emits two cells: `[DOCOL][bodyAddr]`.  `DOCOL`
  reads the body address from the following thread cell, pushes the return
  address onto the return stack (`t4`), and re-enters `NEXT`.
- `exit` pops the return stack.
- Literal in a body: `[LIT][value]`.

Registers used by the vector machine (see `forth.h`):
`t3`=IP, `t4`=return stack, `t5`=data stack.

## Building

You need a RISC-V toolchain (this project targets `riscv-none-embed-gcc` or
`riscv64-unknown-elf-gcc`, `-march=rv32imac -mabi=ilp32`).

1. Set `RISCV_DIR` (and optionally `RISCV_PREFIX`) in the Makefile to the
   root of your toolchain.
2. Build:
   ```
   make
   ```
   This produces `milliForth-gd32vf103.bin` (and `.elf`).

`make clean` removes the build products.

## Flashing / running

Write `milliForth-gd32vf103.bin` at 0x08000000 with the XDS110/OpenOCD or
the vendor flasher of your choice.  Then open a serial terminal on USART0
(PA9 = TX, PA10 = RX) at 115200 8N1.  You can type numbers and words; e.g.

```
5 3 +
42 emit
: sq dup * ;
```

Note: numbers are unsigned-decimal; this minimal kernel deliberately mirrors
milliForth's comparably terse behaviour (unknown words print an extra blank
line and the rest of the line is abandoned).