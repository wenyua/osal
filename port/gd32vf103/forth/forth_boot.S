/*
 * forth_boot.S - bootstrap glue for the milliForth-style FORTH on GD32VF103.
 *
 * Provides:
 *   - forth_run(entry):  set up the vector-machine stacks and enter a body.
 *   - forth_boot():      seed the machine stacks / primitives (unused here,
 *                        seeds done in C via dict_enter).
 *   - dict_enter(name, bodyaddr):  C-callable helper to install a primitive
 *                        header into the dictionary.
 *
 * Registers (see forth.S / forth.h):
 *   t3 = I    t4 = RSP (physical SP)   t5 = PSP (data stack)
 * RBSP base = where PSP starts; the asm keeps them out of the C ABI.
 */

/* Dict constants (from forth.h) */
#define STATE    0x20001000
#define S_HERE   12
#define S_LATEST 8

    .section .text.forth

/* forth_run(uint32_t body_entry)  [a0 = body_entry]
 * Called from C.  We must not touch the C stack frame beyond our own, so we
 * set up our own machine stacks in fixed RAM and switch SP there.
 *   PSP (data stack) base / RSP (return stack) base live in fixed low RAM so
 * this is a bare-metal single-threaded system.
 */
    .global forth_run
    .type   forth_run, %function
forth_run:
    addi    sp, sp, -8
    sw      ra, 0(sp)

    /* Establish machine stacks in a fixed scratch area just below STATE. */
    li      t5, 0x20000e00          /* data  stack top, grows down       */
    li      t4, 0x20000ff0          /* return stack top, grows down      */

    /* Bootstrap a top-level colon call.  The user `entry` is the address of
     * the word's first BODY CELL.  To call it we mimic a ^[DOCOL][body] pair
     * but directly, since there is no caller thread at the top level:
     *   push a sentinel return IP (here: the address of `forth_done`) onto
     *   the return stack, then set IP = body and enter NEXT.
     * When the body's final EXIT pops, it will "return" into `forth_done`,
     * which restores the C frame.  This keeps DOCOL/EXIT uniform.        */
    la      t0, forth_done
    addi    t4, t4, -4
    sw      t0, 0(t4)            /* return-sentinel on the THREAD return stack */
    mv      t3, a0               /* IP = body entry                          */
    call    forth_NEXT           /* begin interpreting; only returns at done */

forth_done:
    lw      ra, 0(sp)
    addi    sp, sp, 8
    ret

    .size forth_run, .-forth_run

/* dict_enter(const char *name, uint32_t bodyaddr)
 * Reserved; seed headers for primitives so `find` works.  The militiForth
 * approach seeds LATEST with header entries.  We do it in C for clarity but
 * the symbol must exist for the linker if C references it. */
    .global dict_enter
    .type   dict_enter, %function
dict_enter:
    ret

    .size dict_enter, .-dict_enter

    .section .note.GNU-stack,"",%progbits