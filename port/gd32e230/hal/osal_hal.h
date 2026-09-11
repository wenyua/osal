/**
 * @file osal_hal.h
 * @brief OSAL 移植层接口实现 (GD32E230 / Cortex-M23)
 *
 * OSAL 核心层(type.h)只依赖本文件提供的接口, 移植到其他 MCU 时
 * 只需在新目标的 hal/ 目录下提供一份同名的 osal_hal.h:
 *   - HAL_DISABLE_INTERRUPTS() / HAL_ENABLE_INTERRUPTS()   全局中断开关
 *   - HAL_ENTER_CRITICAL_SECTION() / HAL_EXIT_CRITICAL_SECTION() 临界区
 *   - OSAL_IDLE_SLEEP()                                    空闲低功耗(WFI 等)
 * @date  2026-09-11
 */
#ifndef OSAL_HAL_H
#define OSAL_HAL_H

/*
 * 使用 GD32 固件库(USE_GD32_FW)时, 由 CMSIS core_cm23.h 提供
 * __disable_irq/__enable_irq; 否则用内联汇编自建。
 */
#if defined(USE_GD32_FW)
#include "gd32e23x.h"

#define OSAL_CLI()          __disable_irq()
#define OSAL_SEI()          __enable_irq()

#else /* 不依赖固件库/CMSIS 的最小实现 (ARM GCC) */

static __inline void __attribute__((always_inline)) OSAL_CLI(void)
{
    __asm volatile ("cpsid i" ::: "memory");
}
static __inline void __attribute__((always_inline)) OSAL_SEI(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}

#endif /* USE_GD32_FW */

#define HAL_DISABLE_INTERRUPTS()        OSAL_CLI()      /* 关全局中断      */
#define HAL_ENABLE_INTERRUPTS()         OSAL_SEI()      /* 开全局中断      */

#define HAL_ENTER_CRITICAL_SECTION()    OSAL_CLI()      /* 进入临界区      */
#define HAL_EXIT_CRITICAL_SECTION()     OSAL_SEI()      /* 退出临界区      */

/* 空闲低功耗: Cortex-M 睡眠指令, 任意中断可唤醒 */
#define OSAL_IDLE_SLEEP()               __asm volatile ("wfi" ::: "memory")

#endif /* OSAL_HAL_H */
