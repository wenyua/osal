/*
 * forth.S - RISC-V inner interpreter ("NEXT") and primitive words for a
 * milliForth-style minimal FORTH on the GD32VF103 (RV32IM).
 *
 * Vector-machine registers (see forth.h):
 *   t3 (x28)  I    instruction pointer (points into a threaded body)
 *   t4 (x29)  RSP  return stack (RISC-V physical sp)
 *   t5 (x30)  PSP  data stack (grows down, FORTH convention)
 *
 * Threading scheme (final, consistent with the C outer loop):
 *
 *   A "compile cell" (thread cell) is a single 32-bit word that is either:
 *     (a) the address of a primitive (all primitives below), or
 *     (b) the address of a colon word's FIRST BODY CELL.
 *
 *   NEXT:  a0 = lw(I); I+=4; jalr a0.
 *
 *   For case (b) the callee is a *body*, not a routine.  We mark colon
 *   bodies by having their first word be DOCOL (the assembly label).  So a
 *   colon body cell-layout is:
 *
 *       +0   DOCOL            <-- the token a BODY ADDRESS points at
 *       +4   <first real cell>
 *       ...
 *
 *   NEXT jumps to the address of that first body cell, i.e. to a cell whose
 *   contents == DOCOL.  So we never jump to DOCOL directly; we jump to a
 *   *body start address*, which loads DOCOL from the cell.
 *
 *   That normalizes everything: NEXT always does lw(a0).  Execution of a
 *   body start: at that location the word is DOCOL.  We implement DOCOL's
 *   handler as an inline fragment that lives at every body start?  No.
 *
 *   Simpler canonical approach (chosen):  DO NOT store DOCOL in the body.
 *   A token that references a colon word -> the address of its header; the
 *   header (see forth.h) has, right after the name, a code field whose value
 *   is DOCOL.  NEXT on a header pointer must therefore jump to the code
 *   field *value* (DOCOL) again, doable but adds a fetch.
 *
 *   Cleanest:  thread cell directly holds the address of the entry code.
 *   For colon words the entry code = assembly DOCOL, and DOCOL needs the
 *   body pointer.  MilliForth stores body pointer *for the caller* in the
 *   header code field = body start.
 *
 * Decision (final, avoid the +4 two-step):
 *   - NEXT fetches token a0. a0 is ALWAYS an address of an assembly routine.
 *       * primitives : the routine is one above.
 *       * colon words: the routine is DOCOL, and DOCOL reads the body address
 *         from the *returning* location, i.e. the word AR2 cells in the caller:
 *         DOCOL assumes the calling body cell layout is [DOCOL][bodyAddr].
 *   So colon words are called by APPENDING TWOM cell(raw): [DOCOL][bodyAddr].
 *   NEXT doubles the IP jump only on DOCOL.
 *
 * That's exactly milliForth: `dw DOCOL, bodyAddr` two cells.  NEXT fetches
 * the token that is DOCOL, jumps to DOCOL, and DOCOL picks the NEXT cell
 * (which NEXT already advanced past) as the body addr.  Good — this is the
 * design; below implements exactly that with a clean frame.
 *
 * Frame format on RSP (each level = 2 cells: words 4 bytes each):
 *   RSP base .. push return-IP then later body branch value is transient.
 *   EXIT pops return-IP.
 */
#include "forth.h"

#define PUSH(x) addi t5, t5, -4 ; sw x, 0(t5)
#define POP(x)  lw x, 0(t5) ; addi t5, t5, 4

    .section .text.forth

/* NEXT --------------------------------------------------------------- */
    .global forth_NEXT
    .type   forth_NEXT, %function
forth_NEXT:
    lw      a0, 0(t3)
    addi    t3, t3, 4
    jalr    zero, 0(a0)

/* DOCOL ----------------------------------------------------------------
   Pairs with a 2-cell call site in the caller's thread: [DOCOL][bodyAddr].
   NEXT jumps here when it fetches the token DOCOL; NEXT already advanced I
   past the DOCOL token, so I points at the [bodyAddr] cell.
        body = lw(I); I += 4; push I(return); I = body; NEXT.          */
    .global DOCOL
    .type   DOCOL, %function
DOCOL:
    lw      a0, 0(t3)          /* bodyAddr          */
    addi    t3, t3, 4
    addi    t4, t4, -4
    sw      t3, 0(t4)          /* push return IP    */
    mv      t3, a0
    j       forth_NEXT

/* EXIT ----------------------------------------------------------------- */
    .global EXIT
    .type   EXIT, %function
EXIT:
    lw      t3, 0(t4)
    addi    t4, t4, 4
    j       forth_NEXT

/* @  ( addr -- value ) ----------------------------------------------------- */
    .global FETCH
    .type   FETCH, %function
FETCH:
    POP(a1)
    lw      a0, 0(a1)
    PUSH(a0)
    j       forth_NEXT

/* !  ( value addr -- ) ------------------------------------------------------- */
    .global STORE
    .type   STORE, %function
STORE:
    POP(a1)
    POP(a0)
    sw      a0, 0(a1)
    j       forth_NEXT

/* sp@ ( -- sp ) -------------------------------------------------------------- */
    .global SPFETCH
    .type   SPFETCH, %function
SPFETCH:
    PUSH(t5)
    j       forth_NEXT

/* rp@ ( -- rp ) -------------------------------------------------------------- */
    .global RPFETCH
    .type   RPFETCH, %function
RPFETCH:
    PUSH(t4)
    j       forth_NEXT

/* 0# ( value -- flag ) flag = value ? -1 : 0 --------------------------------- */
    .global ZEROEQ
    .type   ZEROEQ, %function
ZEROEQ:
    POP(a0)
    snez    a0, a0
    neg     a0, a0
    PUSH(a0)
    j       forth_NEXT

/* + ( a b -- a+b ) ------------------------------------------------------------ */
    .global PLUS
    .type   PLUS, %function
PLUS:
    POP(a1)
    POP(a0)
    add     a0, a0, a1
    PUSH(a0)
    j       forth_NEXT

/* nand ( a b -- aNANDb ) ------------------------------------------------------ */
    .global NAND
    .type   NAND, %function
NAND:
    POP(a1)
    POP(a0)
    and     a0, a0, a1
    not     a0, a0
    PUSH(a0)
    j       forth_NEXT

/* s@ ( -- stateaddr ) -------------------------------------------------------- */
    .global STATEVAR
    .type   STATEVAR, %function
STATEVAR:
    li      a0, STATE
    PUSH(a0)
    j       forth_NEXT

/* LIT ( -- n ) : push the literal in the following thread cell.
   A number in interpreted/compiled context expands to [LIT][value]. */
    .global LIT
    .type   LIT, %function
LIT:
    lw      a0, 0(t3)
    addi    t3, t3, 4
    PUSH(a0)
    j       forth_NEXT

/* key ( -- ch ) -------------------------------------------------------------- */
    .global KEY
    .type   KEY, %function
KEY:
    call    forth_key_c
    PUSH(a0)
    j       forth_NEXT

/* emit ( ch -- ) -------------------------------------------------------------- */
    .global EMIT
    .type   EMIT, %function
EMIT:
    POP(a0)
    call    forth_emit_c
    j       forth_NEXT

/* led ( n -- ) : n!=0 => turn LED on.  Thin wrapper over C board_led_set. */
    .global LED
    .type   LED, %function
LED:
    POP(a0)
    call    board_led_set_nonzero
    j       forth_NEXT

/* ms ( n -- ) : busy-wait ~n milliseconds using the RISC-V mcycle counter. */
    .global MS
    .type   MS, %function
MS:
    POP(a0)                     /* a0 = ms count to wait */
    call    forth_ms_c
    j       forth_NEXT

/* clock ( -- csr ) : push the low 32 bits of the mcycle counter. */
    .global CLOCK
    .type   CLOCK, %function
CLOCK:
    csrr    a0, mcycle
    PUSH(a0)
    j       forth_NEXT

/* dec ( n -- ) : print n as decimal + space. */
    .global DEC
    .type   DEC, %function
DEC:
    POP(a0)
    call    forth_dec_c
    j       forth_NEXT

/* bench ( -- ) : print mcycle delta over a fixed C hot loop. */
    .global BENCH
    .type   BENCH, %function
BENCH:
    call    forth_bench_c
    j       forth_NEXT

    .section .note.GNU-stack,"",%progbits