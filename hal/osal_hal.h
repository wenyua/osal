/**
 * @file osal_hal.h
 * @brief OSAL 移植层接口实现 (通用/默认版本)
 *
 * OSAL 核心层(type.h)只依赖本文件提供的接口, 移植到其他平台时
 * 只需在对应 hal/ 目录下提供一份同名的 osal_hal.h:
 *   - HAL_DISABLE_INTERRUPTS() / HAL_ENABLE_INTERRUPTS()   全局中断开关
 *   - HAL_ENTER_CRITICAL_SECTION() / HAL_EXIT_CRITICAL_SECTION() 临界区
 *   - OSAL_IDLE_SLEEP()                                    空闲低功耗
 *
 * 默认版本:
 *   - ARM Cortex (gcc): 内联汇编 cpsid/cpsie + wfi
 *   - 其他主机/仿真环境: 空实现(单线程下安全)
 * @date  2026-09-11
 */
#ifndef OSAL_HAL_H
#define OSAL_HAL_H

#if defined(__GNUC__) && (defined(__arm__) || defined(__aarch64__))

/* ARM Cortex-M: 全局中断用 PRIMASK 控制 */
static __inline void __attribute__((always_inline)) OSAL_CLI(void)
{
    __asm volatile ("cpsid i" ::: "memory");
}
static __inline void __attribute__((always_inline)) OSAL_SEI(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}

#define HAL_DISABLE_INTERRUPTS()        OSAL_CLI()
#define HAL_ENABLE_INTERRUPTS()         OSAL_SEI()
#define HAL_ENTER_CRITICAL_SECTION()    OSAL_CLI()
#define HAL_EXIT_CRITICAL_SECTION()     OSAL_SEI()

/* 空闲低功耗: WFI, 任意中断可唤醒 */
#define OSAL_IDLE_SLEEP()               __asm volatile ("wfi" ::: "memory")

#else /* 主机/仿真环境: 无硬件中断概念, 空实现 */

#define HAL_DISABLE_INTERRUPTS()        do {} while (0)
#define HAL_ENABLE_INTERRUPTS()         do {} while (0)
#define HAL_ENTER_CRITICAL_SECTION()    do {} while (0)
#define HAL_EXIT_CRITICAL_SECTION()     do {} while (0)
#define OSAL_IDLE_SLEEP()               do {} while (0)

#endif

#endif /* OSAL_HAL_H */
