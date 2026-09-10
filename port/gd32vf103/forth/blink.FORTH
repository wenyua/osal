\ milliForth on GD32VF103 - blink the onboard LED (PC13, active-low)
\
\   led ( n -- )  n != 0 => LED on, else off
\   ms  ( n -- )  busy-wait ~n ms
\
\ This kernel has no loop word, so "blink" recurses by calling itself at
\ the end of its own body (the return stack handle the repeated frames).
\
: blink
    1 led 250 ms          \ on 250 ms
    0 led 250 ms          \ off 250 ms
    blink                 \ loop: jump back to the start of this definition
    ;