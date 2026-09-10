/*
 * forth.c - milliForth-style minimal FORTH outer interpreter for GD32VF103.
 *
 * The inner interpreter (NEXT) and all primitive words are implemented in
 * RISC-V assembly in forth.S.  This file provides the outer interpreter:
 *
 *   - a line-oriented terminal driver over the board UART (key/emit),
 *   - a tokenizer over the terminal input buffer (TIB),
 *   - dictionary search and ":" compilation,
 *   - the compile-or-execute loop.
 *
 * Threading scheme (see forth.S / forth.h):
 *
 *   NEXT:           a0 = lw(I); I += 4; jalr a0
 *   colon call:     emitted as TWO thread cells  [DOCOL] [bodyAddr]
 *   literal:        emitted as TWO thread cells  [LIT]   [value]
 *
 * A primitive token is the address of its assembly routine.  A colon word's
 * entry is the address of its FIRST BODY CELL.  To CALL a word we emit
 * [DOCOL][bodyAddr].  DOCOL reads the body addr from the following thread
 * cell.
 *
 * Dictionary header (at HERE/LATEST):
 *     +0   link     uint32   previous header addr (0 = none)
 *     +4   lenflags uint8    (len | FLAG_IMM)
 *     +5.. name     len bytes
 *     (pad to 4)
 *
 * `find` returns the address of the first body cell.
 */
#include "forth.h"
#include <stdint.h>
#include <string.h>

/* ---- primitive asm entry points (from forth.S) ---- */
extern void FETCH(void);
extern void STORE(void);
extern void SPFETCH(void);
extern void RPFETCH(void);
extern void ZEROEQ(void);
extern void PLUS(void);
extern void NAND(void);
extern void EXIT(void);
extern void KEY(void);
extern void EMIT(void);
extern void STATEVAR(void);
extern void LIT(void);
extern void DOCOL(void);
extern void LED(void);
extern void MS(void);
extern void CLOCK(void);
extern void DEC(void);
extern void BENCH(void);

/* ---- board terminal I/O ---- */
extern int  board_key(void);
extern void board_emit(int ch);

/* ---- flat map helpers ---- */
static inline uint32_t *cw(uint32_t a){ return (uint32_t *)(uintptr_t)a; }
static inline uint8_t  *cb(uint32_t a){ return (uint8_t  *)(uintptr_t)a; }

#define RD(off)   (*(cw(STATE + (off))))
#define WR(off,v) ((*(cw(STATE + (off)))) = (uint32_t)(v))

/* ---- dictionary pointer helpers ---- */
static uint32_t here(void)          { return RD(S_HERE); }
static void     here_set(uint32_t a){ WR(S_HERE, a); }
static void     body_word(uint32_t tok){
    uint32_t a = here(); *cw(a) = tok; here_set(a + 4);
}

/* ---------------------------------------------------------------------
 * Terminal line handler
 * ------------------------------------------------------------------- */
static void getline(void)
{
    uint32_t i = 0;
    for (;;) {
        int c = board_key();
        if (c == '\r' || c == '\n') {
            board_emit('\r');
            board_emit('\n');
            cb(TIB + i) = 0;
            WR(S_CIN, TIB);
            return;
        }
        if (c == 0x7f || c == 8) {
            if (i) { i--; board_emit(8); board_emit(' '); board_emit(8); }
            continue;
        }
        cb(TIB + i) = (uint8_t)c;
        board_emit(c);
        i++;
    }
}

/* ---------------------------------------------------------------------
 * Tokenizer.  S_CIN holds a TIB offset; next_tok advances it and sets
 * t_start/t_len.  Returns 1 if a token, else 0 (end of line).
 * ------------------------------------------------------------------- */
static uint32_t t_start, t_len;

static int next_tok(void)
{
    uint32_t p = RD(S_CIN);
    for (;;) {
        uint8_t c = cb(TIB + p);
        if (c == 0)       { WR(S_CIN, p); return 0; }
        if (c != ' ') break;
        p++;
    }
    uint32_t s = p;
    while (cb(TIB + p) != ' ' && cb(TIB + p) != 0) p++;
    t_start = s;
    t_len   = p - s;
    WR(S_CIN, p);
    return 1;
}

/* ---------------------------------------------------------------------
 * Dictionary
 * ------------------------------------------------------------------- */
static int find_word(uint32_t *entry, int *is_imm)
{
    uint32_t h = RD(S_LATEST);
    while (h) {
        uint32_t link = *cw(h);
        uint8_t  lf   = *cb(h + 4);
        uint32_t n    = lf & LEN_MASK;
        if (n == t_len && memcmp(cb(h + 5), cb(TIB + t_start), n) == 0) {
            *is_imm = (lf & FLAG_IMM) != 0;
            uint32_t e = h + 5 + n;
            e = (e + 3) & ~3u;                 /* align body to 4 bytes */
            *entry = e;
            return 1;
        }
        h = link;
    }
    return 0;
}

static uint32_t dict_header(void)
{
    uint32_t at  = here();
    uint32_t link = RD(S_LATEST);
    *cw(at)       = link;
    *cb(at + 4)   = (uint8_t)t_len;
    memcpy(cb(at + 5), cb(TIB + t_start), t_len);
    uint32_t e = at + 5 + t_len;
    e = (e + 3) & ~3u;
    here_set(e);
    WR(S_LATEST, at);
    return e;                                  /* first body cell addr */
}

static void emit_call(uint32_t body) { body_word((uint32_t)(uintptr_t)DOCOL); body_word(body); }
static void emit_lit(uint32_t v)     { body_word((uint32_t)(uintptr_t)LIT);   body_word(v);   }
static void emit_exit(void)          { body_word((uint32_t)(uintptr_t)EXIT); }

static int parse_num(uint32_t *v)
{
    uint32_t val = 0;
    if (t_len == 0) return 0;
    for (uint32_t i = 0; i < t_len; i++) {
        uint8_t c = cb(TIB + t_start + i);
        if (c < '0' || c > '9') return 0;
        val = val * 10 + (c - '0');
    }
    *v = val;
    return 1;
}

/* Faster path for interpretation of a literal: build [LIT][v][EXIT] in a
 * fixed scratch zone (well above the dictionary), run it, and unwind. */
static void run_literal(uint32_t v)
{
    uint32_t save   = RD(S_HERE);
    uint32_t scratch = LIT_SCRATCH;         /* fixed scratch zone        */
    WR(S_HERE, scratch);
    body_word((uint32_t)(uintptr_t)LIT);
    body_word(v);
    emit_exit();
    uint32_t entry = scratch;
    WR(S_HERE, save);                        /* restore dictionary top    */
    forth_run(entry);
}

/* ---------------------------------------------------------------------
 * Seed the dictionary so `find` can resolve the primitive words.
 * Each primitive is a header placed at HERE; its body address points at
 * the assembly routine (so forth_run runs the primitive directly).
 * ------------------------------------------------------------------- */
static uint32_t seed_one(const char *name, uint32_t bodyaddr)
{
    uint32_t at  = here();
    uint32_t link = RD(S_LATEST);
    *cw(at)       = link;
    uint32_t n    = strlen(name);
    *cb(at + 4)   = (uint8_t)(n & LEN_MASK);
    memcpy(cb(at + 5), name, n);
    uint32_t e = at + 5 + n;
    e = (e + 3) & ~3u;
    *(cw(e)) = bodyaddr;                /* body == the primitive addr token */
    here_set(e + 4);
    WR(S_LATEST, at);
    return e;                            /* entry = this body cell          */
}

static void seed_primitives(void)
{
    seed_one("@",    (uint32_t)(uintptr_t)FETCH);
    seed_one("!",    (uint32_t)(uintptr_t)STORE);
    seed_one("sp@",  (uint32_t)(uintptr_t)SPFETCH);
    seed_one("rp@",  (uint32_t)(uintptr_t)RPFETCH);
    seed_one("0#",   (uint32_t)(uintptr_t)ZEROEQ);
    seed_one("+",    (uint32_t)(uintptr_t)PLUS);
    seed_one("nand", (uint32_t)(uintptr_t)NAND);
    seed_one("exit", (uint32_t)(uintptr_t)EXIT);
    seed_one("key",  (uint32_t)(uintptr_t)KEY);
    seed_one("emit", (uint32_t)(uintptr_t)EMIT);
    seed_one("s@",   (uint32_t)(uintptr_t)STATEVAR);
    seed_one("led",  (uint32_t)(uintptr_t)LED);
    seed_one("ms",   (uint32_t)(uintptr_t)MS);
    seed_one("clock",(uint32_t)(uintptr_t)CLOCK);
    seed_one("dec",  (uint32_t)(uintptr_t)DEC);
    seed_one("bench",(uint32_t)(uintptr_t)BENCH);
}

void forth_init(void)
{
    WR(S_STATE, 0);
    WR(S_CIN, TIB);
    WR(S_HERE, DICT_BASE);                             /* dictionary grows up from here */
    WR(S_LATEST, 0u);
    seed_primitives();
}

/* ---- warm start: re-parse loops forever ---- */
void forth_outer(void)
{
    for (;;) {
        getline();
        while (next_tok()) {
            if (t_len == 1 && cb(TIB + t_start)[0] == ':') {
                if (!next_tok()) break;             /* need a name */
                dict_header();
                WR(S_STATE, 1);                     /* compile on  */
                continue;
            }
            if (t_len == 1 && cb(TIB + t_start)[0] == ';') {
                emit_exit();
                WR(S_STATE, 0);                     /* compile off */
                continue;
            }
            int is_imm = 0;
            uint32_t entry;
            if (find_word(&entry, &is_imm)) {
                if (RD(S_STATE) && !is_imm) emit_call(entry);
                else                       forth_run(entry);
                continue;
            }
            uint32_t num;
            if (parse_num(&num)) {
                if (RD(S_STATE)) emit_lit(num);
                else             run_literal(num);
                continue;
            }
            /* unknown word -> blank line (milliForth behavior) */
            board_emit('\n');
            break;
        }
    }
}