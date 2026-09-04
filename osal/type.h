#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>

#define ZSUCCESS                  1
#define INVALID_TASK              2
#define INVALID_MSG_POINTER       3
#define INVALID_EVENT_ID          4
#define NO_TIMER_AVAIL            5
#define TASK_NO_TASK              6
#define MSG_BUFFER_NOT_AVAIL      7

typedef unsigned char       BOOL;

//芯片硬件字长，GD32E230 数据总线为32位，保证数据对齐到4字节
typedef unsigned int        halDataAlign_t;

// Unsigned numbers
typedef uint8_t             uint8;
typedef uint8_t             byte;
typedef uint16_t            uint16;
typedef uint16_t            int16U;
typedef uint32_t            uint32;
typedef uint32_t            int32U;

// Signed numbers
typedef int8_t              int8;
typedef int16_t             int16;
typedef int32_t             int32;

#ifndef FALSE
#define FALSE       0
#endif

#ifndef TRUE
#define TRUE        1
#endif

#ifndef ARRAY_NULL
#define ARRAY_NULL  '\0'
#endif

#ifndef OPEN
#define OPEN        1
#endif

#ifndef CLOSE
#define CLOSE       0
#endif

#ifndef NULL
#define NULL        ((void*) 0)
#endif

#ifndef HIGH
#define HIGH        1
#endif

#ifndef LOW
#define LOW         0
#endif

#ifndef SUCCESS
#define SUCCESS     1
#endif

#ifndef ERROR
#define ERROR       0
#endif

/*
 * GD32E230 为 Cortex-M23 内核，临界区使用 PRIMASK 控制全局中断。
 * 若工程已提供 CMSIS core_cm23.h，则 __disable_irq/__enable_irq 由
 * CMSIS 提供；否则可在这里实现内联汇编版本。
 */
#if defined(__GNUC__)

#ifndef __disable_irq
static __inline void __attribute__((always_inline)) __disable_irq(void)
{
    __asm volatile ("cpsid i" ::: "memory");
}

static __inline void __attribute__((always_inline)) __enable_irq(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}
#endif

#define CLI()         __disable_irq()                 // Disable Interrupts
#define SEI()         __enable_irq()                 // Enable Interrupts

#else

#define CLI()         __set_PRIMASK(1)               // Disable Interrupts
#define SEI()         __set_PRIMASK(0)               // Enable Interrupts

#endif

#define HAL_ENABLE_INTERRUPTS()         SEI()       // Enable Interrupts
#define HAL_DISABLE_INTERRUPTS()        CLI()       // Disable Interrupts
#define HAL_INTERRUPTS_ARE_ENABLED()    SEI()       // Enable Interrupts

#define HAL_ENTER_CRITICAL_SECTION()    CLI()
#define HAL_EXIT_CRITICAL_SECTION()     SEI()

#endif
