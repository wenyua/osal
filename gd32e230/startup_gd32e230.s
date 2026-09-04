.syntax unified
.cpu cortex-m23
.fpu softvfp
.thumb

/* 向量表首项：栈顶地址，字在链接脚本中定义 */
.global g_pfnVectors
.global Default_Handler

/* 链接脚本符号 */
.extern _estack
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss

/* OSAL 滴答中断与 C 入口 */
.extern SysTick_Handler
.extern main

.section  .text.Reset_Handler
.weak  Reset_Handler
.type  Reset_Handler, %function
Reset_Handler:
    ldr   sp, =_estack        /* 设置栈指针 */
    ldr   r0, =_sdata
    ldr   r1, =_edata
    ldr   r2, =_sidata        /* 初始化数据段起始(flash) */
copy_data:
    cmp   r0, r1
    bge   zero_bss
    ldr   r3, [r2]
    str   r3, [r0]
    adds  r0, r0, #4
    adds  r2, r2, #4
    b     copy_data
zero_bss:
    ldr   r0, =_sbss
    ldr   r1, =_ebss
    movs  r2, #0
zero_loop:
    cmp   r0, r1
    bge   call_main
    str   r2, [r0]
    adds  r0, r0, #4
    b     zero_loop
call_main:
    bl    main
    b     .

.size  Reset_Handler, .-Reset_Handler

/* 默认中断处理：未使用的中断跳转死循环 */
.thumb_func
.section .text.Default_Handler, "ax", %progbits
.type  Default_Handler, %function
Default_Handler:
    b     .
.size  Default_Handler, .-Default_Handler

/* GD32E230 中断向量表 */
.section .isr_vector, "ax", %progbits
.type  g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word 0
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* 外部中断（默认指向 Default_Handler） */
    .word WWDT_IRQHandler
    .word LVD_IRQHandler
    .word TAMPER_IRQHandler
    .word RTC_IRQHandler
    .word FMC_IRQHandler
    .word RCU_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA0_Channel0_IRQHandler
    .word DMA0_Channel1_IRQHandler
    .word DMA0_Channel2_IRQHandler
    .word DMA0_Channel3_IRQHandler
    .word DMA0_Channel4_IRQHandler
    .word DMA0_Channel5_IRQHandler
    .word DMA0_Channel6_IRQHandler
    .word ADC_CMP_IRQHandler
    .word USART0_IRQHandler
    .word USART1_IRQHandler
    .word TIMER0_BRK_UP_TRG_COM_IRQHandler
    .word TIMER0_Channel_IRQHandler
    .word TIMER1_IRQHandler
    .word TIMER2_IRQHandler
    .word TIMER3_IRQHandler
    .word I2C0_EV_IRQHandler
    .word I2C0_ER_IRQHandler
    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word SPI0_IRQHandler
    .word SPI1_IRQHandler
    .word UART4_IRQHandler
    .word UART5_IRQHandler
    .word UART6_IRQHandler
    .word UART7_IRQHandler
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .size  g_pfnVectors, .-g_pfnVectors

/* 其余中断处理器各占一个跳板，指向 Default_Handler */
.macro IRQ_HANDLER name
.thumb_func
.weak \name
.section .text.\name, "ax", %progbits
.type \name, %function
\name:
    b Default_Handler
.size \name, .-\name
.endm

    IRQ_HANDLER NMI_Handler
    IRQ_HANDLER HardFault_Handler
    IRQ_HANDLER SVC_Handler
    IRQ_HANDLER PendSV_Handler
    IRQ_HANDLER WWDT_IRQHandler
    IRQ_HANDLER LVD_IRQHandler
    IRQ_HANDLER TAMPER_IRQHandler
    IRQ_HANDLER RTC_IRQHandler
    IRQ_HANDLER FMC_IRQHandler
    IRQ_HANDLER RCU_IRQHandler
    IRQ_HANDLER EXTI0_IRQHandler
    IRQ_HANDLER EXTI1_IRQHandler
    IRQ_HANDLER EXTI2_IRQHandler
    IRQ_HANDLER EXTI3_IRQHandler
    IRQ_HANDLER EXTI4_IRQHandler
    IRQ_HANDLER DMA0_Channel0_IRQHandler
    IRQ_HANDLER DMA0_Channel1_IRQHandler
    IRQ_HANDLER DMA0_Channel2_IRQHandler
    IRQ_HANDLER DMA0_Channel3_IRQHandler
    IRQ_HANDLER DMA0_Channel4_IRQHandler
    IRQ_HANDLER DMA0_Channel5_IRQHandler
    IRQ_HANDLER DMA0_Channel6_IRQHandler
    IRQ_HANDLER ADC_CMP_IRQHandler
    IRQ_HANDLER USART0_IRQHandler
    IRQ_HANDLER USART1_IRQHandler
    IRQ_HANDLER TIMER0_BRK_UP_TRG_COM_IRQHandler
    IRQ_HANDLER TIMER0_Channel_IRQHandler
    IRQ_HANDLER TIMER1_IRQHandler
    IRQ_HANDLER TIMER2_IRQHandler
    IRQ_HANDLER TIMER3_IRQHandler
    IRQ_HANDLER I2C0_EV_IRQHandler
    IRQ_HANDLER I2C0_ER_IRQHandler
    IRQ_HANDLER I2C1_EV_IRQHandler
    IRQ_HANDLER I2C1_ER_IRQHandler
    IRQ_HANDLER SPI0_IRQHandler
    IRQ_HANDLER SPI1_IRQHandler
    IRQ_HANDLER UART4_IRQHandler
    IRQ_HANDLER UART5_IRQHandler
    IRQ_HANDLER UART6_IRQHandler
    IRQ_HANDLER UART7_IRQHandler
.end


