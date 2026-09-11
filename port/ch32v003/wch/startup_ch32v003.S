/*
 * @file startup_ch32v003.S
 * @brief CH32V003 最小启动文件 (rv32e)
 *        - 初始化栈指针
 *        - mtvec 指向向量表(直接模式, 4字节一格)
 *        - 清零 .bss, 拷贝 .data, 跳转 main
 */
    .section .init
    .globl _start
_start:
    li      sp, _eusrstack              /* 栈顶由链接脚本提供 */

    /* 向量表基址: 直接模式 */
    la      t0, vector_base
    csrw    mtvec, t0

    /* 清零 .bss */
    la      t0, _sbss
    la      t1, _ebss
1:  bgeu    t0, t1, 2f
    sw      zero, 0(t0)
    addi    t0, t0, 4
    j       1b
2:
    /* 拷贝 .data 初始化值: Flash(_sidata) -> RAM(_sdata) */
    la      t0, _sidata
    la      t1, _sdata
    la      t2, _edata
3:  bgeu    t1, t2, 4f
    lw      t3, 0(t0)
    sw      t3, 0(t1)
    addi    t0, t0, 4
    addi    t1, t1, 4
    j       3b
4:
    call    main
5:  j       5b                          /* main 不应返回 */

    /* 默认中断处理: 死循环, 便于调试发现意外中断 */
    .weak   default_handler
default_handler:
    j       default_handler

    .size _start, .-_start

/* ------------------------------------------------------------------
 * 向量表: 直接模式, 每格 4 字节。
 * CH32V003 中断号: 12 = SysTick(STK)
 * ------------------------------------------------------------------ */
    .section .isr_vector, "ax"
    .globl vector_base
vector_base:
    j _start                        /*  0: 顶部栈指针/复位入口 */
    .rept 11                        /*  1..11: 保留           */
    j default_handler
    .endr
    j SysTick_Handler               /* 12: SysTick            */
    j default_handler               /* 13                     */
    j default_handler               /* 14                     */
    j default_handler               /* 15                     */
