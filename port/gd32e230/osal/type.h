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

#if !defined(USE_GD32_FW) && !defined(SUCCESS)
#define SUCCESS     1
#endif

/*
 * ERROR 在包含 GD32 固件库时与其 ErrStatus 枚举冲突, 因此当固件库被使用时
 * (USE_GD32_FW) 不再定义 ERROR。应用层请优先使用 ZSUCCESS / INVALID_* 常量。
 */
#if !defined(USE_GD32_FW) && !defined(ERROR)
#define ERROR       0
#endif

/*
 * GD32E230 为 Cortex-M23 内核。临界区使用 __disable_irq/__enable_irq。
 * 当使用了 GD32 固件库(USE_GD32_FW)时, 由 CMSIS core_cm23.h 提供这两个函数;
 * 否则在此用内联汇编自建, 避免找不到符号。
 */
#if defined(USE_GD32_FW)
#include "gd32e23x.h"          /* 经由固件库引入 core_cm23.h, 提供 __enable_irq/__disable_irq */
#define OSAL_CMSIS_INCLUDED     1
#endif /* USE_GD32_FW */

#if !defined(__CORTEX_M) && !defined(__CORE_CM23_H_GENERIC)
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

#define HAL_ENABLE_INTERRUPTS()         SEI()       // Enable Interrupts
#define HAL_DISABLE_INTERRUPTS()        CLI()       // Disable Interrupts
#define HAL_INTERRUPTS_ARE_ENABLED()    SEI()       // Enable Interrupts

#define HAL_ENTER_CRITICAL_SECTION()    CLI()
#define HAL_EXIT_CRITICAL_SECTION()     SEI()

#endif
