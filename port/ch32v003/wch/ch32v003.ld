/* CH32V003: 16KB Flash, 2KB RAM */
ENTRY(_start)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 16K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 2K
}

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        *(.init)
        *(.text .text.*)
        *(.rodata .rodata.*)
        . = ALIGN(4);
    } >FLASH

    .isr_vector :
    {
        . = ALIGN(4);
        *(.isr_vector)
        . = ALIGN(4);
    } >FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data.*)
        . = ALIGN(4);
        _edata = .;
    } >RAM AT>FLASH
    _sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >RAM

    _eusrstack = ORIGIN(RAM) + LENGTH(RAM);
}
