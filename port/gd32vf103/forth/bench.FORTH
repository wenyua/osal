\ milliForth on GD32VF103 - measure Forth / server overhead.
\
\ Machine words added for benchmarking (see README/forth.c):
\   clock ( -- low32 )       low 32 bits of the mcycle counter
\   dec   ( n -- )           print n in decimal followed by a space
\   bench ( -- )             run a 100000-iteration C loop, print cycles
\                            as:  <delta>/<N>=<per>
\
\ How to measure one primitive's overhead from the console:
\   1.  Input `bench`  -> prints native /N overhead for a plain C loop.
\   2.  For the threaded interpreter's per-token cost, run two `clock`
\       readings around a whole colon call and diff on paper:
\           clock dec  clock dec
\       (the difference in the two printed numbers = mcycle span of that
\        single word; dump it by hand).
\
\ Because the small vocabulary lacks `-` and a loop/#if, this file keeps the
\ benchmark harness in C (`bench`) rather than in Forth; the per-word clock
\ approach is done interactively for settled quick checks.
: