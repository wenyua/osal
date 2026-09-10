/*
 * forth.h - minimal milliForth-style FORTH for GD32VF103 (RISC-V).
 *
 * Behavior mirrors fuzzballcat/milliForth sector.asm.  The core vocabulary:
 *
 *     @  !  sp@  rp@  0#  +  nand  exit  key  emit  s@  :  ;
 *
 * Threaded-code model (mapping faithfully onto RISC-V):
 *
 *   - A "code word" is the address of an assembly routine in forth.S.  Every
 *     primitive ends with `j forth_NEXT`.
 *
 *   - A colon definition has a body: a run of 32-bit cells.  Each cell is
 *     either:
 *         (a) the address of a primitive, or
 *         (b) a 2-cell DOCOL call: [DOCOL][calleeBodyAddr].
 *
 *   - DOCOL (assembly): it expects the *next* cell after the one that just
 *     jumped to it to hold the callee's first-body-cell address.  It pushes
 *     the call-site IP onto the return stack and sets IP there.
 *
 *   - EXIT pops a return frame.
 *
 * Dictionary header (written at HERE, laid out at LATEST):
 *     +0  link      uint32   ptr to previous header (0 = end)
 *     +4  lenflags  uint8    (len | FLAG_IMM)
 *     +5  name      ASCII    len bytes
 *     (padding to 4 alignment)
 *     +N  entry     uint32   = DOCOL for a colon word  (code field)
 *             ...    body cells follow inline (for colon words)
 *
 *       For a primitive the code field itself is not stored; the primitive is
 *       referenced by its asm label.  The entry field for a colon word is the
 *       assembly DOCOL label, and the first body cell is the first thing the
 *       outer loop appends.
 *
 * State struct at STATE (cells, all uint32):
 *     +0  state   0 = interpret, 1 = compile
 *     +4  in      pointer (absolute address) into TIB cursor
 *     +8  latest  addr of most recent header
 *     +12 here    addr of next free dictionary cell
 *
 * Memory:
 *     TIB      0x20000000  terminal input buffer
 *     STATE    0x20001000  state struct
 *     DICT     follows HERE (firmware static dictionary then user HERE)
 */
#ifndef FORTH_H
#define FORTH_H

#include <stdint.h>

#define TIB      0x20000000u
#define STATE    0x20001000u

/* Fixed zone for transient interpretation literals / machine stacks that
 * live between dictionary fill and RAM top.  Chosen well away from the
 * dictionary (which grows up from HERE) and the stack area. */
#define DICT_BASE   0x20001c00u
#define LIT_SCRATCH 0x20001e00u
#define PSP_BASE    0x20000e00u
#define RSP_BASE    0x20000ff0u

#define S_STATE  0u
#define S_CIN    4u
#define S_LATEST 8u
#define S_HERE   12u

#define FLAG_IMM 0x80u
#define LEN_MASK 0x1fu

/* Entry point memorized in forth.S for a fresh, top-level thread. */
void forth_run(uint32_t body_entry);

/* C implementation of the primitive words key/emit (referenced by asm). */
uint32_t forth_key_c(void);
void     forth_emit_c(uint32_t ch);

#endif /* FORTH_H */