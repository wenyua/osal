/* GD32E230 Keil-EVB RAM/FLASH 配置
 * GD32E230 系列：Flash 常见 64KB（此处示例），SRAM 8KB。
 * 若实际芯片 Flash 容量不同，请修改下面两行 LENGTH。
 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 8K
}

/* 栈大小，8KB RAM 中分配 2KB 栈 */
_estack = ORIGIN(RAM) + LENGTH(RAM);
_Min_Heap_Size = 0x1000;

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP (*(.init))
        KEEP (*(.fini))
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.arm.extab.*)
    } >FLASH

    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH

    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

    .data :
    {
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(8);
        _edata = .;
    } >RAM AT> FLASH

    _sidata = LOADADDR(.data);

    _sbss = .;
    .bss :
    {
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        _ebss = .;
    } >RAM
    . = ALIGN(4);
    _end = .;
    PROVIDE ( end = . );
}

